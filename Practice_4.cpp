#include <iostream>
#include <math.h>
#include <cmath>
#include <limits>
#include <algorithm>
#include <iomanip>
#include <fstream>
#include <string>
#include <cstring>
#include <cctype>
#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <unistd.h>

using namespace std;
// setlocale(LC_ALL, "rus");

const double PI = 3.14;

double Rect(double width, double height) {
    return width * height;
}

double Tri(double base, double height) {
    return 0.5 * base * height;
}

double Circle(double radius) {
    return PI * pow(radius, 2);
}

double sign(double x) {
    if (x > 0) {
        return 1;
    }
    if (x == 0) {
        return 0;
    }
     if (x < 0) {
        return -1;
    }
}


int main()
{
    int dznum;
    const double PI = 3.14;
    cout << "Введите номер задания (1-5): ";
    cin >> dznum;
    // sleep(5);
    while (dznum != -1)
    {
        if (dznum == 1)
        {
            int numbers[10];
    
            cout << "Введите " << 10 << " чисел: ";
            for (int i = 0; i < 10; ++i) {
                cin >> numbers[i];
            }
            ofstream outFile("tekst.txt");
            for (int i = 0; i < 10; ++i) {
                outFile << numbers[i] << (i < 10 - 1 ? " " : "");
            }
            outFile.close();

            ifstream inFile("tekst.txt");
            int sum = 0, number;
            while (inFile >> number) {
                sum += number;
            }
            inFile.close();
            cout << "Сумма чисел из файла: " << sum;
        }
        if (dznum == 2)
        {
            int numb;
            cout << "Введите число для определения знака: ";
            cin >> numb;
            cout << sign(numb);
            // if (isdigit(numb)) {
            //     cout << sign(numb);
            // }
            // else {
            //     cout << "Вы ввели не число";
            //     cin.clear();
            //     sleep(1);
            // }
        }
        if (dznum == 3)
        {
            int choice;
            cout << "Выберите фигуру для расчета площади:\n1 - Прямоугольник\n2 - Треугольник\n3 - Круг\nВведите номер фигуры (1-3): ";
            cin >> choice;

    switch (choice) {
        case 1: {
            double width, height;
            cout << "Введите ширину прямоугольника: ";
            cin >> width;
            cout << "Введите высоту прямоугольника: ";
            cin >> height;
            cout << "Площадь прямоугольника: " << Rect(width, height);
            break;
        }
        case 2: {
            double base, height;
            cout << "Введите основание треугольника: ";
            cin >> base;
            cout << "Введите высоту треугольника: ";
            cin >> height;
            cout << "Площадь треугольника: " << Tri(base, height);
            break;
        }
        case 3: {
            double radius;
            cout << "Введите радиус круга: ";
            cin >> radius;
            cout << "Площадь круга: " << Circle(radius);
            break;
        }
        default:
            cout << "Пожалуйста, выберите число от 1 до 3.\n";
            break;
    }
        }
        if (dznum == 4)
        {
            const int stripes = 25, stars = 12;

    for (int i = 0; i < 8; ++i)
    {
        if (i < 4) {
            for (int j = 0; j < stars; ++j)
            {
                cout << "*";
            }
            for (int j = 0; j < stripes; ++j)
            {
                cout << "_";
            }
            cout << endl;
        } 
        else {
            for (int j = 0; j < (stripes + stars); ++j)
            {
                cout << "_";
            }
            cout << endl;
        }
    }
        }
        if (dznum == 5)
        {
        //    n5();
        }
        cout << "\nВведите номер задания (1-5): ";
        cin >> dznum;
    }
    return 0;
}

