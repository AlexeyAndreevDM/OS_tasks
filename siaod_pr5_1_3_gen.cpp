#include <iostream>
#include <fstream>
#include <random>
#include <vector>
#include <algorithm>
#include <chrono>

using namespace std;
using namespace std::chrono;

int main() {
    // Параметры генерации
    const int MIN_NUMBER = 1000000;
    const int MAX_NUMBER = 9999999;
    const int NUM_COUNT = 9000000; // Всего 9 миллионов семизначных чисел
    
    cout << "Генерация файла неповторяющихся семизначных чисел..." << endl;
    auto start_time = high_resolution_clock::now();
    
    // Создаем вектор всех возможных семизначных чисел
    vector<int> all_numbers;
    all_numbers.reserve(NUM_COUNT);
    
    for (int i = MIN_NUMBER; i <= MAX_NUMBER; ++i) {
        all_numbers.push_back(i);
    }
    
    // Перемешиваем числа
    random_device rd;
    mt19937 g(rd());
    shuffle(all_numbers.begin(), all_numbers.end(), g);
    
    // Записываем в файл
    ofstream outfile("numbers.txt");
    if (!outfile) {
        cerr << "Ошибка при создании файла!" << endl;
        return 1;
    }
    
    for (int i = 0; i < NUM_COUNT; ++i) {
        outfile << all_numbers[i] << '\n';
    }
    outfile.close();
    
    auto end_time = high_resolution_clock::now();
    auto duration = duration_cast<milliseconds>(end_time - start_time);
    
    cout << "Файл 'numbers.txt' успешно создан." << endl;
    cout << "Сгенерировано чисел: " << NUM_COUNT << endl;
    cout << "Время генерации: " << duration.count() << " мс" << endl;
    
    return 0;
}