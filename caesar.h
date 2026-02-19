/* caesar.h */

#ifndef CAESAR_H
#define CAESAR_H

#ifdef __cplusplus
extern "C" {
#endif

void set_key(char key); // Установка ключа
void caesar(void* src, void* dst, int len); // XOR шифровка/дешифровка

#ifdef __cplusplus
}
#endif

#endif /* CAESAR_H */
