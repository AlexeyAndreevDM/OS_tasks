#include <iostream>
#include <vector>
#include <cstdlib> // Для rand() и srand()
#include <ctime>   // Для time()

using namespace std;

// Функция для вывода массива
void printArray(int arr[], int n) {
    for (int i = 0; i < n; i++) {
        cout << arr[i] << " ";
    }
    cout << endl;
}

// Функция сортировки методом вставки
void insertionSort(int arr[], int n, int& comparisons, int& movements, double& t) {
    time_t start1 = clock();
    for (int i = 1; i < n; i++) {
        comparisons++;
        int key = arr[i];
        movements++;
        int j = i - 1;
        movements++;
        while (j >= 0 && arr[j] > key) {
            comparisons++;
            arr[j + 1] = arr[j];
            movements++;
            j--;
        }
        arr[j + 1] = key;
        movements++;
    }
    time_t end1 = clock();
    t = end1 - start1;
}

int main() {
    int n;
    cout << "Введите кол-во значений массива: ";
    cin >> n;

    int arr[n];

    // Инициализация генератора случайных чисел
    srand(static_cast<unsigned int>(time(0)));

    // Заполнение массива случайными значениями
    // for (int i = 0; i < n; i++) {
    //     arr[i] = rand() % 100; // Случайные числа от 0 до 99
    // }
    // Заполнение массива значениями от n до 1
    // for (int i = 0; i < n; i++) {
    //     arr[i] = n - i; // Заполняем от n до 1
    // }
    // Заполнение массива значениями от 1 до n
    for (int i = 0; i < n; i++) {
        arr[i] = i; // Заполняем от 1 до n
    }

    // Сортировка массива
    int comparisons1 = 0, movements1 = 0;
    double t1 = 0;
    insertionSort(arr, n, comparisons1, movements1, t1);

    // Вывод отсортированного массива
    // cout << "Отсортированный массив: ";
    // printArray(arr, n);
    
    cout << endl;
    cout << "n = " << n << endl;
    cout << comparisons1 << " " << movements1 << " " << comparisons1 + movements1 << endl;
    cout << "Время: " << (double)((t1) * 1000 / CLOCKS_PER_SEC) << " секунд." << endl;
    cout << endl;

    return 0;
}
