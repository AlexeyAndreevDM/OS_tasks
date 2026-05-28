/**
 * 4 байта - размер данных файлов
 * 4 байта - размер имени
 * 16 байт - соль
 * N байт - имя файла
 * M байт - зашифрованные данные файла
 */


#include <pthread.h>
#include <signal.h> // для обработки сигнала SIGSEGV при попытке доступа к странице ключа
#include <unistd.h> // для sysconf и _exit, exit - для безопасного завершения процесса при нарушении доступа к ключу

#include <cstdint> // для uint32_t при работе с форматом образа, чтобы обеспечить переносимость и точное определение размера данных
#include <cstdio> // для работы с файлами и потоками ввода-вывода
#include <cstdlib> // для malloc, free и exit, malloc - для динамического выделения памяти при работе с файлами и буферами, динамическое выделение может потребоваться при работе с большими файлами или при чтении переменных по размеру данных из образа
#include <cstring> // для memcpy и memset при работе с буферами и страницей ключа, memcpy - для копирования данных в выделенную страницу ключа, memset - для очистки страницы ключа при освобождении, чтобы предотвратить утечку данных
#include <algorithm> // для std::sort при сортировке списка файлов в образе
#include <string> // для удобной работы со строками при обработке имен файлов и путей
#include <vector> // для хранения списка файлов при работе с образом

#include <dirent.h> // для работы с директориями при сборе файлов для образа, DIR, struct dirent и функции opendir, readdir, closedir - для обхода директорий и получения списка файлов для добавления в образ
#include <sys/stat.h> // для получения информации о файлах при сборе файлов для образа, struct stat и функция stat - для получения размера файлов и проверки типа (файл или директория) при сборе файлов для образа

#include "caesar.h" // наша библиотека шифрования RC4

static void handle_sigsegv(int sig, siginfo_t* info, void*) { // вызывается при попытке доступа к странице ключа, чтобы предотвратить утечку ключа
    if (info && is_key_page_address(info->si_addr)) {
        // Явное нарушение доступа к странице ключа
        const char message[] = "[SECURITY] Попытка доступа к защищенному ключу\n";
        write(STDERR_FILENO, message, sizeof(message) - 1);
        _exit(1);
    }

    // Для остальных адресов возвращаем стандартное поведение
    struct sigaction sa; // восстанавливаем стандартную обработку сигнала, чтобы не нарушать работу программы при других ошибках доступа к памяти
    memset(&sa, 0, sizeof(sa)); // обнуляем структуру sigaction для корректной инициализации
    sa.sa_handler = SIG_DFL; // устанавливаем обработчик по умолчанию для сигнала, чтобы при повторном возникновении сигнала программа завершилась стандартным образом
    sigemptyset(&sa.sa_mask); // очищаем маску сигналов, чтобы не блокировать другие сигналы при обработке
    sigaction(sig, &sa, nullptr); // восстанавливаем стандартную обработку сигнала
    raise(sig); // повторно посылаем сигнал, чтобы программа завершилась стандартным образом при других ошибках доступа к памяти
}


struct ImageEntry { // структура для хранения информации о файлах в образе, которая будет использоваться при чтении и записи образа, а также для сортировки файлов по имени
    std::string name;
    uint32_t size;
};

struct AddJob { // структура для хранения информации о файлах, которые нужно добавить в образ, используется при сборе файлов из директории и их последующей обработке для добавления в образ
    std::string source_path;
    std::string stored_name;
};

static bool write_u32_le(FILE* file, uint32_t value) { // вспомогательная функция для записи 32-битного числа в формате little-endian, которая используется при записи заголовков файлов в образ, чтобы обеспечить совместимость с форматом образа и корректное чтение при извлечении файлов из образа
    unsigned char buf[4]; // буфер для хранения байтов числа, который будет записан в файл, использование буфера позволяет корректно формировать представление числа в нужном формате перед записью
    buf[0] = static_cast<unsigned char>(value & 0xFFu); // записываем младший байт числа в первый элемент буфера
    buf[1] = static_cast<unsigned char>((value >> 8) & 0xFFu);
    buf[2] = static_cast<unsigned char>((value >> 16) & 0xFFu);
    buf[3] = static_cast<unsigned char>((value >> 24) & 0xFFu);
    return fwrite(buf, 1, sizeof(buf), file) == sizeof(buf); // записываем буфер в файл и проверяем, что запись прошла успешно, возвращаем true при успешной записи и false при ошибке
}

