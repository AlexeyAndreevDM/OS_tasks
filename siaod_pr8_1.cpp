#include <iostream>
#include <vector>
#include <map>
#include <algorithm>
#include <string>
#include <fstream>
#include <bitset>
#include <cmath>
#include <iomanip>

using namespace std;

struct Symbol {
    char character;
    int frequency;
    double probability;
    string code;
};

// Структура для узла дерева Шеннона-Фано
struct ShannonFanoNode {
    vector<Symbol> symbols;
    string code;
    ShannonFanoNode* left;
    ShannonFanoNode* right;
    
    ShannonFanoNode(const vector<Symbol>& syms) : symbols(syms), left(nullptr), right(nullptr) {}
};

// Функция для подсчета частот символов
map<char, int> calculateFrequencies(const string& text) {
    map<char, int> freq;
    for (char c : text) {
        freq[c]++;
    }
    return freq;
}

// Рекурсивная функция построения дерева Шеннона-Фано
ShannonFanoNode* buildShannonFanoTree(vector<Symbol>& symbols, int start, int end, const string& current_code = "") {
    if (start > end) return nullptr;
    
    ShannonFanoNode* node = new ShannonFanoNode(vector<Symbol>(symbols.begin() + start, symbols.begin() + end + 1));
    node->code = current_code;
    
    // Базовый случай - остался один символ
    if (start == end) {
        symbols[start].code = current_code;
        return node;
    }
    
    // Базовый случай - осталось 2 символа
    if (start == end - 1) {
        symbols[start].code = current_code + "0";
        symbols[end].code = current_code + "1";
        node->left = new ShannonFanoNode({symbols[start]});
        node->right = new ShannonFanoNode({symbols[end]});
        node->left->code = current_code + "0";
        node->right->code = current_code + "1";
        return node;
    }
    
    // Находим оптимальную точку разделения
    double total = 0;
    for (int i = start; i <= end; i++) {
        total += symbols[i].frequency;
    }
    
    double half = total / 2;
    double current_sum = 0;
    int split_index = start;
    
    // Ищем точку разделения с примерно равными суммами частот
    for (int i = start; i <= end; i++) {
        if (current_sum + symbols[i].frequency <= half) {
            current_sum += symbols[i].frequency;
            split_index = i;
        } else {
            break;
        }
    }
    
    // Рекурсивно строим левое и правое поддерево
    node->left = buildShannonFanoTree(symbols, start, split_index, current_code + "0");
    node->right = buildShannonFanoTree(symbols, split_index + 1, end, current_code + "1");
    
    return node;
}

// Функция для вывода дерева Шеннона-Фано
void printShannonFanoTree(ShannonFanoNode* root, const string& prefix = "", bool isLeft = true) {
    if (!root) return;
    
    cout << prefix;
    cout << (isLeft ? "├──" : "└──" );
    
    // Выводим информацию о узле
    if (root->symbols.size() == 1) {
        // Лист - один символ
        Symbol s = root->symbols[0];
        if (s.character == ' ') {
            cout << "пробел: " << s.frequency << " [код: " << root->code << "]" << endl;
        } else {
            cout << "'" << s.character << "': " << s.frequency << " [код: " << root->code << "]" << endl;
        }
    } else {
        // Внутренний узел - группа символов
        int total_freq = 0;
        string symbols_str;
        for (const auto& s : root->symbols) {
            total_freq += s.frequency;
            if (s.character == ' ') {
                symbols_str += "пробел ";
            } else {
                symbols_str += string(1, s.character) + " ";
            }
        }
        cout << "Группа: " << symbols_str << "(" << total_freq << ") [код: " << root->code << "]" << endl;
    }
    
    // Рекурсивно выводим левое и правое поддерево
    printShannonFanoTree(root->left, prefix + (isLeft ? "│   " : "    "), true);
    printShannonFanoTree(root->right, prefix + (isLeft ? "│   " : "    "), false);
}

// Функция для очистки дерева
void deleteTree(ShannonFanoNode* root) {
    if (!root) return;
    deleteTree(root->left);
    deleteTree(root->right);
    delete root;
}

