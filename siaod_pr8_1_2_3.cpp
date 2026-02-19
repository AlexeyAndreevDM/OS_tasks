#include <iostream>
#include <vector>
#include <map>
#include <queue>
#include <algorithm>
#include <string>
#include <iomanip>
#include <cmath>
#include <bitset>
#include <fstream>
#include <chrono>

using namespace std;
using namespace std::chrono;

// Простая настройка для Mac
void setupForMac() {
    std::ios_base::sync_with_stdio(true);
}

// Структура для узла дерева Хаффмана
struct HuffmanNode {
    char character;
    int frequency;
    HuffmanNode* left;
    HuffmanNode* right;
    
    HuffmanNode(char ch, int freq) : character(ch), frequency(freq), left(nullptr), right(nullptr) {}
    HuffmanNode(int freq) : character('\0'), frequency(freq), left(nullptr), right(nullptr) {}
};

// Компаратор для priority_queue (минимальная куча)
struct CompareNodes {
    bool operator()(HuffmanNode* a, HuffmanNode* b) {
        return a->frequency > b->frequency;
    }
};

// Функция для построения дерева Хаффмана
HuffmanNode* buildHuffmanTree(const map<char, int>& frequencies) {
    priority_queue<HuffmanNode*, vector<HuffmanNode*>, CompareNodes> pq;
    
    // Создаем листья для каждого символа
    for (const auto& pair : frequencies) {
        pq.push(new HuffmanNode(pair.first, pair.second));
    }
    
    // Объединяем узлы пока не останется один (корень)
    while (pq.size() > 1) {
        HuffmanNode* left = pq.top(); pq.pop();
        HuffmanNode* right = pq.top(); pq.pop();
        
        HuffmanNode* parent = new HuffmanNode(left->frequency + right->frequency);
        parent->left = left;
        parent->right = right;
        
        pq.push(parent);
    }
    
    return pq.top();
}

// Функция для генерации кодов Хаффмана (обход дерева)
void generateHuffmanCodes(HuffmanNode* root, const string& code, map<char, string>& codes) {
    if (!root) return;
    
    // Если достигли листа - сохраняем код
    if (!root->left && !root->right) {
        codes[root->character] = code;
        return;
    }
    
    // Рекурсивно обходим левое и правое поддерево
    generateHuffmanCodes(root->left, code + "0", codes);
    generateHuffmanCodes(root->right, code + "1", codes);
}

// Функция для кодирования текста
string encodeHuffman(const string& text, const map<char, string>& codes) {
    string encoded;
    for (char c : text) {
        encoded += codes.at(c);
    }
    return encoded;
}

// Функция для декодирования текста
string decodeHuffman(const string& encoded, HuffmanNode* root) {
    string decoded;
    HuffmanNode* current = root;
    
    for (char bit : encoded) {
        // Двигаемся по дереву в зависимости от бита
        current = (bit == '0') ? current->left : current->right;
        
        // Если достигли листа - добавляем символ и возвращаемся к корню
        if (!current->left && !current->right) {
            decoded += current->character;
            current = root;
        }
    }
    
    return decoded;
}

// Функция для расчета средней длины кода
double calculateAverageCodeLength(const map<char, string>& codes, const map<char, int>& frequencies, int totalChars) {
    double sum = 0;
    for (const auto& pair : codes) {
        char ch = pair.first;
        string code = pair.second;
        double probability = (double)frequencies.at(ch) / totalChars;
        sum += code.length() * probability;
    }
    return sum;
}

// Функция для расчета дисперсии длины кода
double calculateCodeVariance(const map<char, string>& codes, const map<char, int>& frequencies, int totalChars, double avgLength) {
    double variance = 0;
    for (const auto& pair : codes) {
        char ch = pair.first;
        string code = pair.second;
        double probability = (double)frequencies.at(ch) / totalChars;
        variance += probability * pow(code.length() - avgLength, 2);
    }
    return variance;
}

// Функция для очистки дерева (память)
void deleteTree(HuffmanNode* root) {
    if (!root) return;
    deleteTree(root->left);
    deleteTree(root->right);
    delete root;
}

// Функция для отображения символа в читаемом формате
string displayChar(char c) {
    if (c == ' ') return "пробел";
    return string(1, c);
}

// Функция для создания тестового файла
void createTestFile() {
    ofstream file("test_file.txt");
    if (!file.is_open()) {
        cout << "Ошибка создания файла!\n";
        return;
    }
    
    // Текст из задания 1, вариант 2
    file << "One, two, Freddy's coming for you Three, four, better lock your door Five, six, grab a crucifix Seven, eight, gonna stay up late.";
    file.close();
    
    cout << "Создан тестовый файл: test_file.txt\n";
}

// Функция для чтения файла
string readFile(const string& filename) {
    ifstream file(filename);
    if (!file.is_open()) {
        throw runtime_error("Не удалось открыть файл: " + filename);
    }
    string content((istreambuf_iterator<char>(file)), istreambuf_iterator<char>());
    file.close();
    return content;
}

int main() {
    // Настройка для Mac
    setupForMac();
    
    cout << "Программа сжатия методом Хаффмана\n";
    
    // Часть 1: Создаем тестовый файл автоматически
    cout << "1. Создание тестового файла\n";
    cout << "---------------------------\n";
    createTestFile();
    
    // Часть 2: Архивация файла
    cout << "\n2. Архивация файла\n";
    cout << "------------------\n";
    
    try {
        string filename = "test_file.txt";  // Файл в той же папке
        string content = readFile(filename);
        
        cout << "Содержимое файла:\n" << content << "\n\n";
        cout << "Размер файла: " << content.length() << " символов\n";
        cout << "Размер в ASCII: " << content.length() * 8 << " бит\n\n";
        
        auto startTime = high_resolution_clock::now();
        
        // Подсчет частот
        map<char, int> frequencies;
        for (char c : content) {
            frequencies[c]++;
        }
        
        // Построение дерева Хаффмана
        HuffmanNode* root = buildHuffmanTree(frequencies);
        
        // Генерация кодов
        map<char, string> huffmanCodes;
        generateHuffmanCodes(root, "", huffmanCodes);
        
        // Кодирование
        string encoded = encodeHuffman(content, huffmanCodes);
        
        auto endTime = high_resolution_clock::now();
        auto duration = duration_cast<microseconds>(endTime - startTime);
        
        cout << "Результаты архивации:\n";
        cout << "----------------------------------------\n";
        cout << "| Параметр               | Значение   |\n";
        cout << "----------------------------------------\n";
        cout << "| Исходный размер (бит)  | " << setw(10) << content.length() * 8 << " |\n";
        cout << "| Сжатый размер (бит)    | " << setw(10) << encoded.length() << " |\n";
        cout << "| Коэффициент сжатия     | " << setw(10) << fixed << setprecision(2) 
             << (double)(content.length() * 8) / encoded.length() << " |\n";
        cout << "| Время выполнения (мкс) | " << setw(10) << duration.count() << " |\n";
        cout << "----------------------------------------\n\n";
        
        // Проверка корректности
        string decoded = decodeHuffman(encoded, root);
        cout << "Проверка корректности:\n";
        cout << "Восстановленный текст совпадает с исходным: " << (content == decoded ? "Да" : "Нет") << "\n";
        
        // Очистка памяти
        deleteTree(root);
        
    } catch (const exception& e) {
        cout << "Ошибка: " << e.what() << endl;
    }
    
    return 0;
}