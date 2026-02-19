#include <iostream>

using namespace std;

// class DynamicArray {
// private:
//     int size;
//     int* arr; // используем указатель, чтобы динамически выделить память на массив int с помощью new
// public:
//     DynamicArray(int s) : size(s) {
//         arr = new int[size];
//         for (int i = 0; i < size; i++) arr[i] = 0;
//         cout << "Конструктор для массива размером " << size << " отработал\n";
//     }

//     // const - переданный по сслыке объект не изменяется, передаем по ссылке, тк иначе конструктор копии будет вызван для передачи в функцию (сам себя до бесконечности)
//     DynamicArray(const DynamicArray& og) : size(og.size){ // инициализируется поле size текущего объекта со значением og.size - длиной ориинального объекта
//         arr = new int[size]; // new - динамическое выделение памяти для целоч массива длиной size
//         for (int i = 0; i < size; i++) arr[i] = og.arr[i];
//         cout << "Конструктор копии скопировал: " << arr << endl;
//     }

//     ~DynamicArray() {delete[] arr;}; // [] - происходит очистка памяти из под массива arr

//     void set(int index, int value) {
//         arr[index] = value;
//     }

//     void print() {
//         for (int i = 0; i < size; ++i) cout << arr[i] << " ";
//         cout << endl;
//     }
// };

// int main() {
//     DynamicArray arr1(5);
//     arr1.set(0, 10);
//     arr1.set(1, 20);
//     arr1.print(); // [ 10 20 0 0 0 ]

//     DynamicArray arr2 = arr1; // Вызов конструктора копии
//     arr2.set(2, 30);
//     arr2.print(); // [ 10 20 30 0 0 ]

//     arr1.print(); // [ 10 20 0 0 0 ] (не изменился)

//     return 0;
// }





// вопрос 1 - Объект
// class Car {
// public:
//     int speed = 0;
//     void accelerate() {speed += 10;};
// };

// int main() {
//     Car ferra;
//     cout << ferra.speed << endl;
//     ferra.accelerate();
//     cout << ferra.speed << endl;
//     return 0;
// }

// вопрос 2 - Система
// class Bank {
// private:
//     static int id;
//     double dep = 0;
// public:
//     string name;
//     void acc(int money) {this->dep += money;};
//     int user(string nm) {this->name = nm; this->id++; return 1;};
//     int transaction(int summ, int id1, int id2) {return 1;};
//     int showdep() {return dep;};
// };

// int Bank::id = 0;

// int main() {
//     Bank bank;
//     bank.acc(100);
//     cout << "Ваш баланс: " << bank.showdep() << endl;
//     if (bank.user("Mr. Fuck")) cout << "Succeeded to create an account, " << bank.name << "!" << endl;
//     return 0;
// }

// вопрос 3 - 