static bool read_u32_le(FILE* file, uint32_t* value, bool* eof) { // вспомогательная функция для чтения 32-битного числа в формате little-endian, которая используется при чтении заголовков файлов из образа
    unsigned char buf[4]; // буфер для хранения байтов числа в формате little-endian, который будет прочитан из файла, использование буфера позволяет корректно формировать представление числа в нужном формате после чтения
    size_t read_bytes = fread(buf, 1, sizeof(buf), file);
    if (read_bytes == 0 && feof(file)) {
        // Достигнут конец файла без ошибки чтения
        *eof = true;
        return false;
    }
    if (read_bytes != sizeof(buf)) {
        // Поврежденный/неполный заголовок
        *eof = false;
        return false;
    }

    *eof = false;
    *value = static_cast<uint32_t>(buf[0]) | // формируем число из байтов в формате little-endian, используя побитовые операции для корректного объединения байтов в 32-битное число
             (static_cast<uint32_t>(buf[1]) << 8) |
             (static_cast<uint32_t>(buf[2]) << 16) |
             (static_cast<uint32_t>(buf[3]) << 24);
    return true;
}

static bool read_exact(FILE* file, void* buffer, size_t len) {
    // При неполном чтении считаем образ поврежденным
    return fread(buffer, 1, len, file) == len;
}

static bool write_exact(FILE* file, const void* buffer, size_t len) {
    // Любая недозапись критична для целостности образа
    return fwrite(buffer, 1, len, file) == len;
}

static bool path_is_dir(const std::string& path) {
    // Проверка, является ли путь директорией
    struct stat st;
    if (stat(path.c_str(), &st) != 0) {
        return false;
    }
    return S_ISDIR(st.st_mode);
}

static std::string trim_trailing_slashes(const std::string& path) {
    // Нормализация пути для корректной обработки базового имени директории, удаляя лишние слэши в конце пути, чтобы избежать проблем при формировании относительных путей внутри образа
    size_t end = path.size();
    while (end > 1 && path[end - 1] == '/') {
        --end; // уменьшаем индекс конца строки, пока не достигнем первого символа, который не является слэшем, или не останется один символ (для корневого пути "/")
    }
    return path.substr(0, end); // возвращаем подстроку от начала до найденного индекса, которая будет использоваться для получения базового имени директории и формирования относительных путей внутри образа
}

static std::string basename_of(const std::string& path) {
    // Берем имя корневой директории для записи в образ
    std::string clean = trim_trailing_slashes(path); // удаляем лишние слэши в конце пути, чтобы корректно получить базовое имя директории, особенно если путь заканчивается на слэш, что может привести к неправильному определению базового имени
    size_t pos = clean.find_last_of('/'); // находим позицию последнего слэша в пути, чтобы отделить базовое имя от остальной части пути, если слэш не найден, то базовым именем будет весь путь
    if (pos == std::string::npos) { // слэш не найден, возвращаем весь путь как базовое имя
        return clean;
    }
    return clean.substr(pos + 1); // возращаем подстроку - базовое имя директориии
}

