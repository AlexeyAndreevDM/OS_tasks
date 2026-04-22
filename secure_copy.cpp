#include <pthread.h>
#include <signal.h>
#include <unistd.h>

#include <algorithm>
#include <cerrno>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <ctime>
#include <queue>
#include <string>
#include <vector>

#include "caesar.h"

static volatile sig_atomic_t keep_running = 1;  // Глобальный флаг остановки по SIGINT

static pthread_mutex_t g_log_mutex;  // Защита общего лог-файла
static FILE* g_log_file = nullptr;

#ifndef WORKERS_COUNT
#define WORKERS_COUNT 4
#endif

enum class Mode {
    Sequential,
    Parallel,
    Auto
};

// Статистика одного запуска в конкретном режиме

struct RunStats {
    size_t total_files;
    size_t completed;
    size_t failed;
    size_t interrupted;
    size_t bytes_written;
    double total_ms;
    double avg_ms;
};

struct DataBlock {
    std::vector<unsigned char> bytes;
};

struct PipelineState {
    FILE* input_file;
    FILE* output_file;
    std::queue<DataBlock> blocks;  // Очередь обмена producer -> consumer
    size_t max_blocks;             // Ограничение размера очереди (чтобы не копить весь файл в памяти)
    bool producer_done;
    bool has_error;
    std::string error_message;
    size_t bytes_written;
    pthread_mutex_t mutex;
    pthread_cond_t can_produce;
    pthread_cond_t can_consume;
};

struct WorkerTask {
    std::string input_path;
    std::string output_path;
    int status;
    size_t written_bytes;
    double duration_ms;
};

struct ParallelContext {
    std::vector<WorkerTask>* tasks;
    size_t next_index;          // Индекс следующей необработанной задачи
    bool ready;                 // Главный поток разрешил worker-потокам старт
    pthread_mutex_t queue_mutex;
    pthread_cond_t queue_cv;
    pthread_mutex_t stats_mutex;
    RunStats stats;
};

static void handle_sigint(int) {
    keep_running = 0;  // ставим флаг на 0 чтоб потоки завершились сами, увидев флаг
}

static double now_ms() {
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return static_cast<double>(ts.tv_sec) * 1000.0 + static_cast<double>(ts.tv_nsec) / 1000000.0;
}

// Таймаутный захват мьютекса: помогает не висеть бесконечно при взаимоблокировке
static bool lock_with_timeout(pthread_mutex_t* mutex, int timeout_ms) {
    const int step_ms = 5;  // раз в 5 мс пробуем захватить мьютекс
    int waited_ms = 0;      // сколько мс уже ждем
    while (waited_ms < timeout_ms) {
        int rc = pthread_mutex_trylock(mutex);
        if (rc == 0) {
            return true;
        }
        if (rc != EBUSY) {  // Ошибка, отличная от "занят"
            return false;
        }
        usleep(step_ms * 1000);
        waited_ms += step_ms;
    }
    return false;
}

static void log_line(const std::string& line) {
    if (g_log_file == nullptr) {
        return;
    }

    if (!lock_with_timeout(&g_log_mutex, 300)) {
        fprintf(stderr, "[WARN] Таймаут захвата log_mutex\n");
        return;
    }

    time_t now = time(nullptr);
    struct tm tm_now;
    localtime_r(&now, &tm_now);
    char ts[32];
    strftime(ts, sizeof(ts), "%Y-%m-%d %H:%M:%S", &tm_now);

    fprintf(g_log_file, "[%s] %s\n", ts, line.c_str());  // Потокобезопасное логирование
    fflush(g_log_file);
    pthread_mutex_unlock(&g_log_mutex);
}

static void set_pipeline_error(PipelineState* state, const char* message) {
    state->has_error = true;
    state->error_message = message;
    pthread_cond_broadcast(&state->can_produce);
    pthread_cond_broadcast(&state->can_consume);
}

