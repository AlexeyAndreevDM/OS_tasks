#include <iostream>
#include <bitset>
#include <vector>
#include <algorithm>

using namespace std;

unsigned long long change_bit(unsigned long long x, int bit_num, bool value) {
    unsigned long long mask = 1ULL << (bit_num - 1);
    return value ? (x | mask) : (x & ~mask);
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

    unsigned long long bit_array = 0;
    
    for (int num : numbers) {
        bit_array = change_bit(bit_array, 64 - num, true);
    }

    cout << "Битовый массив: " << bitset<64>(bit_array) << endl;

    cout << "Отсортированные числа: ";
    for (int i = 64; i >= 1; i--) {
        unsigned long long mask = 1ULL << (i - 1);
        if (bit_array & mask) {
            cout << (64 - i) << " ";
        }
    }
    cout << endl;

    return 0;
}