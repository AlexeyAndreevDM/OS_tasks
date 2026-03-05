#include <pthread.h>
#include <signal.h>

#include <cerrno>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <queue>
#include <string>
#include <vector>

#include "caesar.h"

static volatile sig_atomic_t keep_running = 1;

static void handle_sigint(int) {
    keep_running = 0;
}

struct DataBlock {
    std::vector<unsigned char> bytes;
};

struct SharedState {
    FILE* input_file;
    FILE* output_file;
    std::queue<DataBlock> blocks; // Ограниченная очередь между потоками
    size_t max_blocks;
    bool producer_done;
    bool has_error;
    std::string error_message;
    pthread_mutex_t mutex;
    pthread_cond_t can_produce;
    pthread_cond_t can_consume;
};

static void set_error(SharedState* state, const char* message) {
    state->has_error = true;
    state->error_message = message;
    pthread_cond_broadcast(&state->can_produce);
    pthread_cond_broadcast(&state->can_consume);
}

static void* producer_thread(void* arg) {
    SharedState* state = static_cast<SharedState*>(arg);
    unsigned char temp[4096]; // Размер буфера 4096 по условию

    while (keep_running) {
        const size_t bytes_read = fread(temp, 1, sizeof(temp), state->input_file); // Producer читает файл

        if (bytes_read > 0) {
            caesar(temp, temp, static_cast<int>(bytes_read)); // Шифрование через библиотеку из задания 1

            DataBlock block;
            block.bytes.assign(temp, temp + bytes_read); // копирует диапазон (блок) байтов в vector, тк temp потом обнуляется

            pthread_mutex_lock(&state->mutex);
            while (keep_running && !state->has_error && state->blocks.size() >= state->max_blocks) {
                pthread_cond_wait(&state->can_produce, &state->mutex);
            }

            if (!keep_running || state->has_error) {
                pthread_mutex_unlock(&state->mutex);
                break;
            }

            state->blocks.push(std::move(block));
            pthread_cond_signal(&state->can_consume);
            pthread_mutex_unlock(&state->mutex);
        }

        if (bytes_read < sizeof(temp)) {
            pthread_mutex_lock(&state->mutex);
            if (ferror(state->input_file)) {
                set_error(state, "Ошибка чтения входного файла");
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
    SharedState* state = static_cast<SharedState*>(arg);

    while (true) {
        pthread_mutex_lock(&state->mutex);

        while (!state->has_error && keep_running && state->blocks.empty() && !state->producer_done) {
            pthread_cond_wait(&state->can_consume, &state->mutex);
        }

        if (state->has_error || (state->blocks.empty() && (!keep_running || state->producer_done))) {
            pthread_mutex_unlock(&state->mutex);
            break;
        }

        DataBlock block = std::move(state->blocks.front());
        state->blocks.pop();
        pthread_cond_signal(&state->can_produce);
        pthread_mutex_unlock(&state->mutex);

        const size_t to_write = block.bytes.size();
        size_t total_written = 0;
        while (total_written < to_write) {
            const size_t written =
                fwrite(block.bytes.data() + total_written, 1, to_write - total_written, state->output_file); // Consumer пишет файл
            if (written == 0) {
                pthread_mutex_lock(&state->mutex);
                set_error(state, "Ошибка записи выходного файла");
                pthread_mutex_unlock(&state->mutex);
                return nullptr;
            }
            total_written += written;
        }
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

int main(int argc, char* argv[]) {
    // Проверка аргументов командной строки
    if (argc != 4) {
        fprintf(stderr, "Использование: %s <входной_файл> <выходной_файл> <ключ>\n", argv[0]);
        return 1;
    }

    char key = 0;
    if (!parse_key(argv[3], &key)) {
        fprintf(stderr, "Ошибка: ключ должен быть 1 символом или числом 0..255\n");
        return 1;
    }

    struct sigaction sa;
    memset(&sa, 0, sizeof(sa));
    sa.sa_handler = handle_sigint;
    sigemptyset(&sa.sa_mask);
    sigaction(SIGINT, &sa, nullptr); // Обработка Ctrl+C по условию

    FILE* input_file = fopen(argv[1], "rb");
    if (!input_file) {
        fprintf(stderr, "Ошибка открытия входного файла: %s\n", argv[1]);
        return 1;
    }

    FILE* output_file = fopen(argv[2], "wb");
    if (!output_file) {
        fprintf(stderr, "Ошибка открытия выходного файла: %s\n", argv[2]);
        fclose(input_file);
        return 1;
    }

    set_key(key);

    SharedState state;
    state.input_file = input_file;
    state.output_file = output_file;
    state.max_blocks = 4;
    state.producer_done = false;
    state.has_error = false;
    pthread_mutex_init(&state.mutex, nullptr);
    pthread_cond_init(&state.can_produce, nullptr);
    pthread_cond_init(&state.can_consume, nullptr);

    pthread_t producer;
    pthread_t consumer;

    // 2 рабочих потока по условию
    if (pthread_create(&producer, nullptr, producer_thread, &state) != 0) {
        fprintf(stderr, "Ошибка создания потока producer\n");
        fclose(input_file);
        fclose(output_file);
        pthread_mutex_destroy(&state.mutex);
        pthread_cond_destroy(&state.can_produce);
        pthread_cond_destroy(&state.can_consume);
        return 1;
    }

    if (pthread_create(&consumer, nullptr, consumer_thread, &state) != 0) {
        fprintf(stderr, "Ошибка создания потока consumer\n");
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

    // Утечки предотвращаются: через join потоков, файлы закрываются, mutex/cond освобождаются в любом сценарии
    fclose(input_file);
    fclose(output_file);
    pthread_mutex_destroy(&state.mutex);
    pthread_cond_destroy(&state.can_produce);
    pthread_cond_destroy(&state.can_consume);

    if (state.has_error) {
        fprintf(stderr, "%s\n", state.error_message.c_str());
        return 1;
    }

    if (!keep_running) {
        // при Ctrl+C удаляем неполный dst-файл
        remove(argv[2]);
        printf("Операция прервана пользователем\n");
        return 130;
    }

    return 0;
}