static void* producer_thread(void* arg) {
    PipelineState* state = static_cast<PipelineState*>(arg);
    unsigned char temp[4096];

    while (keep_running) {
        size_t bytes_read = fread(temp, 1, sizeof(temp), state->input_file);

        if (bytes_read > 0) {
            caesar(temp, temp, static_cast<int>(bytes_read));

            DataBlock block;
            block.bytes.assign(temp, temp + bytes_read);

            pthread_mutex_lock(&state->mutex);
            while (keep_running && !state->has_error && state->blocks.size() >= state->max_blocks) {
                // Если очередь переполнена — producer ждет, пока consumer освободит место
                pthread_cond_wait(&state->can_produce, &state->mutex);
            }

            if (!keep_running || state->has_error) {
                pthread_mutex_unlock(&state->mutex);
                break;
            }

            state->blocks.push(std::move(block));  // Кладем шифрованный блок в очередь
            pthread_cond_signal(&state->can_consume);
            pthread_mutex_unlock(&state->mutex);
        }

        if (bytes_read < sizeof(temp)) {
            pthread_mutex_lock(&state->mutex);
            if (ferror(state->input_file)) {
                set_pipeline_error(state, "Ошибка чтения входного файла");
            }
            state->producer_done = true;
            pthread_cond_broadcast(&state->can_consume);
            pthread_mutex_unlock(&state->mutex);
            break;
        }
    }

    pthread_mutex_lock(&state->mutex);
    state->producer_done = true;
    pthread_cond_broadcast(&state->can_consume);
    pthread_mutex_unlock(&state->mutex);
    return nullptr;
}

static void* consumer_thread(void* arg) {
    PipelineState* state = static_cast<PipelineState*>(arg);

    while (true) {
        pthread_mutex_lock(&state->mutex);

        while (!state->has_error && keep_running && state->blocks.empty() && !state->producer_done) {
            // Если данных пока нет — consumer ждет сигнал от producer
            pthread_cond_wait(&state->can_consume, &state->mutex);
        }

        if (state->has_error || (state->blocks.empty() && (!keep_running || state->producer_done))) {
            pthread_mutex_unlock(&state->mutex);
            break;
        }

        DataBlock block = std::move(state->blocks.front());  // Берем следующий блок из очереди
        state->blocks.pop();
        pthread_cond_signal(&state->can_produce);
        pthread_mutex_unlock(&state->mutex);

        size_t to_write = block.bytes.size();
        size_t total_written = 0;
        while (total_written < to_write) {
            size_t written = fwrite(block.bytes.data() + total_written, 1, to_write - total_written, state->output_file);
            if (written == 0) {
                pthread_mutex_lock(&state->mutex);
                set_pipeline_error(state, "Ошибка записи выходного файла");
                pthread_mutex_unlock(&state->mutex);
                return nullptr;
            }
            total_written += written;
        }
        state->bytes_written += to_write;
    }

    return nullptr;
}

// Нужна для поддержки двух форм ключа: символ (X) и число (88)
static bool parse_key(const char* key_arg, char* out_key) {
    char* end = nullptr;
    errno = 0;
    long value = strtol(key_arg, &end, 10);

    if (errno == 0 && end != nullptr && *key_arg != '\0' && *end == '\0') {
        if (value < 0 || value > 255) {
            return false;
        }
        *out_key = static_cast<char>(static_cast<unsigned char>(value));
        return true;
    }

    if (strlen(key_arg) == 1) {
        *out_key = key_arg[0];
        return true;
    }

    return false;
}

