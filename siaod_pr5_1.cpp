#include <iostream>
#include <bitset>

using namespace std;

unsigned char change_bit(unsigned char x, int bit_num, bool value) {
    unsigned char mask = 1 << (bit_num - 1);
    return value ? (x | mask) : (x & ~mask);
}

void print_binary(unsigned char x) {
    for (int i = 7; i >= 0; i--) {
        cout << ((x >> i) & 1);
    }
}

int main() {
    unsigned char x;
    int bit_num, decimal_input;
    bool value;

    cout << endl;

    x = 255; // 8-разрядное двоичное число 11111111
    cout << "Начальное число: " << (int)x << " (десятичное)" << endl;
    cout << "Начальное число: ";
    print_binary(x);
    cout << " (двоичное)" << endl;
    x = change_bit(x, 5, 0);
    cout << "Новое число: " << (int)x << " (десятичное)" << endl;
    cout << "Новое число: ";
    print_binary(x);
    cout << " (двоичное)" << endl << endl;

    cout << "Введите число в десятичном виде: ";
    cin >> decimal_input;
    x = decimal_input;
    cout << "Начальное число: " << (int)x << " (десятичное)" << endl; // 218
    cout << "Начальное число: ";
    print_binary(x);
    cout << " (двоичное)" << endl;
    x = change_bit(x, 3, 1);
    cout << "Новое число: " << (int)x << " (десятичное)" << endl;
    cout << "Новое число: ";
    print_binary(x);
    cout << " (двоичное)" << endl << endl;

    cout << "Введите число в десятичном виде: ";
    cin >> decimal_input;
    x = decimal_input;
    cout << "Начальное число: " << (int)x << " (десятичное)" << endl; // 128
    cout << "Начальное число: ";
    print_binary(x);
    cout << " (двоичное)" << endl;
    x = change_bit(x, 7, 1);
    cout << "Новое число: " << (int)x << " (десятичное)" << endl;
    cout << "Новое число: ";
    print_binary(x);
    cout << " (двоичное)" << endl;

    cout << endl;
    return 0;
}