// Функция кодирования текста
string encodeText(const string& text, const map<char, string>& codes) {
    string encoded;
    for (char c : text) {
        encoded += codes.at(c);
    }
    return encoded;
}

// Функция декодирования текста
string decodeText(const string& encoded, const map<string, char>& reverse_codes) {
    string decoded;
    string current_code;
    
    for (char bit : encoded) {
        current_code += bit;
        if (reverse_codes.find(current_code) != reverse_codes.end()) {
            decoded += reverse_codes.at(current_code);
            current_code.clear();
        }
    }
    
    return decoded;
}

// Функция для расчета коэффициента сжатия
double calculateCompressionRatio(int original_bits, int compressed_bits) {
    return (double)original_bits / compressed_bits;
}

// Функция для отображения символа в читаемом формате
string displayChar(char c) {
    if (c == ' ') return "пробел";
    return string(1, c);
}

int main() {
    // Исходный текст из задания
    string text = "One, two, Freddy's coming for you Three, four, better lock your door Five, six, grab a crucifix Seven, eight, gonna stay up late.";
    
    cout << "Программа сжатия методом Шеннона-Фано\n\n";
    cout << "Исходный текст:\n" << text << "\n\n";
    cout << "Длина исходного текста: " << text.length() << " символов\n";
    cout << "Размер в ASCII: " << text.length() * 8 << " бит\n\n";
    
    // Шаг 1: Подсчет частот
    map<char, int> frequency_map = calculateFrequencies(text);
    
    // Шаг 2: Создание и сортировка символов
    vector<Symbol> symbols;
    for (auto& pair : frequency_map) {
        symbols.push_back({pair.first, pair.second, (double)pair.second / text.length(), ""});
    }
    
    // Сортируем по убыванию частоты
    sort(symbols.begin(), symbols.end(), [](const Symbol& a, const Symbol& b) {
        return a.frequency > b.frequency;
    });
    
    // Шаг 3: Построение дерева и кодов
    ShannonFanoNode* root = buildShannonFanoTree(symbols, 0, symbols.size() - 1);
    
    // Шаг 4: Вывод дерева Шеннона-Фано
    cout << "Дерево Шеннона-Фано:\n";
    printShannonFanoTree(root);
    cout << endl;
    
    // Шаг 5: Вывод таблицы кодов
    cout << "Таблица кодов Шеннона-Фано:\n";
    cout << "--------------------------------------------------------\n";
    cout << "| Символ   | Частота | Вероятность | Код        |\n";
    cout << "--------------------------------------------------------\n";
    
    map<char, string> codes;
    map<string, char> reverse_codes;
    
    for (const auto& symbol : symbols) {
        cout << "| " << setw(8) << left << displayChar(symbol.character) 
             << " | " << setw(7) << right << symbol.frequency 
             << " | " << setw(11) << fixed << setprecision(6) << symbol.probability 
             << " | " << setw(10) << left << symbol.code << " |\n";
        
        codes[symbol.character] = symbol.code;
        reverse_codes[symbol.code] = symbol.character;
    }
    cout << "--------------------------------------------------------\n\n";
    
    // Шаг 6: Кодирование текста
    string encoded = encodeText(text, codes);
    cout << "Закодированная последовательность:\n" << encoded << "\n\n";
    cout << "Длина закодированной последовательности: " << encoded.length() << " бит\n";
    
    // Шаг 7: Расчет коэффициента сжатия
    double compression_ratio = calculateCompressionRatio(text.length() * 8, encoded.length());
    cout << "Коэффициент сжатия: " << compression_ratio << "\n\n";
    
    // Шаг 8: Декодирование и проверка
    string decoded = decodeText(encoded, reverse_codes);
    cout << "Восстановленный текст:\n" << decoded << "\n\n";
    cout << "Совпадение с исходным: " << (text == decoded ? "Да" : "Нет") << "\n";
    
    // Очистка памяти
    deleteTree(root);
    
    return 0;
}