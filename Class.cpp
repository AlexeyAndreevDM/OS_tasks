#include <iostream>
#include "Class.h"

using namespace std;

MyArray::MyArray(int size) : size(size) {
    arr = new int[size];
    for (int i = 0; i < size; ++i) {
        arr[i] = i * i;
    }
    cout << "Parameterized constructor" << endl;
}

MyArray::MyArray() : size(0), arr(nullptr) {
    cout << "Default constructor" << endl;
}

MyArray::~MyArray() {
    cout << "Destructor" << endl;
    delete[] arr; // Освобождаем память при деструкции
}

void MyArray::display() {
    for (int i = 0; i < size; ++i) {
        cout << arr[i] << "   ";
    }
    cout << endl;
}

void MyArray::changeSigns() {
    for (int i = 1; i < size; i += 2) {
        arr[i] = -arr[i];
    }
}