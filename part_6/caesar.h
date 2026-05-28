#ifndef CAESAR_H // защита от двойного включения заголовочного файла (переопределения функций и переменных)
#define CAESAR_H

#include <stddef.h>
#include <stdio.h>

#ifdef __cplusplus
extern "C" { // для линковки с C кодом, чтобы избежать name mangling (смена имен функций компилятором C++)
#endif

void clear_key(); // Безопасная очистка ключа
int is_key_page_address(const void* addr); // Проверка адреса на попадание в страницу ключа
int set_master_key(const unsigned char* key, size_t len); // Установка мастер-ключа для RC4
int rc4_crypt_stream(FILE* input, FILE* output, const unsigned char* salt, size_t salt_len, size_t total_len); // RC4 шифрование/расшифровка потока

#ifdef __cplusplus
}
#endif

#endif
