#include <iostream>
#include <fstream>
#include <vector>
#include <chrono>
#include <bitset>

using namespace std;
using namespace std::chrono;

unsigned char change_bit(unsigned char x, int bit_num, bool value) {
    unsigned char mask = 1 << (bit_num - 1);
    return value ? (x | mask) : (x & ~mask);
}

int main() {
    auto start_time = high_resolution_clock::now();
    
    // Параметры
    const int MIN_NUMBER = 1000000;
    const int MAX_NUMBER = 9999999;
    const int NUM_RANGE = MAX_NUMBER - MIN_NUMBER + 1;
    const size_t BIT_ARRAY_SIZE = (NUM_RANGE + 7) / 8; // Размер в байтах
    
    cout << "Сортировка чисел с использованием битового массива..." << endl;
    cout << "Размер битового массива: " << BIT_ARRAY_SIZE << " байт (" 
         << BIT_ARRAY_SIZE / 1024 << " КБ)" << endl;
    
    // Создаем битовый массив
    vector<unsigned char> bit_array(BIT_ARRAY_SIZE, 0);
    
    // Чтение чисел из файла и установка битов
    ifstream infile("numbers.txt");
    if (!infile) {
        cerr << "Ошибка при открытии файла!" << endl;
        return 1;
    }
    
    int number;
    while (infile >> number) {
        int index = number - MIN_NUMBER;
        int byte_index = index / 8;
        int bit_index = index % 8;
        bit_array[byte_index] = change_bit(bit_array[byte_index], 8 - bit_index, true);
    }
    infile.close();
    
    // Запись отсортированных чисел в файл
    ofstream outfile("sorted_numbers.txt");
    if (!outfile) {
        cerr << "Ошибка при создании файла!" << endl;
        return 1;
    }
    
    for (int i = 0; i < NUM_RANGE; ++i) {
        int byte_index = i / 8;
        int bit_index = i % 8;
        
        // Проверяем установлен ли бит с помощью маски
        unsigned char mask = 1 << (7 - bit_index);
        if (bit_array[byte_index] & mask) {
            outfile << (i + MIN_NUMBER) << '\n';
        }
    }
    outfile.close();
    
    auto end_time = high_resolution_clock::now();
    auto duration = duration_cast<milliseconds>(end_time - start_time);
    
    cout << "Сортировка завершена. Результат в файле 'sorted_numbers.txt'" << endl;
    cout << "Время сортировки: " << duration.count() << " мс" << endl;
    
    return 0;
}