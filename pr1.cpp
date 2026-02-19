// libcaesar - Побайтовое XOR шифрование

#ifdef __cplusplus
extern "C" {
#endif

static char encryption_key = 0; // Ключ шифрования

void set_key(char key) {
    encryption_key = key;
}

void caesar(void* src, void* dst, int len) {
    unsigned char* source = (unsigned char*)src;
    unsigned char* destination = (unsigned char*)dst;
    
    for (int i = 0; i < len; i++) {
        destination[i] = source[i] ^ encryption_key; // XOR операция
    }
}

#ifdef __cplusplus
}
#endif