static bool collect_from_dir(const std::string& root, const std::string& base_name, const std::string& current, std::vector<AddJob>& jobs) {
    // Рекурсивный сбор файлов с относительными путями внутри выбранной директории
    DIR* dir = opendir(current.c_str());
    if (!dir) {
        // Нельзя открыть директорию для обхода
        return false;
    }

    struct dirent* entry = nullptr; // структура для хранения информации о текущем файле или директории при обходе, которая будет использоваться для получения имен файлов и определения их типа (файл или директория) при сборе файлов для образа
    while ((entry = readdir(dir)) != nullptr) {
        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) { // пропускаем текущую и родительскую директории, чтобы избежать бесконечной рекурсии при обходе директорий
            continue;
        }

        std::string full_path = current + "/" + entry->d_name;
        struct stat st;
        if (stat(full_path.c_str(), &st) != 0) {
            // Ошибка - не удалось заполнить струтуру stat: прекращаем сбор, чтобы не пропустить файл
            closedir(dir);
            return false;
        }

        if (S_ISDIR(st.st_mode)) { // если текущий элемент является директорией, рекурсивно вызываем функцию для сбора файлов внутри этой директории, передавая обновленный путь и базовое имя для формирования относительных путей внутри образа
            if (!collect_from_dir(root, base_name, full_path, jobs)) {
                // Прерываемся при ошибке в поддиректории
                closedir(dir);
                return false;
            }
        } else if (S_ISREG(st.st_mode)) { // если текущий элемент является обычным файлом, добавляем его в список заданий для добавления в образ, формируя относительный путь внутри образа на основе базового имени и пути к файлу относительно корневой директории
            std::string relative = full_path.substr(root.size()); // формируем относительный путь внутри образа, удаляя корневой путь из полного пути к файлу, чтобы сохранить структуру директорий внутри образа и избежать дублирования корневого пути в именах файлов внутри образа
            if (!relative.empty() && relative[0] == '/') {
                relative.erase(0, 1);
            }
            AddJob job;
            job.source_path = full_path;
            job.stored_name = base_name + "/" + relative;
            jobs.push_back(job); // добавляем задание на добавление файла в образ в общий список заданий, который будет использоваться для обработки и добавления файлов в образ, обеспечивая сохранение структуры директорий внутри образа
        }
    }

    closedir(dir);
    return true;
}


static bool read_salt(unsigned char* salt, size_t len) {
    // Соль генерируется для каждого файла отдельно
    FILE* rnd = fopen("/dev/urandom", "rb");
    if (!rnd) {
        return false;
    }
    bool ok = fread(salt, 1, len, rnd) == len; // читаем случайные байты из /dev/urandom для использования в качестве соли при шифровании каждого файла, соль обеспечивает уникальность шифрования для каждого файла
    fclose(rnd);
    return ok;
}

struct AddContext {
    // Контекст общего очередного списка заданий для потоков
    std::vector<AddJob> jobs;
    size_t next_index;
    pthread_mutex_t jobs_mutex;
    pthread_mutex_t image_mutex;
    FILE* image_file;
    std::string image_path;
    int errors;
    int skipped;
};

static int image_contains_name(const std::string& image_path, const std::string& target_name) {
    // Проверка на дубликаты имен в образе
    FILE* file = fopen(image_path.c_str(), "rb");
    if (!file) {
        return -1;
    }

    while (true) {
        bool eof = false;
        uint32_t file_len = 0; // читаем длину файла из заголовка образа, чтобы знать, сколько байт данных нужно пропустить после чтения имени и соли, если имя не совпало, для корректного обхода образа и проверки всех записей на наличие дубликатов имен
        if (!read_u32_le(file, &file_len, &eof)) { // читаем длину файла из заголовка образа, если достигнут конец файла, то выходим из цикла, так как все записи были проверены, если произошла ошибка чтения, то считаем образ поврежденным и возвращаем ошибку
            if (eof) {
                break;
            }
            fclose(file);
            return -1;
        }

        uint32_t name_len = 0;
        if (!read_u32_le(file, &name_len, &eof) || eof) { // читаем длину имени из заголовка образа, если достигнут конец файла, то считаем образ поврежденным, так как запись неполная, если произошла ошибка чтения, то также считаем образ поврежденным и возвращаем ошибку
            fclose(file);
            return -1;
        }

        unsigned char salt[16];
        if (!read_exact(file, salt, sizeof(salt))) { // читаем соль из заголовка образа, если произошла ошибка чтения, то считаем образ поврежденным и возвращаем ошибку
            fclose(file);
            return -1;
        }

        if (name_len > 65535) { // проверяем, что длина имени не превышает разумные пределы, чтобы предотвратить попытки атак на образ с целью создания очень длинных имен, которые могут привести к проблемам с памятью при чтении, если имя слишком длинное, то считаем образ поврежденным и возвращаем ошибку
            fclose(file);
            return -1;
        }

        std::string name(name_len, '\0');
        if (!read_exact(file, &name[0], name_len)) { // читаем имя файла из заголовка образа, если произошла ошибка чтения, то считаем образ поврежденным и возвращаем ошибку
            fclose(file);
            return -1;
        }

        if (name == target_name) { // если имя совпало с искомым, то закрываем файл и возвращаем 1, чтобы указать на наличие дубликата имени в образе
            fclose(file);
            return 1;
        }

        if (fseek(file, static_cast<long>(file_len), SEEK_CUR) != 0) { // пропускаем данные файла, если произошла ошибка при попытке пропустить данные, то считаем образ поврежденным и возвращаем ошибку, fseek - перестановка курсора в файле для пропуска данных, SEEK_CUR - относительно текущей позиции, static_cast<long> - приведение типа для корректной работы функции fseek с размером данных
            fclose(file);
            return -1;
        }
    }

    fclose(file);
    return 0;
}

