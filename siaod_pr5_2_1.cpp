#include <iostream>
#include <fstream>
#include <string>
#include <cstring>
#include <random>
#include <set>

using namespace std;

struct BankAccount {
    int accountNumber; // 4 байта
    char name[50]; // 50 байт
    char address[100]; // 100 байт
};

// Предусловие: n > 0
// Постусловие: Создан текстовый файл с n уникальными случайными записями
void createTextFile(const string& filename, int n) {
    ofstream file(filename);
    if (!file.is_open()) {
        cerr << "Error creating text file!" << endl;
        return;
    }
    
    // Инициализация генератора случайных чисел
    random_device rd;
    mt19937 gen(rd());
    uniform_int_distribution<int> dist(1000000, 9999999);
    
    set<int> usedNumbers;
    
    for (int i = 0; i < n; ++i) {
        int accountNum;
        // Генерируем уникальный номер счета, повторяем пока не получим уникальный номер
        do {
            accountNum = dist(gen);
        } while (usedNumbers.count(accountNum) > 0);
        usedNumbers.insert(accountNum); // Добавляем номер в использованные
        
        file << accountNum << "\n";
        file << "Client_" << i << "_Name\n";
        file << "Address_" << i << "_Street_" << accountNum % 1000 << "\n";
    }
    file.close();
    cout << "Text file '" << filename << "' created with " << n << " records." << endl;
}

// Предусловие: textFile существует и имеет корректный формат
// Постусловие: Создан двоичный файл с данными из textFile
void textToBinary(const string& textFile, const string& binFile) {
    ifstream in(textFile); // Открываем текстовый файл для чтения
    ofstream out(binFile, ios::binary); // // Открываем двоичный файл для записи в ios::binary - бинарном режиме
    
    if (!in.is_open() || !out.is_open()) {
        cerr << "Error opening files for conversion!" << endl;
        return;
    }
    
    BankAccount acc;
    string temp;
    int recordCount = 0;
    
    while (getline(in, temp)) {
        acc.accountNumber = stoi(temp); // Первая строка - номер счета (преобразуем строку в int)
        
        getline(in, temp);
        strncpy(acc.name, temp.c_str(), 49); // Копируем строку в поле name с ограничением длины
        acc.name[49] = '\0';
        
        getline(in, temp);
        strncpy(acc.address, temp.c_str(), 99); // Копируем строку в поле address с ограничением длины
        acc.address[99] = '\0';
        
        // Записываем структуру и размер одной записи в байтах в двоичный файл с преобразование указателя на структуру в указатель на char
        out.write(reinterpret_cast<char*>(&acc), sizeof(BankAccount));
        recordCount++;
    }
    
    in.close();
    out.close();
    cout << "Binary file '" << binFile << "' created with " << recordCount << " records." << endl;
    cout << "Record size: " << sizeof(BankAccount) << " bytes" << endl;
}

int main() {
    const string textFile = "text10000.txt";
    const string binFile = "binary10000.bin";
    const int RECORD_COUNT = 10000; // количество генерируемых записей
    
    // Создание текстового файла
    createTextFile(textFile, RECORD_COUNT);
    
    // Преобразование в двоичный формат
    textToBinary(textFile, binFile);
    
    cout << "\nFiles generated successfully!" << endl;
    cout << "Text file: " << textFile << endl;
    cout << "Binary file: " << binFile << endl;
    
    return 0;
}