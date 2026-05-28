// libcaesar - RC4 и защищенное хранение ключа

#include <pthread.h>
#include <sys/mman.h> // подклеючение системного вызова для выделения страниц виртуальной памяти под ключ
#include <unistd.h> // узнать ращмер страницы ОС

#include <cstdio> // для чтения и записи файлов
#include <cstring> // для записи ключа в выделенную память и очистки    

#ifdef __cplusplus // для линковки с C кодом, чтобы избежать name mangling (смена имен функций компилятором C++)
extern "C" {
#endif

static unsigned char* g_key_page = nullptr; // Выделяем отдельную страницу памяти для хранения ключа
static size_t g_page_size = 0; // Размер страницы ОС
static pthread_mutex_t g_key_mutex = PTHREAD_MUTEX_INITIALIZER; // Защита операций с ключом
static size_t g_master_key_len = 0; // Длина мастер-ключа для RC4

static size_t get_page_size() {
    // Размер страницы нужен для корректного mmap/mprotect
    long size = sysconf(_SC_PAGESIZE);
    if (size <= 0) {
        // если sysconf недоступен и мы не получили валидный размер страницы ОС - берем дефолтное значение
        return 4096;
    }
    return static_cast<size_t>(size);
}

static bool ensure_key_page() {
    // Выделяем страницу под ключ только один раз
    if (g_key_page) {
        return true;
    }

    g_page_size = get_page_size();
    // Выделяем отдельную страницу памяти под ключ
    void* addr = mmap(nullptr, g_page_size, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
    if (addr == MAP_FAILED) {
        // mmap не выделил страницу — работу с ключом продолжать нельзя
        g_key_page = nullptr;
        g_page_size = 0;
        return false;
    }

    g_key_page = static_cast<unsigned char*>(addr); // сохраняем указатель и размер страницы для дальнейшего использования
    memset(g_key_page, 0, g_page_size); // обнуляем выделенную страницу для безопасности

    // После инициализации запрещаем доступ к странице
    if (mprotect(g_key_page, g_page_size, PROT_NONE) != 0) {
        // Не удалось закрыть доступ к странице
        munmap(g_key_page, g_page_size); // освобождаем выделенную память, так как она не защищена
        g_key_page = nullptr;
        g_page_size = 0;
        return false;
    }

#ifdef MADV_DONTDUMP
    // Просим ОС не включать страницу с ключом в дампы памяти, тк дамп - снимок памяти процесса, и если он будет создан (например, при падении), то злоумышленник может извлечь ключ из дампа
    madvise(g_key_page, g_page_size, MADV_DONTDUMP);
#endif
    return true;
}

int set_master_key(const unsigned char* key, size_t len) {
    // Мастер-ключ хранится в защищенной странице памяти
    if (key == nullptr || len == 0) {
        // Пустой ключ недопустим
        return 1;
    }

    pthread_mutex_lock(&g_key_mutex); // Защищаем операции с ключом от гонок в многопоточной среде, чтобы избежать одновременного изменения ключа несколькими потоками, что может привести к повреждению данных или утечке ключа
    if (!ensure_key_page()) {
        // Не удалось создать защищенную страницу
        pthread_mutex_unlock(&g_key_mutex);
        return 1;
    }

    if (len > g_page_size) {
        // Ключ не помещается в одну страницу
        pthread_mutex_unlock(&g_key_mutex);
        return 1;
    }

    if (mprotect(g_key_page, g_page_size, PROT_READ | PROT_WRITE) != 0) {
        // Не получилось временно открыть запись
        pthread_mutex_unlock(&g_key_mutex);
        return 1;
    }

    memcpy(g_key_page, key, len); // копируем ключ в выделенную страницу
    g_master_key_len = len;
    mprotect(g_key_page, g_page_size, PROT_NONE);
    pthread_mutex_unlock(&g_key_mutex);
    return 0;
}

void clear_key() {
    pthread_mutex_lock(&g_key_mutex); // опять делаем операции с ключом атомарными, чтобы избежать гонок при очистке ключа несколькими потоками
    if (g_key_page) {
        // Перед освобождением затираем страницу с ключом
        mprotect(g_key_page, g_page_size, PROT_READ | PROT_WRITE); // убедились, что у нас есть доступ к странице, чтобы очистить ключ
        memset(g_key_page, 0, g_page_size); // затираем ключ нулями для безопасности, чтобы он не остался в памяти после освобождения
        // Возвращаем защиту, затем освобождаем страницу
        mprotect(g_key_page, g_page_size, PROT_NONE); // убираем доступ к странице, чтобы предотвратить дальнейший доступ к ней после очистки
        munmap(g_key_page, g_page_size); // освобождаем выделенную память, так как она больше не нужна
        g_key_page = nullptr;
        g_page_size = 0;
        g_master_key_len = 0;
    }
    pthread_mutex_unlock(&g_key_mutex); // разблокируем мьютекс после завершения операций с ключом
}

int is_key_page_address(const void* addr) {
    if (!g_key_page || g_page_size == 0) { // Если страница ключа не инициализирована или ее размер равен 0, то никакой адрес не может быть адресом страницы ключа
        return 0;
    }
    const unsigned char* ptr = static_cast<const unsigned char*>(addr); // Проверяем, находится ли указанный адрес в пределах выделенной страницы для ключа
    // Используется обработчиком SIGSEGV для различения доступа к ключу и остальной памяти, чтобы при попытке доступа к странице ключа можно было отреагировать на это событие и предотвратить утечку ключа
    return (ptr >= g_key_page) && (ptr < (g_key_page + g_page_size));
}

struct Rc4State {
    // Внутреннее состояние RC4: массив и два индекса
    unsigned char s[256]; // массив перестановки байтов откуда берется гамма RC4
    unsigned char i; // индекс i для генерации гаммы - какой байт массива s использовать для генерации гаммы
    unsigned char j; // индекс j
};

static void rc4_init(Rc4State* state, const unsigned char* salt, size_t salt_len) {
    // Инициализация RC4 на ключе (master_key + salt)
    for (int i = 0; i < 256; ++i) {
        state->s[i] = static_cast<unsigned char>(i); // заполняем массив s начальными значениями от 0 до 255
    }

    state->i = 0;
    state->j = 0;

    size_t total_len = g_master_key_len + salt_len; // общая длина ключа для RC4 - мастер-ключ + соль
    unsigned char j = 0;

    for (int i = 0; i < 256; ++i) {
        size_t key_index = static_cast<size_t>(i) % total_len; // определяем, какой байт использовать для генерации гаммы - из мастер-ключа или из соли, в зависимости от текущего индекса i
        unsigned char key_byte = 0;
        if (key_index < g_master_key_len) { // берем байт из мастер-ключа, если индекс меньше длины мастер-ключа, иначе берем байт из соли
            key_byte = g_key_page[key_index];
        } else {
            key_byte = salt[key_index - g_master_key_len];
        }
        j = static_cast<unsigned char>(j + state->s[i] + key_byte); // обновляем индекс j на основе текущего байта массива s и байта ключа
        unsigned char tmp = state->s[i]; // сохраняем временное значение для перестановки
        state->s[i] = state->s[j]; // меняем местами элементы s[i] и s[j]
        state->s[j] = tmp; // завершаем перестановку
    }
}

static void rc4_apply(Rc4State* state, unsigned char* data, size_t len) {
    // Генерация гаммы RC4 и XOR с данными
    unsigned char i = state->i; // индексы i и j сохраняются в состоянии, чтобы продолжать генерацию гаммы при последующих вызовах для потокового шифрования
    unsigned char j = state->j;

    for (size_t n = 0; n < len; ++n) {
        i = static_cast<unsigned char>(i + 1); // увеличиваем индекс i на 1 по модулю 256 для каждого байта данных, чтобы последовательно генерировать гамму
        j = static_cast<unsigned char>(j + state->s[i]);
        unsigned char tmp = state->s[i]; // сохраняем временное значение для перестановки
        state->s[i] = state->s[j]; // меняем местами элементы s[i] и s[j]
        state->s[j] = tmp; // завершаем перестановку
        unsigned char k = state->s[static_cast<unsigned char>(state->s[i] + state->s[j])]; // генерируем следующий байт гаммы
        data[n] ^= k; // XOR данных с байтом гаммы для шифрования/расшифровки
    }

    state->i = i; // сохраняем обновленные индексы i и j обратно в состояние для продолжения генерации гаммы при следующих вызовах
    state->j = j;
}

int rc4_crypt_stream(FILE* input, FILE* output, const unsigned char* salt, size_t salt_len, size_t total_len) {
    // Потоковое шифрование/расшифровка файла с RC4
    if (input == nullptr || output == nullptr || salt == nullptr || salt_len == 0) {
        // Невалидные параметры шифрования
        return 1;
    }

    if (g_key_page == nullptr || g_master_key_len == 0) {
        // Мастер-ключ не установлен
        return 1;
    }

    if (g_master_key_len + salt_len == 0) {
        return 1;
    }

    // Выделяем отдельную страницу памяти для хранения s-box (состояния RC4)
    size_t state_page_size = get_page_size();
    void* state_page = mmap(nullptr, state_page_size, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0); // MAP_PRIVATE - страница приватная для процесса, MAP_ANONYMOUS - не связана с файлом, используется для выделения анонимной памяти, -1 - дескриптор, 0 - смещение
    if (state_page == MAP_FAILED) {
        return 1;
    }
    memset(state_page, 0, state_page_size); // обнуляем страницу для безопасности
    if (mprotect(state_page, state_page_size, PROT_NONE) != 0) { // закрыли память
        munmap(state_page, state_page_size); // освобождаем выделенную память, еслиие защитилась
        return 1;
    }



    Rc4State* state = static_cast<Rc4State*>(state_page); // состояние RC4 хранится в защищенной странице
    int result = 0; // переменная для хранения результата операции, 0 - успех, 1 - ошибка


    // открываем для использования s-box
    if (mprotect(state_page, state_page_size, PROT_READ | PROT_WRITE) != 0) {
        munmap(state_page, state_page_size);
        return 1;
    }

    pthread_mutex_lock(&g_key_mutex);
    if (mprotect(g_key_page, g_page_size, PROT_READ) != 0) {
        // Не удалось безопасно открыть доступ к ключу
        pthread_mutex_unlock(&g_key_mutex);
        mprotect(state_page, state_page_size, PROT_NONE);
        munmap(state_page, state_page_size);
        return 1;
    }
    rc4_init(state, salt, salt_len); // инициализируем состояние RC4 на основе мастер-ключа и соли, чтобы подготовиться к шифрованию данных
    pthread_mutex_unlock(&g_key_mutex);

    unsigned char buffer[4096];
    size_t remaining = total_len; // количество байт, которые осталось обработать, для корректного завершения шифрования

    while (remaining > 0) {
        size_t chunk = (remaining > sizeof(buffer)) ? sizeof(buffer) : remaining; // определяем размер текущего блока данных для обработки, не превышая размер буфера
        size_t read_bytes = fread(buffer, 1, chunk, input); // читаем данные из входного потока в буфер для последующего шифрования
        if (read_bytes == 0) {
            if (ferror(input)) {
                // Ошибка чтения входного потока
                result = 1;
                break;
            }
            result = 1;
            break;
        }

        rc4_apply(state, buffer, read_bytes); // применяем RC4 к прочитанным данным в буфере, чтобы зашифровать или расшифровать их

        size_t written = fwrite(buffer, 1, read_bytes, output); // записываем зашифрованные данные в выходной поток (запись в файл)
        if (written != read_bytes) {
            // Ошибка записи выходного потока
            result = 1;
            break;
        }

        remaining -= read_bytes;
    }

    pthread_mutex_lock(&g_key_mutex);
    mprotect(g_key_page, g_page_size, PROT_NONE);
    pthread_mutex_unlock(&g_key_mutex);

    mprotect(state_page, state_page_size, PROT_READ | PROT_WRITE);
    memset(state_page, 0, state_page_size);
    mprotect(state_page, state_page_size, PROT_NONE);
    munmap(state_page, state_page_size);

    return result;
}

#ifdef __cplusplus
}
#endif
