#include <iostream>
#include <bitset>
#include <vector>
#include <algorithm>

using namespace std;

unsigned char change_bit(unsigned char x, int bit_num, bool value) {
    unsigned char mask = 1 << (bit_num - 1);
    return value ? (x | mask) : (x & ~mask);
}

int main() {
    // Ввод чисел
    vector<int> numbers;
    int input;
    
    cout << "Введите до 8 чисел от 0 до 7 (для завершения введите любое число вне этого диапазона):" << endl;
    
    while (numbers.size() < 8) {
        cin >> input;
        if (input < 0 || input > 7) {
            break;
        }
        numbers.push_back(input);
    }

    cout << endl << "Введенные числа: ";
    for (int num : numbers) {
        cout << num << " ";
    }
    cout << endl;

    unsigned char bit_array = 0; // Создание битового массива
    
    for (int num : numbers) {
        bit_array = change_bit(bit_array, 8 - num, true);
    }

    cout << "Битовый массив: " << bitset<8>(bit_array) << endl;

    cout << "Отсортированные числа: ";
    for (int i = 8; i >= 1; i--) {
        // Проверяем установлен ли i-й бит
        unsigned char mask = 1 << (i - 1);
        if (bit_array & mask) {
            cout << (8 - i) << " ";
        }
    }
    cout << endl;

    return 0;
}