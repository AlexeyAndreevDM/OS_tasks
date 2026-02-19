#include <iostream>
#include <math.h>
#include <cmath>
#include <limits>
#include <algorithm>
#include <iomanip>
#include <string>

using namespace std;
// setlocale(LC_ALL, "rus");


void n1()
{
    double l, r1, r2, h;
    cout << "Введите размеры конуса (высота, больший радиус, меньший радиус): ";
    cin >> h >> r1 >> r2;
    while (r1 <= r2 || r1 <= 0 || r2 <= 0)
    {
        cout << "\nВведите сначала больший радиус, потом меньший радиус :(\n";
        cin >> r1 >> r2;
    }
    while (h <= 0)
    {
        cout << "\nВведите высоту корректно :(\n";
        cin >> h;
    }
    l = sqrt(pow(h, 2)+pow((r1-r2), 2));
    cout << "\nV = " << 1./3*M_1_PI*h*(pow(r1, 2)+r1*r2+pow(r2, 2)) << "\nS = " << M_1_PI*(pow(r1, 2)+(r1+r2)*l+pow(r2, 2));

}

void n2()
{
    double x, a;
    cout << "Введите a, x: ";
    cin >> a >> x;
    while (x == 0 || a - x*x < 0 && abs(x)>=1)
    {
        cout << "Введите a, x заново :(\n";
        cin >> a >> x;
    }
    if (abs(x) < 1)
    {
        cout << a * log(abs(x));
    }
    else if (abs(x) >= 1)
    {
        cout << sqrt(a-x*x);
    }

}

void n3()
{
    double x, y, b;
    cout << "Введите x, y, b: ";
    cin >> x >> y >> b;
    while (b - y <= 0 || b - x < 0)
    {
        cout << "\nВведите заново :(\n";
        cin >> x >> y >> b;
    }
    cout << "z=" << log(b-y)*sqrt(b-x);
    

}

bool isdigit(const std::string& s) {
    return !s.empty() && (s.find_first_not_of("0123456789") == s.npos);
}

void n4()
{
    string a;
    cout << "Введите произвольное целое a: ";
    cin >> a;
    if (isdigit(a))
    {
        for (int i=stoi(a); i < stoi(a)+10; i++) cout << i << " ";
    }
    else
    {
        cout << "Некорректный ввод\n";
    }
    

}

void n5()
{
    for (double x=-4; x<=4; x+=0.5)
    {
        cout << "\nПри x = " << x << " y = " << (x*x - 2*x + 2) / (x - 1) << "\n";
    }

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
           n1();
        }
        if (dznum == 2)
        {
           n2();
        }
        if (dznum == 3)
        {
           n3();
        }
        if (dznum == 4)
        {
           n4();
        }
        if (dznum == 5)
        {
           n5();
        }
        cout << "\nВведите номер задания (1-5): ";
        cin >> dznum;
    }
    return 0;
}

