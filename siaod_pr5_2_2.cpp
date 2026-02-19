#include <iostream>
#include <fstream>
#include <string>
#include <cstring>
#include <chrono>

using namespace std;
using namespace std::chrono;

// Структура записи банковского счета
struct BankAccount {
    int accountNumber;
    char name[50];
    char address[100];
};

int linearSearch(const string& filename, int key, long long& duration) {
    // Засекаем время начала поиска
    auto start = high_resolution_clock::now();
    
    // Открываем бинарный файл для чтения
    ifstream file(filename, ios::binary);
    if (!file.is_open()) {
        cerr << "Ошибка открытия файла!" << endl;
        duration = 0;
        return -1;
    }
    
    // Определяем размер файла и количество записей
    file.seekg(0, ios::end); // Перемещаемся в конец файла
    int fileSize = file.tellg(); // Получаем размер файла в байтах
    int recordCount = fileSize / sizeof(BankAccount); // Вычисляем количество записей
    file.seekg(0, ios::beg); // Возвращаемся в начало файла
    
    BankAccount acc; // Буфер для чтения одной записи
    int foundIndex = -1;
    
    // Линейный поиск: последовательно проверяем все записи в файле
    for (int i = 0; i < recordCount; ++i) {
        file.read(reinterpret_cast<char*>(&acc), sizeof(BankAccount));
        
        // Проверяем совпадение номера счета с искомым ключом
        if (acc.accountNumber == key) {
            foundIndex = i;
            break;
        }
    }
    
    file.close();

    auto end = high_resolution_clock::now();
    duration = duration_cast<microseconds>(end - start).count();
    
    return foundIndex; // Возвращаем результат поиска или -1 если запись не найдена
}

void testSearchPerformance(const string& filename, int recordCount, int testCases[], int caseCount) {
    cout << "\nТестирование файла: " << filename << " (" << recordCount << " записей)" << endl;
    
    long long totalTime = 0; // Общее время всех поисков
    int foundCount = 0;      // Количество успешно найденных записей
    
    // Выполняем поиск для каждого тестового ключа
    for (int i = 0; i < caseCount; ++i) {
        long long duration; // Время выполнения текущего поиска
        int result = linearSearch(filename, testCases[i], duration);
        
        // Выводим результат поиска
        cout << "Поиск " << (i + 1) << ": Ключ=" << testCases[i] 
             << ", Время=" << duration << " мкс, Результат = ";
        
        if (result != -1) {
            cout << "Найден по индексу " << result;
            foundCount++; // Увеличиваем счетчик найденных записей
        } else {
            cout << "Не найден";
        }
        cout << endl;
        
        totalTime += duration;
    }
    
    cout << "Итоги" << endl;
    cout << "Среднее время поиска: " << (totalTime / caseCount) << " мкс" << endl;
    cout << "Найдено записей: " << foundCount << "/" << caseCount << endl;
}

int main() {
    const string file100 = "binary100.bin";
    const string file1000 = "binary1000.bin"; 
    const string file10000 = "binary10000.bin";
    
    int testCases[] = {
        5080135,
        9999154,
        7647047,
        9210015,
        3290407, 
        8544081
    };
    int caseCount = sizeof(testCases) / sizeof(testCases[0]); // Вычисляем количество тестов
    
    cout << "Практическая оценка времени выполнения линейного поиска" << endl << endl;

    testSearchPerformance(file100, 100, testCases, caseCount);

    testSearchPerformance(file1000, 1000, testCases, caseCount);
    
    testSearchPerformance(file10000, 10000, testCases, caseCount);
    
    cout << "\nТестирование завершено!" << endl;
    
    return 0;
}