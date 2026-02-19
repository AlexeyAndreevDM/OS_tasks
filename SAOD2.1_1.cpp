#include <iostream>
#include <ctime>
#include <cstdlib>
#include <vector>

using namespace std;

void shaker_sort(int* arr, int n, unsigned long long& c, unsigned long long& m) {
    int left = 0;
    int right = n - 1;
    bool swapped_forward, swapped_backward;
    c = 0;
    m = 0;
    do {
        c++;
        swapped_forward = false;
        for (int i = left; i < right; ++i) {
            c++;
            if (arr[i] > arr[i + 1]) {
                c++;
                swap(arr[i], arr[i + 1]);
                m++;
                m++;
                m++;
                swapped_forward = true;
            }
        }
        if (!swapped_forward) break;
        c++;
        right--;
        swapped_backward = false;
        for (int i = right; i > left; --i) {
            c++;
            if (arr[i - 1] > arr[i]) {
                c++;
                swap(arr[i - 1], arr[i]);
                m++;
                m++;
                m++;
                swapped_backward = true;
            }
        }
        if (!swapped_backward) break;
        c++;
        left++;
    } while (left <= right);
}

int main() {
    vector<int> sizes = {100, 200, 500, 1000, 2000, 5000, 10000, 100000, 200000, 500000, 1000000};
    // vector<int> sizes = {100000, 200000, 500000, 1000000};
    srand(time(nullptr));

    for (int n : sizes) {
        int* arr = new int[n];
        for (int i = n; i > 0; i--) {
            arr[n-i] = i;
        }

        // for (int i = 0; i < n; i++) {
        //     cout << arr[i] << " ";
        // }
        // cout << endl;

        unsigned long long c = 0, m = 0;
        clock_t start = clock();
        shaker_sort(arr, n, c, m);
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