static int process_one_file(const std::string& input_path, const std::string& output_path, size_t* written_bytes) {
    FILE* input_file = fopen(input_path.c_str(), "rb");
    if (!input_file) {
        return 1;
    }

    FILE* output_file = fopen(output_path.c_str(), "wb");
    if (!output_file) {
        fclose(input_file);
        return 1;
    }

    PipelineState state;
    state.input_file = input_file;
    state.output_file = output_file;
    state.max_blocks = 4;  // Небольшой буфер между потоками: стабильный расход памяти
    state.producer_done = false;
    state.has_error = false;
    state.bytes_written = 0;
    pthread_mutex_init(&state.mutex, nullptr);
    pthread_cond_init(&state.can_produce, nullptr);
    pthread_cond_init(&state.can_consume, nullptr);

    pthread_t producer;
    pthread_t consumer;

    if (pthread_create(&producer, nullptr, producer_thread, &state) != 0) {
        fclose(input_file);
        fclose(output_file);
        pthread_mutex_destroy(&state.mutex);
        pthread_cond_destroy(&state.can_produce);
        pthread_cond_destroy(&state.can_consume);
        return 1;
    }

    if (pthread_create(&consumer, nullptr, consumer_thread, &state) != 0) {
        keep_running = 0;
        pthread_join(producer, nullptr);
        fclose(input_file);
        fclose(output_file);
        pthread_mutex_destroy(&state.mutex);
        pthread_cond_destroy(&state.can_produce);
        pthread_cond_destroy(&state.can_consume);
        return 1;
    }

    pthread_join(producer, nullptr);
    pthread_join(consumer, nullptr);

    fclose(input_file);
    fclose(output_file);
    pthread_mutex_destroy(&state.mutex);
    pthread_cond_destroy(&state.can_produce);
    pthread_cond_destroy(&state.can_consume);

    if (!keep_running) {
        remove(output_path.c_str());  // Удаляем битый выходной файл при Ctrl+C
        return 130;
    }

    if (state.has_error) {
        remove(output_path.c_str());  // Удаляем неполный файл при ошибке пайплайна
        return 1;
    }

    *written_bytes = state.bytes_written;  // Сколько реально записали по этой задаче
    return 0;
}

static const char* mode_name(Mode mode) {
    if (mode == Mode::Sequential) {
        return "sequential";
    }
    if (mode == Mode::Parallel) {
        return "parallel";
    }
    return "auto";
}

static bool parse_mode(const char* arg, Mode* out_mode) {
    const char* prefix = "--mode=";
    if (strncmp(arg, prefix, strlen(prefix)) != 0) {
        return false;
    }

    const char* value = arg + strlen(prefix);
    if (strcmp(value, "sequential") == 0) {
        *out_mode = Mode::Sequential;
        return true;
    }
    if (strcmp(value, "parallel") == 0) {
        *out_mode = Mode::Parallel;
        return true;
    }
    if (strcmp(value, "auto") == 0) {
        *out_mode = Mode::Auto;
        return true;
    }
    return false;
}

static Mode choose_auto_mode(size_t files_count) {
    // Эвристика из задания 4: <5 -> sequential, >=5 -> parallel
    return (files_count < 5) ? Mode::Sequential : Mode::Parallel;
}

static void init_stats(RunStats* stats, size_t total_files) {
    stats->total_files = total_files;
    stats->completed = 0;
    stats->failed = 0;
    stats->interrupted = 0;
    stats->bytes_written = 0;
    stats->total_ms = 0.0;
    stats->avg_ms = 0.0;
}

static void accumulate_task_result(RunStats* stats, const WorkerTask& task) {
    if (task.status == 0) {
        stats->completed++;
        stats->bytes_written += task.written_bytes;
    } else if (task.status == 130) {
        stats->interrupted++;
    } else {
        stats->failed++;
    }
}

static void finalize_avg(RunStats* stats, const std::vector<WorkerTask>& tasks) {
    size_t measured = 0;
    double sum_ms = 0.0;
    for (size_t i = 0; i < tasks.size(); ++i) {
        if (tasks[i].duration_ms > 0.0) {
            measured++;
            sum_ms += tasks[i].duration_ms;
        }
    }
    stats->avg_ms = (measured == 0) ? 0.0 : (sum_ms / static_cast<double>(measured));
}

static void process_task(WorkerTask* task) {
    log_line("Старт: " + task->input_path + " -> " + task->output_path);

    const double started_ms = now_ms();
    task->status = process_one_file(task->input_path, task->output_path, &task->written_bytes);  // Один worker обрабатывает одну пару in/out
    task->duration_ms = now_ms() - started_ms;

    if (task->status == 0) {
        log_line("Готово: " + task->output_path);
    } else if (task->status == 130) {
        log_line("Прервано: " + task->output_path);
    } else {
        log_line("Ошибка: " + task->output_path);
    }
}

