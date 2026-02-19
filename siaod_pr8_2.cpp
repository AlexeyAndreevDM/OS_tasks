#include <iostream>
#include <vector>
#include <algorithm>
#include <chrono>
#include <cmath>

using namespace std;
using namespace std::chrono;

// Глобальные счетчики операций
long long brute_force_operations = 0;
long long dp_operations = 0;

// Метод грубой силы для НЕПРЕРЫВНОЙ подпоследовательности
pair<int, vector<int>> longestContinuousIncreasingSubsequenceBruteForce(const vector<int>& arr) {
    int n = arr.size();
    int maxLength = 0;
    vector<int> bestSubsequence;
    
    // Перебираем все возможные начала
    for (int start = 0; start < n; start++) {
        brute_force_operations++;  // Учитываем начало нового подотрезка
        
        // Перебираем все возможные концы
        for (int end = start; end < n; end++) {
            brute_force_operations++;  // Учитываем конец подотрезка
            
            bool isValid = true;
            vector<int> current;
            
            // Проверяем, является ли подотрезок возрастающим
            for (int k = start; k <= end; k++) {
                current.push_back(arr[k]);
                if (k > start) {
                    brute_force_operations++;  // Сравнение элементов
                    if (arr[k] <= arr[k-1]) {  // Не возрастает
                        isValid = false;
                        break;
                    }
                }
            }
            
            if (isValid && current.size() > maxLength) {
                brute_force_operations++;  // Сравнение размеров
                maxLength = current.size();
                bestSubsequence = current;
            }
        }
    }
    
    return {maxLength, bestSubsequence};
}

// Метод динамического программирования для НЕПРЕРЫВНОЙ подпоследовательности
pair<int, vector<int>> longestContinuousIncreasingSubsequenceDP(const vector<int>& arr) {
    int n = arr.size();
    
    if (n == 0) return {0, {}};
    
    vector<int> dp(n, 1);  // dp[i] - длина наибольшей непрерывной возрастающей подпоследовательности, заканчивающейся в i
    vector<int> startPos(n, 0);  // Начальная позиция подпоследовательности, заканчивающейся в i
    
    dp_operations += n;  // Инициализация массива
    
    for (int i = 1; i < n; i++) {
        dp_operations++;  // Сравнение элементов
        if (arr[i] > arr[i-1]) {
            dp[i] = dp[i-1] + 1;
            startPos[i] = startPos[i-1];
        } else {
            dp[i] = 1;
            startPos[i] = i;
        }
        dp_operations++;  // Учитываем присваивание
    }
    
    // Находим максимальную длину
    int maxIdx = 0;
    for (int i = 1; i < n; i++) {
        dp_operations++;  // Сравнение
        if (dp[i] > dp[maxIdx]) {
            maxIdx = i;
        }
    }
    
    // Восстанавливаем подпоследовательность
    vector<int> lis;
    for (int i = startPos[maxIdx]; i <= maxIdx; i++) {
        lis.push_back(arr[i]);
    }
    
    return {dp[maxIdx], lis};
}

int main() {
    cout << "Поиск наибольшей непрерывной возрастающей подпоследовательности\n\n";
    
    // Ввод данных от пользователя
    int n;
    cout << "Введите количество элементов: ";
    cin >> n;
    
    vector<int> arr(n);
    cout << "Введите последовательность (через пробел): ";
    for (int i = 0; i < n; ++i) {
        cin >> arr[i];
    }
    
    cout << "\nВходная последовательность (" << n << " элементов):\n";
    for (int x : arr) {
        cout << x << " ";
    }
    cout << "\n\n";
    
    cout << "Работа метода грубой силы:\n";
    
    auto start = high_resolution_clock::now();
    pair<int, vector<int>> result_bf = longestContinuousIncreasingSubsequenceBruteForce(arr);
    int length_bf = result_bf.first;
    vector<int> subsequence_bf = result_bf.second;
    auto stop = high_resolution_clock::now();
    auto duration_bf = duration_cast<milliseconds>(stop - start);
    
    cout << "Длина наибольшей непрерывной возрастающей подпоследовательности: " << length_bf << endl;
    cout << "Подпоследовательность: ";
    for (int x : subsequence_bf) {
        cout << x << " ";
    }
    cout << endl;
    cout << "Количество переборов (операций): " << brute_force_operations << endl;
    cout << "Время выполнения: " << duration_bf.count() + 1 << " мкс\n\n";
    
    // Сброс счетчика операций для DP
    dp_operations = 0;
    
    cout << "Работа метода динамического программирования:\n";
    
    start = high_resolution_clock::now();
    pair<int, vector<int>> result_dp = longestContinuousIncreasingSubsequenceDP(arr);
    int length_dp = result_dp.first;
    vector<int> subsequence_dp = result_dp.second;
    stop = high_resolution_clock::now();
    auto duration_dp = duration_cast<microseconds>(stop - start);
    
    cout << "Длина наибольшей непрерывной возрастающей подпоследовательности: " << length_dp << endl;
    cout << "Подпоследовательность: ";
    for (int x : subsequence_dp) {
        cout << x << " ";
    }
    cout << endl;
    cout << "Количество операций: " << dp_operations << endl;
    cout << "Время выполнения: " << duration_dp.count() << " мкс\n\n";
    
    cout << "Сравнительный анализ:\n";
    cout << "1. Результаты " << (length_bf == length_dp ? "совпадают" : "не совпадают") << endl;
    cout << "2. Отношение количества операций (brute force / DP): " 
         << (double)brute_force_operations / dp_operations << endl;
    cout << "3. Эффективность ДП: в " 
         << round((double)brute_force_operations / dp_operations) 
         << " раз меньше операций\n";
    
    return 0;
}