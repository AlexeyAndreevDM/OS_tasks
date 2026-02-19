#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <algorithm>
#include <chrono>

using namespace std;
using namespace std::chrono;

struct BankAccount {
    int accountNumber;
    char name[50];
    char address[100];
};

// Структура для хранения ключа и его оригинальной позиции
struct KeyPosition {
    int key;              // Номер счета
    int originalIndex;    // Оригинальный индекс в файле
    
    bool operator<(const KeyPosition& other) const {
        return key < other.key;
    }
};

int classicBinarySearch(const vector<KeyPosition>& sortedData, int targetKey) {
    int left = 0;
    int right = sortedData.size() - 1;
    
    // Классический бинарный поиск: на каждом шаге делим диапазон пополам
    while (left <= right) {
        int mid = left + (right - left) / 2;
        
        // Проверяем, совпадает ли ключ в середине с искомым
        if (sortedData[mid].key == targetKey) {
            return sortedData[mid].originalIndex;
        } 
        // Если ключ в середине меньше искомого - ищем в правой половине
        else if (sortedData[mid].key < targetKey) {
            left = mid + 1;  // Выбор правой части массива
        } 
        // Если ключ в середине больше искомого - ищем в левой половине  
        else {
            right = mid - 1; // Выбор левой части массива
        }
    }
    
    return -1; // Элемент не найден
}

BankAccount readRecordByIndex(const string& filename, int index) {
    ifstream file(filename, ios::binary);
    if (!file.is_open()) {
        throw runtime_error("Ошибка открытия файла!");
    }
    
    // Перемещаемся к нужной записи (прямой доступ по индексу)
    file.seekg(index * sizeof(BankAccount));
    
    BankAccount acc;
    file.read(reinterpret_cast<char*>(&acc), sizeof(BankAccount));
    
    file.close();
    return acc;
}

int binarySearchFile(const string& filename, int key, long long& duration) {
    auto start = high_resolution_clock::now();
    
    ifstream file(filename, ios::binary);
    if (!file.is_open()) {
        cerr << "Ошибка открытия файла!" << endl;
        duration = 0;
        return -1;
    }
    
    // Определяем количество записей в файле
    file.seekg(0, ios::end);
    long long fileSize = file.tellg();
    int recordCount = fileSize / sizeof(BankAccount);
    file.seekg(0, ios::beg);
    
    // Создаем массив пар (ключ, оригинальный индекс) для сортировки
    vector<KeyPosition> keyPositions;
    keyPositions.reserve(recordCount);
    
    // Читаем все записи и сохраняем ключи с их оригинальными индексами
    for (int i = 0; i < recordCount; ++i) {
        BankAccount acc;
        file.read(reinterpret_cast<char*>(&acc), sizeof(BankAccount));
        
        KeyPosition kp;
        kp.key = acc.accountNumber;
        kp.originalIndex = i;
        keyPositions.push_back(kp);
    }
    
    file.close();
    
    // СОРТИРОВКА массива по ключам - подготовка для бинарного поиска
    sort(keyPositions.begin(), keyPositions.end());
    
    // Применяем классический бинарный поиск к отсортированному массиву
    int result = classicBinarySearch(keyPositions, key);
    
    auto end = high_resolution_clock::now();
    duration = duration_cast<microseconds>(end - start).count();
    
    return result;
}

void testSearchPerformance(const string& filename, int recordCount, int testCases[], int caseCount) {
    cout << "\nТестирование файла: " << filename << " (" << recordCount << " записей)" << endl;
    
    long long totalTime = 0;
    int foundCount = 0;
    
    for (int i = 0; i < caseCount; ++i) {
        long long duration;
        int result = binarySearchFile(filename, testCases[i], duration);
        
        cout << "Поиск " << (i + 1) << ": Ключ=" << testCases[i] 
             << ", Время=" << duration << " мкс, Результат = ";
        
        if (result != -1) {
            // Если запись найдена, читаем и выводим всю информацию
            BankAccount foundAccount = readRecordByIndex(filename, result);
            cout << "Найден!" << endl;
            cout << "  Номер счета: " << foundAccount.accountNumber << endl;
            cout << "  Имя: " << foundAccount.name << endl;
            cout << "  Адрес: " << foundAccount.address << endl;
            cout << "  (Позиция в файле: " << result << ")" << endl;
            foundCount++;
        } else {
            cout << "Не найден" << endl;
        }
        cout << endl;
        
        totalTime += duration;
    }
    
    cout << "Итоги:" << endl;
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
    int caseCount = sizeof(testCases) / sizeof(testCases[0]);
    
    cout << "Практическая оценка времени выполнения классического бинарного поиска:" << endl;

    testSearchPerformance(file100, 100, testCases, caseCount);
    testSearchPerformance(file1000, 1000, testCases, caseCount);
    testSearchPerformance(file10000, 10000, testCases, caseCount);
    
    return 0;
}