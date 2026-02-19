#include <iostream>
#include <vector>
#include <string>
#include <sstream>
#include <cctype>

using namespace std;

// Проверяет, является ли строка целым числом
bool isInteger(const string& s) {
    if (s.empty()) return false;
    int start = 0;
    if (s[0] == '-' || s[0] == '+') {
        if (s.length() == 1) return false; // только знак
        start = 1;
    }
    for (int i = start; i < s.length(); ++i) {
        if (!isdigit(s[i])) return false;
    }
    return true;
}

vector<string> extractIntegerWords(const string& sentence) {
    vector<string> result;
    stringstream ss(sentence);
    string word;
    while (ss >> word) {  // извлекаем слова через пробел
        if (isInteger(word)) {
            result.push_back(word);
        }
    }
    return result;
}

int main() {
    string input;
    cout << "Введите строку слов, разделённых пробелами: ";
    getline(cin, input);  // корректное чтение строки с пробелами

    vector<string> integers = extractIntegerWords(input);

    cout << "Целые числа (в виде строк) в строке:\n";
    for (const string& num : integers) {
        cout << num << endl;
    }

    return 0;
}