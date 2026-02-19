#include <iostream>
#include <bitset>
#include <vector>

using namespace std;

// Функция для установки бита в байте
unsigned char change_bit(unsigned char x, int bit_num, bool value) {
    unsigned char mask = 1 << (bit_num - 1);
    return value ? (x | mask) : (x & ~mask);
}

// Функция для установки бита в массиве байт
void set_bit(unsigned char arr[], int n) {
    int byte_index = n / 8;
    int bit_num = 8 - (n % 8); // Преобразуем позицию бита (1-8, где 1 - младший, 8 - старший)
    arr[byte_index] = change_bit(arr[byte_index], bit_num, true);
}

int main() {
    vector<int> numbers;
    int input;
    
    cout << "Введите до 64 чисел от 0 до 63 (для завершения введите любое число вне этого диапазона):" << endl;
    
    while (numbers.size() < 64) {
        cin >> input;
        if (input < 0 || input > 63) {
            break;
        }
        numbers.push_back(input);
    }

    cout << endl << "Введенные числа: ";
    for (int num : numbers) {
        cout << num << " ";
    }
    cout << endl;

    unsigned char bit_array[8] = {0};
    
    for (int num : numbers) {
        set_bit(bit_array, num);
    }

    cout << "Битовый массив: ";
    for (int i = 0; i < 8; i++) {
        cout << bitset<8>(bit_array[i]) << " ";
    }
    cout << endl;

    cout << "Отсортированные числа: ";
    for (int byte_index = 0; byte_index < 8; byte_index++) {
        for (int bit_pos = 7; bit_pos >= 0; bit_pos--) {
            // Проверяем установлен ли бит с помощью маски
            unsigned char mask = 1 << bit_pos;
            if (bit_array[byte_index] & mask) {
                cout << (byte_index * 8 + (7 - bit_pos)) << " ";
            }
        }
    }
    cout << endl;

    return 0;
}