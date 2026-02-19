#include <iostream>
#include <vector>
#include <map>
#include <queue>
#include <algorithm>
#include <string>
#include <iomanip>
#include <cmath>
#include <bitset>

using namespace std;

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

// Функция для вывода дерева Хаффмана (обход в глубину)
void printHuffmanTree(HuffmanNode* root, const string& prefix = "", bool isLeft = true) {
    if (!root) return;
    
    cout << prefix;
    cout << (isLeft ? "├──" : "└──" );
    
    // Выводим информацию о узле
    if (root->character != '\0') {
        if (root->character == ' ') {
            cout << "пробел: " << root->frequency << endl;
        } else {
            cout << "'" << root->character << "': " << root->frequency << endl;
        }
    } else {
        cout << root->frequency << endl;
    }
    
    // Рекурсивно выводим левое и правое поддерево
    printHuffmanTree(root->left, prefix + (isLeft ? "│   " : "    "), true);
    printHuffmanTree(root->right, prefix + (isLeft ? "│   " : "    "), false);
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

int main() {
    // Простая настройка для Mac
    setupForMac();
    
    // ФИО студента - используем латиницу для избежания проблем с кодировкой
    string fio = "Gjdfjkjkdjk jfhkdjfk shdksjd jfhkdfn msksnf sm djhfkdjfndfknddfn sfjnfksf df dkfndf dmfndf d fkkdmf";
    
    cout << "Программа сжатия методом Хаффмана\n\n";
    cout << "Исходная строка (ФИО): " << fio << "\n\n";
    
    // Шаг 1: Подсчет частот символов
    map<char, int> frequencies;
    for (char c : fio) {
        frequencies[c]++;
    }
    
    // Шаг 2: Вывод таблицы частот
    cout << "Таблица частот символов:\n";
    cout << "------------------------------------\n";
    cout << "| Символ   | Частота | Вероятность |\n";
    cout << "------------------------------------\n";
    
    int totalChars = fio.length();
    for (const auto& pair : frequencies) {
        double probability = (double)pair.second / totalChars;
        cout << "| " << setw(8) << left << displayChar(pair.first) 
             << " | " << setw(7) << right << pair.second 
             << " | " << setw(11) << fixed << setprecision(6) << probability << " |\n";
    }
    cout << "------------------------------------\n\n";
    
    // Шаг 3: Сортировка по убыванию частот
    vector<pair<char, int>> sortedFreq(frequencies.begin(), frequencies.end());
    sort(sortedFreq.begin(), sortedFreq.end(), 
         [](const pair<char, int>& a, const pair<char, int>& b) {
             return a.second > b.second;
         });
    
    cout << "Отсортированные частоты:\n";
    cout << "------------------------------------\n";
    cout << "| Символ   | Частота | Вероятность |\n";
    cout << "------------------------------------\n";
    for (const auto& pair : sortedFreq) {
        double probability = (double)pair.second / totalChars;
        cout << "| " << setw(8) << left << displayChar(pair.first) 
             << " | " << setw(7) << right << pair.second 
             << " | " << setw(11) << fixed << setprecision(6) << probability << " |\n";
    }
    cout << "------------------------------------\n\n";
    
    // Шаг 4: Построение дерева Хаффмана
    HuffmanNode* root = buildHuffmanTree(frequencies);
    
    // Шаг 5: Вывод дерева Хаффмана
    cout << "Дерево кодирования Хаффмана:\n";
    printHuffmanTree(root);
    cout << endl;
    
    // Шаг 6: Генерация кодов
    map<char, string> huffmanCodes;
    generateHuffmanCodes(root, "", huffmanCodes);
    
    // Вывод таблицы кодов
    cout << "Таблица кодов Хаффмана:\n";
    cout << "-----------------------------\n";
    cout << "| Символ   | Частота | Код   |\n";
    cout << "-----------------------------\n";
    for (const auto& pair : huffmanCodes) {
        cout << "| " << setw(8) << left << displayChar(pair.first) 
             << " | " << setw(7) << right << frequencies[pair.first] 
             << " | " << setw(5) << left << pair.second << " |\n";
    }
    cout << "-----------------------------\n\n";
    
    // Шаг 7: Кодирование строки
    string encoded = encodeHuffman(fio, huffmanCodes);
    cout << "Закодированная строка:\n" << encoded << "\n\n";
    
    // Шаг 8: Расчет параметров сжатия
    int originalBits = fio.length() * 8;
    int compressedBits = encoded.length();
    double compressionRatioAscii = (double)originalBits / compressedBits;
    
    // Расчет для равномерного кода
    int uniformCodeLength = ceil(log2(frequencies.size()));
    int uniformBits = fio.length() * uniformCodeLength;
    double compressionRatioUniform = (double)uniformBits / compressedBits;
    
    // Расчет средней длины и дисперсии
    double avgLength = calculateAverageCodeLength(huffmanCodes, frequencies, totalChars);
    double variance = calculateCodeVariance(huffmanCodes, frequencies, totalChars, avgLength);
    
    cout << "Результаты сжатия:\n";
    cout << "----------------------------------------\n";
    cout << "| Параметр               | Значение   |\n";
    cout << "----------------------------------------\n";
    cout << "| Исходный размер (бит)  | " << setw(10) << originalBits << " |\n";
    cout << "| Сжатый размер (бит)    | " << setw(10) << compressedBits << " |\n";
    cout << "| Коэф. сжатия (ASCII)   | " << setw(10) << fixed << setprecision(3) << compressionRatioAscii << " |\n";
    cout << "| Коэф. сжатия (равном.) | " << setw(10) << fixed << setprecision(3) << compressionRatioUniform << " |\n";
    cout << "| Средняя длина кода     | " << setw(10) << fixed << setprecision(3) << avgLength << " |\n";
    cout << "| Дисперсия длины кода   | " << setw(10) << fixed << setprecision(3) << variance << " |\n";
    cout << "----------------------------------------\n\n";
    
    // Шаг 9: Декодирование и проверка
    string decoded = decodeHuffman(encoded, root);
    cout << "Восстановленная строка: " << decoded << "\n";
    cout << "Совпадение с исходным: " << (fio == decoded ? "Да" : "Нет") << "\n\n";
    
    // Шаг 10: Очистка памяти
    deleteTree(root);
    
    return 0;
}