static int append_record(FILE* image_file, const AddJob& job, uint32_t file_len, const unsigned char* salt, FILE* temp_file, AddContext* ctx) {
    // Запись заголовка + зашифрованных данных файла в образ
    uint32_t name_len = static_cast<uint32_t>(job.stored_name.size());

    pthread_mutex_lock(&ctx->image_mutex);

    int contains = image_contains_name(ctx->image_path, job.stored_name);
    if (contains == 1) {
        // Пропускаем дубликат имени
        pthread_mutex_unlock(&ctx->image_mutex);
        return 2;
    }
    if (contains == -1) {
        // Ошибка чтения образа при проверке дубликатов
        pthread_mutex_unlock(&ctx->image_mutex);
        return 1;
    }

    if (!write_u32_le(image_file, file_len) || // записываем длину файла в заголовок образа, если произошла ошибка записи, то возвращаем ошибку, write_u32_le - вспомогательная функция для записи 32-битного числа в формате little-endian, которая используется для обеспечения совместимости с форматом образа и корректного чтения при извлечении файлов из образа
        !write_u32_le(image_file, name_len) ||
        !write_exact(image_file, salt, 16) ||
        !write_exact(image_file, job.stored_name.data(), name_len)) {
        // Не удалось записать заголовок записи
        pthread_mutex_unlock(&ctx->image_mutex);
        return 1;
    }

    unsigned char buffer[4096];
    size_t read_bytes = 0;
    while ((read_bytes = fread(buffer, 1, sizeof(buffer), temp_file)) > 0) {
        if (fwrite(buffer, 1, read_bytes, image_file) != read_bytes) {
            // Ошибка записи зашифрованных данных в образ
            pthread_mutex_unlock(&ctx->image_mutex);
            return 1;
        }
    }

    if (ferror(temp_file)) {
        // Временный файл поврежден или недочитан
        pthread_mutex_unlock(&ctx->image_mutex);
        return 1;
    }

    fflush(image_file); // гарантируем, что данные записаны на диск, чтобы предотвратить потерю данных при сбое после записи, fflush - сброс буфера вывода в файл, чтобы обеспечить сохранение данных на диске
    pthread_mutex_unlock(&ctx->image_mutex);
    return 0;
}

static int process_add_job(AddContext* ctx, const AddJob& job) { // обработка одного задания на добавление файла в образ - открываем файл и передаем его на шифрование в process_add_job, который шифрует файл во временный поток
    // Шифруем файл во временный поток, затем добавляем в образ
    struct stat st;
    if (stat(job.source_path.c_str(), &st) != 0) {
        // Пропускаем недоступный файл
        return 1;
    }

    uint32_t file_len = static_cast<uint32_t>(st.st_size); // получаем размер файла для записи в заголовок образа, чтобы при извлечении файла из образа знать, сколько байт данных нужно прочитать для восстановления файла, static_cast<uint32_t> - приведение типа для обеспечения корректного представления размера файла в 32-битном формате, который используется в заголовке образа

    FILE* input = fopen(job.source_path.c_str(), "rb");
    if (!input) {
        // Не удалось открыть входной файл
        return 1;
    }

    FILE* temp_file = tmpfile();
    if (!temp_file) {
        // Нельзя создать временный файл для шифрования
        fclose(input);
        return 1;
    }

    unsigned char salt[16];
    if (!read_salt(salt, sizeof(salt))) {
        // Не удалось получить случайную соль
        fclose(input);
        fclose(temp_file);
        return 1;
    }

    if (rc4_crypt_stream(input, temp_file, salt, sizeof(salt), file_len) != 0) {
        // Ошибка шифрования потока
        fclose(input);
        fclose(temp_file);
        return 1;
    }

    fclose(input);
    fflush(temp_file); // гарантируем, что все данные записаны во временный файл, чтобы предотвратить проблемы при чтении данных для записи в образ, fflush - сброс буфера вывода в файл, чтобы обеспечить сохранение данных на диске
    fseek(temp_file, 0, SEEK_SET);

    int result = append_record(ctx->image_file, job, file_len, salt, temp_file, ctx);
    fclose(temp_file);
    return result;
}