static void* pool_worker_thread(void* arg) {
    ParallelContext* context = static_cast<ParallelContext*>(arg);

    while (keep_running) {
        size_t task_index = 0;

        pthread_mutex_lock(&context->queue_mutex);
        while (keep_running && !context->ready) {
            pthread_cond_wait(&context->queue_cv, &context->queue_mutex);
        }

        if (!keep_running || context->next_index >= context->tasks->size()) {
            // Либо пришел SIGINT, либо очередь задач полностью разобрана
            pthread_mutex_unlock(&context->queue_mutex);
            break;
        }

        task_index = context->next_index;
        context->next_index++;
        pthread_mutex_unlock(&context->queue_mutex);

        WorkerTask& task = (*context->tasks)[task_index];
        process_task(&task);

        pthread_mutex_lock(&context->stats_mutex);
        accumulate_task_result(&context->stats, task);
        pthread_mutex_unlock(&context->stats_mutex);
    }

    return nullptr;
}

static RunStats run_sequential(std::vector<WorkerTask>* tasks) {
    RunStats stats;
    init_stats(&stats, tasks->size());

    const double started_ms = now_ms();
    for (size_t i = 0; i < tasks->size(); ++i) {
        if (!keep_running) {
            break;
        }
        process_task(&(*tasks)[i]);
        accumulate_task_result(&stats, (*tasks)[i]);
    }
    stats.total_ms = now_ms() - started_ms;
    finalize_avg(&stats, *tasks);
    return stats;
}

static RunStats run_parallel(std::vector<WorkerTask>* tasks) {
    RunStats stats;
    init_stats(&stats, tasks->size());

    ParallelContext context;
    context.tasks = tasks;
    context.next_index = 0;
    context.ready = false;
    context.stats = stats;

    pthread_mutex_init(&context.queue_mutex, nullptr);
    pthread_cond_init(&context.queue_cv, nullptr);
    pthread_mutex_init(&context.stats_mutex, nullptr);

    // Пул ограничен WORKERS_COUNT: не создаем поток на каждый файл
    const size_t workers_to_start = std::min(static_cast<size_t>(WORKERS_COUNT), tasks->size());
    std::vector<pthread_t> workers(workers_to_start);
    std::vector<bool> created(workers_to_start, false);

    const double started_ms = now_ms();

    for (size_t i = 0; i < workers_to_start; ++i) {
        if (pthread_create(&workers[i], nullptr, pool_worker_thread, &context) == 0) {
            created[i] = true;
        } else {
            log_line("Ошибка создания worker-потока пула");
        }
    }

    pthread_mutex_lock(&context.queue_mutex);
    context.ready = true;
    pthread_cond_broadcast(&context.queue_cv);
    pthread_mutex_unlock(&context.queue_mutex);

    for (size_t i = 0; i < workers_to_start; ++i) {
        if (created[i]) {
            pthread_join(workers[i], nullptr);
        }
    }

    context.stats.total_ms = now_ms() - started_ms;
    finalize_avg(&context.stats, *tasks);

    pthread_mutex_destroy(&context.queue_mutex);
    pthread_cond_destroy(&context.queue_cv);
    pthread_mutex_destroy(&context.stats_mutex);

    return context.stats;
}

static RunStats run_mode(Mode mode, std::vector<WorkerTask>* tasks) {
    if (mode == Mode::Sequential) {
        return run_sequential(tasks);
    }
    return run_parallel(tasks);
}

static std::vector<WorkerTask> clone_tasks_blueprint(const std::vector<WorkerTask>& blueprint) {
    std::vector<WorkerTask> copy = blueprint;
    for (size_t i = 0; i < copy.size(); ++i) {
        copy[i].status = 1;
        copy[i].written_bytes = 0;
        copy[i].duration_ms = 0.0;
    }
    return copy;
}

static void print_stats(const RunStats& stats, Mode mode) {
    printf("Режим: %s\n", mode_name(mode));
    printf("Файлов всего: %zu\n", stats.total_files);
    printf("Успешно: %zu\n", stats.completed);
    printf("С ошибкой: %zu\n", stats.failed);
    printf("Прервано: %zu\n", stats.interrupted);
    printf("Записано байт: %zu\n", stats.bytes_written);
    printf("Общее время: %.3f ms\n", stats.total_ms);
    printf("Среднее время на файл: %.3f ms\n", stats.avg_ms);
}

