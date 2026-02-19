#include <iostream>
#include <cstdlib>
#include <ctime>
#include <cstring>
#include <ctime>

using namespace std;

void delFirstMethod(char* x, int& n, char key, int& comparisons, int& movements, double& t) {
    int i = 0;
    time_t start1 = clock();
    while (i < n) {
        comparisons++;
        if (x[i] == key) {
            comparisons++;
            for (int j = i; j < n - 1; j++) {
                comparisons++;
                x[j] = x[j + 1];
                movements++;
            }
            n--;
            movements++;
        } else {
            i++;
            movements++;
        }
    }
    time_t end1 = clock();
    t = end1 - start1;
}

void delOtherMethod(char* x, int& n, char key, int& comparisons, int& movements,  double& t) {
    int j = 0;
    time_t start1 = clock();
    for (int i = 0; i < n; i++) {
        comparisons++;
        x[j] = x[i];
        movements++;
        if (x[i] != key) {
            comparisons++;
            j++;
            movements++;
        }
    }
    if (x[0] == key) {
        n = 0;
    }
    else {
        n = j;
    }
    time_t end1 = clock();
    t = end1 - start1;
}

int main() {
    srand(static_cast<unsigned>(time(0))); // Инициализация генератора случайных чисел

    int n, n1;
    char key;
    cout << "Введите количество элементов массива (n): ";
    cin >> n;
    n1 = n;

    // Генерация случайной строки из символов
    char* arr = new char[n];
    char* arr1 = new char[n];
    cout << "Сгенерированная строка: ";
    for (int i = 0; i < n; i++) {
        arr[i] = 'a' + rand() % 26; // Генерация символов от 'a' до 'z'
        cout << arr[i];
        arr1[i] = arr[i];
    }
    cout << endl;

    cout << "Введите ключевой символ для удаления: ";
    // cin >> key;
    key = '_';

    // Тестирование первого метода
    int comparisons1 = 0, movements1 = 0;
    double t1 = 0;
    delFirstMethod(arr, n, key, comparisons1, movements1, t1);
    cout << endl;
    cout << "Для n = " << n1 << endl;
    cout << "После удаления с помощью первого метода: ";
    // for (int i = 0; i < n; i++) {
    //     cout << arr[i];
    // }
    cout << endl;
    cout << "Сравнения: " << comparisons1 << ", Перемещения: " << movements1 << ", Всего операций: " << comparisons1 + movements1 << endl;
    cout << "Время выполнения первого метода: " << (double) ((t1) * 1000 / CLOCKS_PER_SEC) << " секунд." << endl;
    cout << endl;
    // Сброс массива для второго метода
    delete[] arr;
    
    // // Генерация новой случайной строки
    // cout << "Сгенерированная строка для второго метода: ";
    // for (int i = 0; i < n; i++) {
    //     arr[i] = 'a' + rand() % 26;
    //     cout << arr[i];
    // }
    // cout << endl;

    // Тестирование второго метода
    int comparisons2 = 0, movements2 = 0;
    double t2 = 0;
    delOtherMethod(arr1, n1, key, comparisons2, movements2, t2);
    
    cout << "После удаления с помощью второго метода: ";
    // for (int i = 0; i < n1; i++) {
    //     cout << arr1[i];
    // }
    cout << endl;

    cout << "Сравнения: " << comparisons2 << ", Перемещения: " << movements2 << ", Всего операций: " << comparisons2 + movements2 << endl;
    cout << "Время выполнения второго метода: " << (double) ((t2) * 1000 / CLOCKS_PER_SEC) << " секунд." << endl;

    delete[] arr1; // Освобождение памяти
    return 0;
}