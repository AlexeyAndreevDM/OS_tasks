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
#include <sstream>
#include <vector>

using namespace std;

int div_up(int x, int y)
{
    return x / y + (x%y ? 1 : 0);
}

int index_coupe(int num)
{
    int index;
    if (num <= 36){
        index = div_up(num, 4)-1;
    }
    else{
        index = - div_up(num - 38, 2) + 8;
    }
    return index;
}

pair<long long, long long> lsp(long long N, long long K) {
    vector<pair<long long, long long> >free_seats;
    free_seats.push_back(make_pair(0, N));

    long long left_free = 0;
    long long right_free = 0;

    for (long long i = 0; i<K; ++i) {
        pair<long long, long long> longest = free_seats[0];
        for (size_t j = 1; j <free_seats.size(); ++j) {
            if ((free_seats[j].second - free_seats[j].first) > (longest.second - longest.first)) {
                longest = free_seats[j];
            }
        }

        free_seats.erase(remove(free_seats.begin(), free_seats.end(), longest), free_seats.end());
        long long start = longest.first;
        long long end = longest.second;
        long long length = end - start;
        long long mid;
        if (length % 2 == 1) {
            mid = (start + end) / 2;
        } else {
            mid = (start + end - 1) / 2;
        }
        left_free = mid - start;
        right_free = end - (mid + 1);
        if (left_free > 0) free_seats.emplace_back(start, mid);
        if (right_free > 0) free_seats.emplace_back(mid + 1, end);
    }

    return make_pair(left_free, right_free);
}

int main()
{
    int dznum;
    cout << "Введите номер задания (1-5): ";
    cin >> dznum;
    while (dznum != -1)
    {
        if (dznum == 1)
        {
            int a, b, c, n=1;
            cout<< "Введите a, b, c: ";
            cin >> a >> b >> c;
            while (true) {
                if (a+b*(n+1)<=c)
                    n++;
                else
                    break;
            }
            cout << "Максимальное число лопастей спинера: " << n;
        }
        if (dznum == 2)
        {
            int M, x, y, remaining;
            cin >> M;
            for (y = 0; 4 * y <= M; ++y) {
                remaining = M - 4 * y; 
                if (remaining % 3 == 0) { 
                    x = remaining / 3; 
                    remaining = 0;
                    cout << x << endl << y << endl;
                    break;
                }
            }
            if (remaining != 0){
                    cout << 0 << endl << 0;
            }

        }
        if (dznum == 3)
        {
            int N, M;
            cin >> N >> M;
            int vert = (M * (M + 1)) / 2;
            int hor = (N * (N + 1)) / 2;
            int rects = hor * vert;
            cout << rects;
        }
            if (dznum == 4)
        {
            int carriage[9][6]{{1, 3, 2, 4, 53, 54},{5, 7, 6, 8, 51, 52},{9, 11, 10, 12, 49, 50},{13, 15, 14, 16, 47, 48},{17, 19, 18, 20, 45, 46},{21, 23, 22, 24, 43, 44},{25, 27, 26, 28, 41, 42},{29, 31, 30, 32, 39, 40},{33, 35, 34, 36, 37, 38}};
    int carr[9][6]{{1, 3, 2, 4, 53, 54},{5, 7, 6, 8, 51, 52},{9, 11, 10, 12, 49, 50},{13, 15, 14, 16, 47, 48},{17, 19, 18, 20, 45, 46},{21, 23, 22, 24, 43, 44},{25, 27, 26, 28, 41, 42},{29, 31, 30, 32, 39, 40},{33, 35, 34, 36, 37, 38}};
    int N, n, indc, indp, add, amnt=0;
    cout << "Введите N свободных мест: ";
    cin >> N;
    for (int i=0; i<N; ++i){
        cin >> n;
        indc = index_coupe(n);
        for(int j=0; j < 6; j++)
        {
            if (carriage[indc][j] == n){
                indp = j;
                break;
            }
        }
        carriage[indc][indp] = 0;
    }

    for (int i = 0; i < 9; ++i) {
        bool allZeros = true;

        for (int j = 0; j < 6; ++j) {
            if (carriage[i][j] != 0) {
                allZeros = false;
                break;
            }
        }
        if (allZeros) {
            amnt += 1;
        }
    }
    cout << amnt;
  
        }
        if (dznum == 5)
        {
            long long N, K;
            cout << "Введите кол-во мест по середине и кол-во школьников: ";
            cin >> N >> K;
            pair<long long, long long> result = lsp(N, K);
            cout << min(result.first, result.second) << " " << max(result.first, result.second) << endl;
        }
        cout << "\nВведите номер задания (1-5): ";
        cin >> dznum;
    }
    return 0;
}