static void print_comparison(const RunStats& chosen, Mode chosen_mode, const RunStats& alternative, Mode alternative_mode) {
    printf("\nСравнение режимов:\n");
    printf("Выбранный (%s): %.3f ms\n", mode_name(chosen_mode), chosen.total_ms);
    printf("Альтернативный (%s): %.3f ms\n", mode_name(alternative_mode), alternative.total_ms);

    const double delta = alternative.total_ms - chosen.total_ms;
    if (delta > 0.0) {
        printf("Выбранный режим быстрее на %.3f ms\n", delta);
    } else if (delta < 0.0) {
        printf("Альтернативный режим быстрее на %.3f ms\n", -delta);
    } else {
        printf("Оба режима показали одинаковое время\n");
    }
}

static void print_usage(const char* prog) {
    fprintf(stderr, "Использование: %s [--mode=sequential|parallel|auto] <ключ> <in1> <out1> [<in2> <out2> ...]\n", prog);
}

int main(int argc, char* argv[]) {
    struct sigaction sa;
    memset(&sa, 0, sizeof(sa));
    sa.sa_handler = handle_sigint;
    sigemptyset(&sa.sa_mask);
    sigaction(SIGINT, &sa, nullptr);

    char key = 0;
    std::vector<WorkerTask> blueprint_tasks;
    Mode requested_mode = Mode::Auto;
    int cursor = 1;

    if (cursor < argc && strncmp(argv[cursor], "--mode=", 7) == 0) {
        if (!parse_mode(argv[cursor], &requested_mode)) {
            fprintf(stderr, "Ошибка: поддерживаются --mode=sequential|parallel|auto\n");
            return 1;
        }
        cursor++;
    }

    const int remaining = argc - cursor;
    if (remaining < 3 || ((remaining - 1) % 2 != 0)) {
        print_usage(argv[0]);
        return 1;
    }

    if (!parse_key(argv[cursor], &key)) {
        fprintf(stderr, "Ошибка: ключ должен быть 1 символом или числом 0..255\n");
        return 1;
    }
    cursor++;

    for (int i = cursor; i < argc; i += 2) {
        WorkerTask task;
        task.input_path = argv[i];
        task.output_path = argv[i + 1];
        task.status = 1;
        task.written_bytes = 0;
        task.duration_ms = 0.0;
        blueprint_tasks.push_back(task);
    }

    set_key(key);  // Один ключ применяется ко всем файлам текущего запуска

    pthread_mutex_init(&g_log_mutex, nullptr);
    g_log_file = fopen("secure_copy.log", "a");

    Mode effective_mode = requested_mode;
    if (effective_mode == Mode::Auto) {
        effective_mode = choose_auto_mode(blueprint_tasks.size());
    }

    log_line(std::string("Запуск secure_copy, режим=") + mode_name(effective_mode));

    std::vector<WorkerTask> chosen_tasks = clone_tasks_blueprint(blueprint_tasks);
    RunStats chosen_stats = run_mode(effective_mode, &chosen_tasks);

    print_stats(chosen_stats, effective_mode);

    if (requested_mode == Mode::Auto && keep_running) {
        // В auto-режиме дополнительно считаем альтернативный режим для сравнения
        Mode alternative_mode = (effective_mode == Mode::Sequential) ? Mode::Parallel : Mode::Sequential;
        std::vector<WorkerTask> alternative_tasks = clone_tasks_blueprint(blueprint_tasks);
        RunStats alternative_stats = run_mode(alternative_mode, &alternative_tasks);
        print_comparison(chosen_stats, effective_mode, alternative_stats, alternative_mode);
    }

    if (!keep_running) {
        printf("Операция прервана пользователем\n");
    }

    log_line("Завершение secure_copy");

    if (g_log_file) {
        fclose(g_log_file);
        g_log_file = nullptr;
    }
    pthread_mutex_destroy(&g_log_mutex);

    if (!keep_running) {
        return 130;
    }
    return (chosen_stats.failed == 0) ? 0 : 1;
}
