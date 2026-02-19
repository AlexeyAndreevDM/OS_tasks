#include <iostream>
#include <fstream>
#include <vector>
#include <dlfcn.h>

using namespace std;

typedef void (*set_key_func)(char);
typedef void (*caesar_func)(void*, void*, int);

int main(int argc, char* argv[]) {
    if (argc != 5) {
        cerr << "Использование: " << argv[0] << " <библиотека> <ключ> <вход> <выход>\n";
        return 1;
    }

    const char* lib_path = argv[1];
    char key = argv[2][0];
    const char* input_file = argv[3];
    const char* output_file = argv[4];

    // Загрузка библиотеки
    void* handle = dlopen(lib_path, RTLD_LAZY);
    if (!handle) {
        cerr << "Ошибка: " << dlerror() << "\n";
        return 2;
    }

    dlerror();
    set_key_func set_key = (set_key_func)dlsym(handle, "set_key");
    caesar_func caesar = (caesar_func)dlsym(handle, "caesar");
    
    if (!set_key || !caesar) {
        cerr << "Ошибка загрузки функций\n";
        dlclose(handle);
        return 3;
    }

    // Чтение файла
    ifstream fin(input_file, ios::binary);
    if (!fin) {
        cerr << "Ошибка чтения " << input_file << "\n";
        dlclose(handle);
        return 4;
    }

    fin.seekg(0, ios::end);
    size_t size = fin.tellg();
    fin.seekg(0, ios::beg);
    
    vector<char> buffer(size);
    fin.read(buffer.data(), size);
    fin.close();

    // Шифрование
    set_key(key);
    caesar(buffer.data(), buffer.data(), size);

    // Запись результата
    ofstream fout(output_file, ios::binary);
    if (!fout) {
        cerr << "Ошибка записи " << output_file << "\n";
        dlclose(handle);
        return 5;
    }

    fout.write(buffer.data(), size);
    fout.close();
    dlclose(handle);

    cout << "Обработано: " << size << " байт\n";
    return 0;
}
