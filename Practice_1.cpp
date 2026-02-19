#include <iostream>
#include <fstream>
#include <vector>
#include <cstdlib>
#include <ctime>
#include <algorithm>
#include <chrono>
#include <random>

using namespace std;
using namespace std::chrono;

void task3() {
    ofstream file_for_input("input.txt");
    int max_num = 10000000;
    vector<int> numbers(max_num);
    for (int i = 0; i < max_num; ++i) {
        numbers[i] = i;
    }
    
    // Замена random_shuffle на современный подход
    random_device rd;
    mt19937 g(rd());
    shuffle(numbers.begin(), numbers.end(), g);
    
    for (const int& num : numbers) {
        file_for_input << num << endl;
    }
    file_for_input.close();
    cout << "File has been created"<< endl;
    auto start_time = high_resolution_clock::now();
    ifstream input_file("input.txt");
    ofstream output_file("output.txt");
    vector<int> bit_array(max_num, 0);
    int num;
    while (input_file >> num) {
        if (num>=0 && num<max_num) {
            bit_array[num] = 1;
        }
    }
    input_file.close();
    for (int i = 0; i < max_num; ++i) {
        if (bit_array[i]) {
            output_file<<i<<endl;
        }
    }
    output_file.close();
    auto end_time = high_resolution_clock::now();
    duration<double> duration = end_time - start_time;
    cout<<"File has been created! Data has been sorted! Time: "<<duration.count()<<endl;
    size_t memory_usage = bit_array.size() / 8;
    cout << "Memory used by bit array: " << memory_usage << " bytes" << endl;
}

int main() {
    task3();
    return 0;
}