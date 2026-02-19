#ifndef CLASS_H
#define CLASS_H

class MyArray {
private:
    int size;         // Количество элементов массива
    int* arr;        // Указатель на массив целого типа

public:
    MyArray(int size); // Параметризованный конструктор
    MyArray();         // Конструктор по умолчанию
    ~MyArray();        // Деструктор

    void display();    // Метод для вывода значений массива
    void changeSigns(); // Метод для изменения знака каждого второго элемента
    void releaseMemory(); // Метод для освобождения памяти
};

#endif // CLASS_H