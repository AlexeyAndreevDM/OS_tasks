#include <iostream>
#include <ctime>
#include <cstdlib>
#include <vector>

using namespace std;

void merge(int* arr, int left, int mid, int right, unsigned long long& c, unsigned long long& m) {
    int n1 = mid - left + 1;
    m++;
    int n2 = right - mid;
    m++;
    int* L = new int[n1];
    m++;
    int* R = new int[n2];
    m++;
    for (int i = 0; i < n1; ++i) {
        c++;
        L[i] = arr[left + i];
        m++;
    }
    c++;
    for (int j = 0; j < n2; ++j) {
        c++;
        R[j] = arr[mid + 1 + j];
        m++;
    }
    c++;
    int i = 0, j = 0, k = left;
    m++;
    m++;
    m++;
    while (i < n1 && j < n2) {
        c++;
        if (L[i] <= R[j]) {
            c++;
            arr[k] = L[i];
            m++;
            i++;
        } else {
            c++;
            arr[k] = R[j];
            m++;
            j++;
        }
        k++;
    }
    c++;
    while (i < n1) {
        c++;
        arr[k] = L[i];
        m++;
        i++;
        k++;
    }
    c++;
    while (j < n2) {
        c++;
        arr[k] = R[j];
        m++;
        j++;
        k++;
    }
    c++;
    delete[] L;
    delete[] R;
}

void merge_sort(int* arr, int left, int right, unsigned long long& c, unsigned long long& m) {
    if (left < right) {
        c++;
        int mid = left + (right - left) / 2;
        m++;
        merge_sort(arr, left, mid, c, m);
        merge_sort(arr, mid + 1, right, c, m);
        merge(arr, left, mid, right, c, m);
    }
}

void merge_sort_wrapper(int* arr, int n, unsigned long long& c, unsigned long long& m) {
    c = 0;
    m = 0;
    merge_sort(arr, 0, n - 1, c, m);
}

int main() {
    vector<int> sizes = {100, 200, 500, 1000, 2000, 5000, 10000, 100000, 200000, 500000, 1000000};
    srand(time(nullptr));

    for (int n : sizes) {
        int* arr = new int[n];
        for (int i = 0; i < n; ++i) {
            arr[i] = rand() % 10000;
        }

    // for (int n : sizes) {
    //     int* arr = new int[n];
    //     for (int i = n; i > 0; i--) {
    //         arr[n-i] = i;
    //     }

        // for (int i = 0; i < n; i++) {
        //     cout << arr[i] << " ";
        // }
        // cout << endl;

        unsigned long long c = 0, m = 0;
        clock_t start = clock();
        merge_sort_wrapper(arr, n, c, m);
        clock_t end = clock();
        double time_ms = double(end - start) * 1000.0 / CLOCKS_PER_SEC;

        cout << "n = " << n << "\n"
             << time_ms << "\n"
             << c << "\n"
             << m << "\n"
             << (c + m) << "\n" << endl;
        // for (int i = 0; i < n; i++) {
        //     cout << arr[i] << " ";
        // }
        delete[] arr;
        // break;
    }

    return 0;
}