static void* add_worker_thread(void* arg) {
    // Поток-воркер для параллельного добавления файлов (до 5 штук)
    AddContext* ctx = static_cast<AddContext*>(arg);

    while (true) {
        pthread_mutex_lock(&ctx->jobs_mutex);
        if (ctx->next_index >= ctx->jobs.size()) {
            // Все задания разобраны другими потоками
            pthread_mutex_unlock(&ctx->jobs_mutex);
            break;
        }
        AddJob job = ctx->jobs[ctx->next_index++];
        pthread_mutex_unlock(&ctx->jobs_mutex);

        int result = process_add_job(ctx, job);
        if (result == 1) {
            // Счетчик ошибок нужен для итогового кода возврата
            pthread_mutex_lock(&ctx->jobs_mutex);
            ctx->errors++;
            pthread_mutex_unlock(&ctx->jobs_mutex);
        } else if (result == 2) {
            // Дубликаты считаем пропущенными, не ошибкой
            pthread_mutex_lock(&ctx->jobs_mutex);
            ctx->skipped++;
            pthread_mutex_unlock(&ctx->jobs_mutex);
        }
    }

    return nullptr;
}

static int add_to_image(const std::string& image_path, const std::string& master_key, const std::vector<std::string>& inputs) {
    // Добавление файлов/директорий в образ с RC4 и индивидуальной солью
    std::vector<AddJob> jobs;

    for (const auto& input : inputs) {
        if (path_is_dir(input)) {
            std::string root = trim_trailing_slashes(input);
            std::string base_name = basename_of(root);
            if (!collect_from_dir(root, base_name, root, jobs)) {
                // Ошибка чтения директории — прекращаем добавление
                return 1;
            }
        } else {
            AddJob job;
            job.source_path = input;
            job.stored_name = input;
            jobs.push_back(job);
        }
    }

    if (jobs.empty()) {
        // Нечего добавлять
        return 1;
    }

    if (set_master_key(reinterpret_cast<const unsigned char*>(master_key.data()), master_key.size()) != 0) {
        // Некорректный мастер-ключ
        return 1;
    }

    FILE* image_file = fopen(image_path.c_str(), "ab+");
    if (!image_file) {
        // Не удалось создать или открыть образ
        return 1;
    }

    AddContext ctx;
    ctx.jobs = jobs;
    ctx.next_index = 0;
    ctx.image_file = image_file;
    ctx.errors = 0;
    ctx.skipped = 0;
    ctx.image_path = image_path;
    pthread_mutex_init(&ctx.jobs_mutex, nullptr);
    pthread_mutex_init(&ctx.image_mutex, nullptr);

    size_t thread_count = jobs.size();
    if (thread_count > 5) {
        thread_count = 5;
    }

    printf("mode: %s\n", (thread_count > 1) ? "parallel" : "sequential");
    printf("files order:\n");
    for (const auto& job : jobs) {
        printf("- %s\n", job.stored_name.c_str());
    }

    // Запускаем потоки для параллельного добавления файлов
    std::vector<pthread_t> threads(thread_count);
    for (size_t i = 0; i < thread_count; ++i) {
        pthread_create(&threads[i], nullptr, add_worker_thread, &ctx);
    }

    for (size_t i = 0; i < thread_count; ++i) {
        pthread_join(threads[i], nullptr);
    }

    pthread_mutex_destroy(&ctx.jobs_mutex);
    pthread_mutex_destroy(&ctx.image_mutex);
    fclose(image_file);

    size_t total = jobs.size();
    size_t skipped = (ctx.skipped >= 0 && static_cast<size_t>(ctx.skipped) <= total) ? static_cast<size_t>(ctx.skipped) : 0;
    size_t errors = (ctx.errors >= 0 && static_cast<size_t>(ctx.errors) <= total) ? static_cast<size_t>(ctx.errors) : 0;
    size_t success = (total >= skipped + errors) ? (total - skipped - errors) : 0;
    printf("written: %zu/%zu\n", success, total);
    if (skipped > 0) {
        printf("skipped: %zu\n", skipped);
    }
    return (ctx.errors == 0) ? 0 : 1;
}

