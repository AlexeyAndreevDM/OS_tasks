#include <iostream>
#include <random>
#include <string> 
#include <algorithm> 

using namespace std;

int main()
{
    random_device rd;
    mt19937 gen(rd());
    uniform_int_distribution<> dist(1, 9);
    int num=0, n, r=1000, pl=0, ms=0;
    string s, ch, sugg;
    while (1){
    while (r > 0){
        n = dist(gen);
        s = to_string(n);
        ch = to_string(num);
        size_t found = ch.find(s);
        if (found == string::npos){
            num += n*r;
            r = r/10;
        }
    }
    pl=0;
    ms=0;
    cout << "Спойлер: " << num << endl;
    cout << "Введите число: ";
    cin >> sugg;
    ch = to_string(num);
    for (int i=0; i<4; ++i){
        if (ch[i] == sugg[i]) pl++;
        else ms++;
    }
    cout << pl << " " << ms << endl;
    if (pl == 4){
        cout << "УГАДАЛ!!!";
        break;
    }
    }

    return 0;
}