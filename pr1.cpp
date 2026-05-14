// libcaesar - Побайтовое XOR шифрование

#include <pthread.h>
#include <sys/mman.h> // подклеючение системного вызова для выделения страниц виртуальной памяти под ключ
#include <unistd.h>

#include <cstring>

#ifdef __cplusplus
extern "C" {
#endif

static unsigned char* g_key_page = nullptr; // Указатель на выделенную страницу для ключа
static size_t g_page_size = 0; // Размер страницы ОС
static pthread_mutex_t g_key_mutex = PTHREAD_MUTEX_INITIALIZER; // Защита операций с ключом

static size_t get_page_size() {
    long size = sysconf(_SC_PAGESIZE);
    if (size <= 0) {
        // если sysconf недоступен и мы не получили валидный размер страницы ОС - берем дефолтное значение
        return 4096;
    }
    return static_cast<size_t>(size);
}

static bool ensure_key_page() {
    if (g_key_page) {
        return true;
    }

    g_page_size = get_page_size();
    // Выделяем отдельную страницу памяти под ключ
    void* addr = mmap(nullptr, g_page_size, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
    if (addr == MAP_FAILED) {
        g_key_page = nullptr;
        g_page_size = 0;
        return false;
    }

    g_key_page = static_cast<unsigned char*>(addr);
    memset(g_key_page, 0, g_page_size);

    // После инициализации делаем страницу только для чтения
    if (mprotect(g_key_page, g_page_size, PROT_READ) != 0) {
        munmap(g_key_page, g_page_size);
        g_key_page = nullptr;
        g_page_size = 0;
        return false;
    }

#ifdef MADV_DONTDUMP
    // Просим ОС не включать страницу с ключом в дампы памяти
    madvise(g_key_page, g_page_size, MADV_DONTDUMP);
#endif
    return true;
}

void set_key(char key) {
    pthread_mutex_lock(&g_key_mutex);
    if (!ensure_key_page()) {
        pthread_mutex_unlock(&g_key_mutex);
        return;
    }

    // Временно открываем запись только на момент установки ключа
    if (mprotect(g_key_page, g_page_size, PROT_READ | PROT_WRITE) != 0) {
        pthread_mutex_unlock(&g_key_mutex);
        return;
    }

    g_key_page[0] = static_cast<unsigned char>(key);
    mprotect(g_key_page, g_page_size, PROT_READ);
    pthread_mutex_unlock(&g_key_mutex);
}

void caesar(void* src, void* dst, int len) {
    unsigned char* source = (unsigned char*)src;
    unsigned char* destination = (unsigned char*)dst;
    unsigned char key = 0;

    if (g_key_page) {
        // Читаем ключ из защищенной страницы
        key = g_key_page[0];
    }

    for (int i = 0; i < len; i++) {
        destination[i] = source[i] ^ key; // XOR операция
    }
}

void clear_key() {
    pthread_mutex_lock(&g_key_mutex);
    if (g_key_page) {
        // Перед освобождением затираем страницу с ключом
        mprotect(g_key_page, g_page_size, PROT_READ | PROT_WRITE);
        memset(g_key_page, 0, g_page_size);
        munmap(g_key_page, g_page_size);
        g_key_page = nullptr;
        g_page_size = 0;
    }
    pthread_mutex_unlock(&g_key_mutex);
}

#ifdef __cplusplus
}
#endif