static int list_image(const std::string& image_path) {
    // Чтение заголовков всех записей и сортировка по имени
    FILE* image_file = fopen(image_path.c_str(), "rb");
    if (!image_file) {
        // Образ недоступен для чтения
        return 1;
    }

    std::vector<ImageEntry> entries;

    while (true) {
        bool eof = false;
        uint32_t file_len = 0;
        if (!read_u32_le(image_file, &file_len, &eof)) {
            if (eof) {
                break;
            }
            // Нарушение формата заголовка
            fclose(image_file);
            return 1;
        }

        uint32_t name_len = 0;
        if (!read_u32_le(image_file, &name_len, &eof) || eof) {
            // Повреждено поле длины имени
            fclose(image_file);
            return 1;
        }

        unsigned char salt[16];
        if (!read_exact(image_file, salt, sizeof(salt))) {
            // Повреждены байты соли
            fclose(image_file);
            return 1;
        }

        if (name_len > 65535) {
            // Защита от некорректной длины имени
            fclose(image_file);
            return 1;
        }

        std::string name(name_len, '\0');
        if (!read_exact(image_file, &name[0], name_len)) {
            // Повреждено поле имени файла
            fclose(image_file);
            return 1;
        }

        if (fseek(image_file, static_cast<long>(file_len), SEEK_CUR) != 0) {
            // Не удалось пропустить тело файла
            fclose(image_file);
            return 1;
        }

        ImageEntry entry;
        entry.name = name;
        entry.size = file_len;
        entries.push_back(entry);
    }

    fclose(image_file);

    std::sort(entries.begin(), entries.end(), [](const ImageEntry& a, const ImageEntry& b) {
        return a.name < b.name;
    });

    for (const auto& entry : entries) {
        printf("%s %u\n", entry.name.c_str(), entry.size);
    }

    return 0;
}

static int get_from_image(const std::string& image_path, const std::string& master_key, const std::string& output_path, const std::string& target_name) {
    // Поиск записи в образе и расшифровка в указанный файл
    if (set_master_key(reinterpret_cast<const unsigned char*>(master_key.data()), master_key.size()) != 0) {
        // Некорректный мастер-ключ
        return 1;
    }

    FILE* image_file = fopen(image_path.c_str(), "rb");
    if (!image_file) {
        // Не удалось открыть образ
        return 1;
    }

    FILE* output_file = fopen(output_path.c_str(), "wb");
    if (!output_file) {
        // Не удалось создать выходной файл
        fclose(image_file);
        return 1;
    }

    bool found = false;

    while (true) {
        bool eof = false;
        uint32_t file_len = 0;
        if (!read_u32_le(image_file, &file_len, &eof)) {
            // Конец файла или повреждение заголовка
            break;
        }

        uint32_t name_len = 0;
        if (!read_u32_le(image_file, &name_len, &eof) || eof) {
            // Повреждено поле длины имени
            break;
        }

        unsigned char salt[16];
        if (!read_exact(image_file, salt, sizeof(salt))) {
            // Повреждены байты соли
            break;
        }

        if (name_len > 65535) {
            // Защита от некорректной длины имени
            break;
        }

        std::string name(name_len, '\0');
        if (!read_exact(image_file, &name[0], name_len)) {
            // Повреждено поле имени файла
            break;
        }

        if (name == target_name) {
            if (rc4_crypt_stream(image_file, output_file, salt, sizeof(salt), file_len) != 0) {
                // Ошибка расшифровки потока
                fclose(image_file);
                fclose(output_file);
                remove(output_path.c_str());
                return 1;
            }
            found = true;
            break;
        }

        if (fseek(image_file, static_cast<long>(file_len), SEEK_CUR) != 0) {
            // Не удалось пропустить тело файла
            break;
        }
    }

    fclose(image_file);
    fclose(output_file);

    if (!found) {
        // Указанного файла нет в образе
        remove(output_path.c_str());
        return 1;
    }

    return 0;
}

