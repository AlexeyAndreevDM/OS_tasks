#include <pthread.h>
#include <signal.h>
#include <unistd.h>

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

static pthread_mutex_t g_log_mutex;    // Защита общего лог-файла
static pthread_mutex_t g_stats_mutex;  // Защита общей статистики
static FILE* g_log_file = nullptr;

struct GlobalStats {
    size_t total_files;
    size_t completed;
    size_t failed;
    size_t interrupted;
    size_t bytes_written;
};

static GlobalStats g_stats = {0, 0, 0, 0, 0};

static void handle_sigint(int) {
    keep_running = 0;  // ставим флаг на 0 чтоб потоки завершились сами, увидев флаг
}

// Таймаутный захват мьютекса: помогает не висеть бесконечно при взаимоблокировке
static bool lock_with_timeout(pthread_mutex_t* mutex, int timeout_ms) {
    const int step_ms = 5; // раз в 5 мс пробуем захватить мьютекс
    int waited_ms = 0; // сколько мс уже ждем
    while (waited_ms < timeout_ms) {
        int rc = pthread_mutex_trylock(mutex);
        if (rc == 0) {
            return true;
        }
        if (rc != EBUSY) { // Ошибка, отличная от "занят"
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
        // Входной файл недоступен: задачу считаем неуспешной
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

struct WorkerTask {
    std::string input_path;
    std::string output_path;
    int status;
    size_t written_bytes;
};

static void* file_worker_thread(void* arg) {
    WorkerTask* task = static_cast<WorkerTask*>(arg);
    log_line("Старт: " + task->input_path + " -> " + task->output_path);

    task->status = process_one_file(task->input_path, task->output_path, &task->written_bytes);  // Один worker обрабатывает одну пару in/out

    if (lock_with_timeout(&g_stats_mutex, 300)) {
        if (task->status == 0) {
            g_stats.completed++;
            g_stats.bytes_written += task->written_bytes;
        } else if (task->status == 130) {
            g_stats.interrupted++;
        } else {
            g_stats.failed++;
        }
        pthread_mutex_unlock(&g_stats_mutex);
    } else {
        fprintf(stderr, "[WARN] Таймаут захвата stats_mutex\n");
    }

    if (task->status == 0) {
        log_line("Готово: " + task->output_path);
    } else if (task->status == 130) {
        log_line("Прервано: " + task->output_path);
    } else {
        log_line("Ошибка: " + task->output_path);
    }

    return nullptr;
}

static void print_usage(const char* prog) {
    fprintf(stderr, "Одиночный режим: %s <входной_файл> <выходной_файл> <ключ>\n", prog);
    fprintf(stderr, "Многопоточный режим: %s <ключ> <in1> <out1> <in2> <out2> ...\n", prog);
}

int main(int argc, char* argv[]) {
    struct sigaction sa;
    memset(&sa, 0, sizeof(sa));
    sa.sa_handler = handle_sigint;
    sigemptyset(&sa.sa_mask);
    sigaction(SIGINT, &sa, nullptr);

    char key = 0;
    std::vector<WorkerTask> tasks;

    if (argc == 4) {  // Режим совместимости: один файл
        if (!parse_key(argv[3], &key)) {
            fprintf(stderr, "Ошибка: ключ должен быть 1 символом или числом 0..255\n");
            return 1;
        }
        WorkerTask task;
        task.input_path = argv[1];
        task.output_path = argv[2];
        task.status = 1;
        task.written_bytes = 0;
        tasks.push_back(task);
    } else if (argc >= 6 && (argc % 2 == 0)) {  // Расширенный режим: несколько файлов
        // Формат: secure_copy <key> <in1> <out1> <in2> <out2> ...
        if (!parse_key(argv[1], &key)) {
            fprintf(stderr, "Ошибка: ключ должен быть 1 символом или числом 0..255\n");
            return 1;
        }
        for (int i = 2; i < argc; i += 2) {
            WorkerTask task;
            task.input_path = argv[i];
            task.output_path = argv[i + 1];
            task.status = 1;
            task.written_bytes = 0;
            tasks.push_back(task);
        }
    } else {
        print_usage(argv[0]);
        return 1;
    }

    set_key(key);  // Один ключ применяется ко всем файлам текущего запуска

    pthread_mutex_init(&g_log_mutex, nullptr);
    pthread_mutex_init(&g_stats_mutex, nullptr);
    g_log_file = fopen("secure_copy.log", "a");

    g_stats.total_files = tasks.size();
    g_stats.completed = 0;
    g_stats.failed = 0;
    g_stats.interrupted = 0;
    g_stats.bytes_written = 0;
    // Общая статистика по всем worker-потокам

    log_line("Запуск secure_copy");

    std::vector<pthread_t> workers(tasks.size());  // По одному worker-потоку на файл (параллельная обработка)
    std::vector<bool> worker_created(tasks.size(), false);

    for (size_t i = 0; i < tasks.size(); ++i) {
        if (pthread_create(&workers[i], nullptr, file_worker_thread, &tasks[i]) == 0) {
            worker_created[i] = true;
        } else {
            tasks[i].status = 1;
            // При ошибке старта потока учитываем задачу как failed
            if (lock_with_timeout(&g_stats_mutex, 300)) {
                g_stats.failed++;
                pthread_mutex_unlock(&g_stats_mutex);
            }
            log_line("Ошибка создания потока задачи");
        }
    }

    for (size_t i = 0; i < tasks.size(); ++i) {
        if (worker_created[i]) {
            pthread_join(workers[i], nullptr);  // Корректное завершение всех потоков
        }
    }
    // Здесь все worker-потоки уже завершены

    if (!keep_running) {
        printf("Операция прервана пользователем\n");
    }

    printf("Файлов всего: %zu\n", g_stats.total_files);
    printf("Успешно: %zu\n", g_stats.completed);
    printf("С ошибкой: %zu\n", g_stats.failed);
    printf("Прервано: %zu\n", g_stats.interrupted);
    printf("Записано байт: %zu\n", g_stats.bytes_written);

    log_line("Завершение secure_copy");

    if (g_log_file) {
        fclose(g_log_file);
        g_log_file = nullptr;
    }
    pthread_mutex_destroy(&g_log_mutex);
    pthread_mutex_destroy(&g_stats_mutex);

    if (!keep_running) {
        // Возвращаем стандартный код прерывания по Ctrl+C
        return 130;
    }
    return (g_stats.failed == 0) ? 0 : 1;  // 0 если нет ошибок, иначе 1
}