static void print_usage(const char* prog) { // инструкция, выводится когда нет выбранного режима (-add, -list, -get); переданы неизвестные опции; не хватает обязательных параметров для выбранного режима.
    fprintf(stderr, "Добавление в образ: %s -add -key <секрет> -image <image> <file/dir> ...\n", prog);
    fprintf(stderr, "Список файлов: %s -list -image <image>\n", prog);
    fprintf(stderr, "Извлечение файла: %s -get -image <image> -key <секрет> -out <file> <name>\n", prog);
}

int main(int argc, char* argv[]) {
    struct sigaction sa_segv; // настраиваем обработчик сигнала SIGSEGV для защиты страницы ключа от несанкционированного доступа, чтобы при попытке доступа к странице ключа программа не раскрывала содержимое ключа и завершалась безопасным образом, предотвращая утечку ключа при ошибках доступа к памяти
    memset(&sa_segv, 0, sizeof(sa_segv)); // обнуляем структуру sigaction чтобы корректно работал обработчик
    sa_segv.sa_sigaction = handle_sigsegv; // устанавливаем handler
    sigemptyset(&sa_segv.sa_mask); // очищаем маску сигналов, чтобы не блокировать другие сигналы при обработке SIGSEGV, sigemptyset - инициализация маски сигналов, которая используется для указания, какие сигналы должны быть заблокированы во время выполнения обработчика, в данном случае мы не блокируем никакие сигналы
    sa_segv.sa_flags = SA_SIGINFO; // указываем, что обработчик будет использовать расширенную информацию о сигнале, которая передается в виде аргумента siginfo_t*, чтобы мы могли определить, был ли доступ к странице ключа при возникновении SIGSEGV
    sigaction(SIGSEGV, &sa_segv, nullptr); // устанавливаем обработчикконкретно для сигнала SIGSEGV

    bool add_mode = false;
    bool list_mode = false;
    bool get_mode = false;
    std::string image_path;
    std::string master_key;
    std::string output_path;
    std::string target_name;
    std::vector<std::string> inputs;

    for (int i = 1; i < argc; ++i) {
        // Разбор CLI для работы с образом
        const char* arg = argv[i];
        if (strcmp(arg, "-add") == 0) {
            add_mode = true;
        } else if (strcmp(arg, "-list") == 0) {
            list_mode = true;
        } else if (strcmp(arg, "-get") == 0) {
            get_mode = true;
        } else if (strcmp(arg, "-image") == 0 && i + 1 < argc) {
            image_path = argv[++i];
        } else if (strcmp(arg, "-key") == 0 && i + 1 < argc) {
            master_key = argv[++i];
        } else if (strcmp(arg, "-out") == 0 && i + 1 < argc) {
            output_path = argv[++i];
        } else if (arg[0] == '-') {
            // Неизвестная опция
            print_usage(argv[0]); // выводим подсказку что вводиь
            return 1;
        } else {
            if (get_mode) {
                target_name = arg; // в режиме извлечения последний аргумент - это имя файла внутри образа, который нужно извлечь, сохраняем его для последующего поиска в образе
            } else {
                inputs.push_back(arg); // в режиме добавления все аргументы после опций - это файлы или директории, которые нужно добавить в образ, сохраняем их в список для последующей обработки и добавления в образ
            }
        }
    }

    if (!add_mode && !list_mode && !get_mode) {
        print_usage(argv[0]); // если не выбран режим работы
        return 1;
    }

    // Новый режим работы с образом
    if (add_mode) {
        if (image_path.empty() || master_key.empty() || inputs.empty()) {
            print_usage(argv[0]);
            return 1;
        }
        int result = add_to_image(image_path, master_key, inputs);
        clear_key();
        return result;
    }

    if (list_mode) {
        if (image_path.empty()) {
            print_usage(argv[0]);
            return 1;
        }
        int result = list_image(image_path);
        clear_key();
        return result;
    }

    if (get_mode) {
        if (image_path.empty() || master_key.empty() || output_path.empty() || target_name.empty()) {
            print_usage(argv[0]);
            return 1;
        }
        int result = get_from_image(image_path, master_key, output_path, target_name);
        clear_key();
        return result;
    }

    print_usage(argv[0]);
    return 1;
}