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
#include <fstream>
#include <sstream>
#include <vector>
#include <string>

using namespace std;
// setlocale(LC_ALL, "rus");

const double PI = 3.14;

int nodDiv(int a, int b) {
    while (b != 0) {
        int temp = b;
        b = a % b;
        a = temp;
    }
    return a;
}

int nodSub(int a, int b) {
    while (a != b) {
        if (a > b) {
            a -= b;
        } else {
            b -= a;
        }
    }
    return a;
}

int fact(int n){
    int i=1, res=1;
    while (i<=n){
        res*=i;
        i++;
    }
    return res;
}

bool containsAllCharacters(const string& s, const string& word) {
    for (char c: word) {
        if (s.find(c) == string::npos) {
            return false;
        }
    }
    return true;
}

string reverseString(const string & str) {
    string reversed = str;
    reverse(reversed.begin(), reversed.end());
    return reversed;
}

struct Book {
    string author;
    string title;
    int year;
};

void writeBooksToFile(string filename, vector<Book> books) {
    ofstream outFile(filename);
    if (!outFile) {
        cerr << "Ошибка при открытии файла для записи." << endl;
        return;
    }
    for (Book book: books) {
        outFile << book.author << " " << book.title << " " << book.year << endl;
    }
}

void findBook(vector<Book> books, string author, int year) {
    bool found = false;
    for (Book book: books) {
        if (book.author == author && book.year == year) {
            cout << "Найдена книга: " << book.title << endl;
            found = true;
            break;
        }
    }
    if (!found) {
        cout << "Такой книги нет." << endl;
    }
}

bool hasPascalInTitle(vector<Book> books) {
    for (Book book: books) {
        if (book.title.find("Паскаль") != string::npos) {
            return true; 
        }
    }
    return false; 
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
            int a, b, c;
            cout << "Введите два числа: ";
            cin >> a >> b;
            int result = nodDiv(a, b);
            cout << "НОД (делением): " << result << endl;
            result = nodSub(a, b);
            cout << "НОД (вычитанием): " << result;
        }
        if (dznum == 2)
        {
            int n, m=2, j;
            cout << "Введите верхнюю границу поиска: ";
            cin >> n;
            int a[n];
            for (int i = 0; i<n; ++i)
            {
               a[i] = i;
            }
            while (m < n) {
             if (a[m] != 0){
                 j = m * 2;
                 while (j < n) {
                        a[j]=0;
                     j += m;
                    }
                    m += 1;
                }
                else {
                    m += 1;
                }
            }  
            for (int i = 0; i<n; i++)
            {
               if (a[i] != 0) {
                   cout << a[i] << " ";
                }
            }
        }
        if (dznum == 3)
        {
            cout << "Задание 3, вариант задания 2\n";
            int rows;
            cout << "Введите количество строк: ";
            cin >> rows;
            cin.ignore();

            vector<string> lines(rows);

            for (int i = 0; i < rows; ++i) {
                cout << "Введите строку " << (i + 1) << ": ";
                getline(cin, lines[i]);
            }

            ofstream outFile("text.txt");
            if (outFile.is_open()) {
                for (int i = 0; i < rows; ++i) {
                   outFile << lines[i] << endl;
                }
                outFile.close();
            }

             ifstream inFile("text.txt");
            if (inFile.is_open()) {
                vector<string> fileslines;
            string line;

             while (getline(inFile, line)) {
               fileslines.push_back(line);
            }
             inFile.close();

             int maxLength = 0;
                for (int i = 0; i < fileslines.size(); ++i) {
                   if (fileslines[i].length() > maxLength) {
                       maxLength = fileslines[i].length();
                    }
                }

                for (int i = 0; i < maxLength; ++i) {
                    for (int j = 0; j < fileslines.size(); ++j) {
                        if (i < fileslines[j].length()) {
                            cout << fileslines[j][i] << " ";
                        } else {
                            cout << "  ";
                        }
                    }
                    cout << endl;
                }
            }
            cout << "Задание 3, вариант задания 34\n";
            string line;
            cout << "Введите строку: ";
            getline(cin, line);
            ofstream outFile1("text1.txt");
            if (outFile1.is_open()) {
                outFile1 << line;
                outFile1.close();
            }

            string searchphrase;
            cout << "Введите слово/словосочетание для поиска: ";
            cin >> searchphrase;
            ifstream inFile1("text1.txt");
            string fileContent;
            getline(inFile1, fileContent);
            inFile1.close();

            int count = 0;
            int pos = fileContent.find(searchphrase);
    
            while (pos != string::npos) {
                count++;
                pos = fileContent.find(searchphrase, pos + searchphrase.length());
            }

            cout << "Количество вхождений слова/словосочетания \"" << searchphrase << "\": " << count << endl;
        }
            if (dznum == 4)
        {
            cout << "Задание 4, вариант задания 2\n";
            int n, y=0, currn=1, cc=1, sum=1;
            cout << "Введите: ";
            cin >> n;
            for (int i=n; i>0; --i){
                cc = currn+1;
                sum = cc;
                for (int j=cc; j<cc+(n-i); ++j){
                    sum += j;
                    currn += 1;
                }
            currn += 1;
            y += fact(i)/sqrt(currn);

            }
            cout << y << endl;
            cout << "Задание 4, вариант задания 34\n";
            string s;
            cin.ignore();
            cout << "Введите строку: ";
            getline(cin, s);

            string word = "студенчество";
            if (containsAllCharacters(s, word)) {
                cout << "Все символы слова 'студенчество' содержатся в строке" << endl;
            } else {
                cout << "Не все символы слова 'студенчество' содержатся в строке" << endl;
            }


        }
        if (dznum == 5)
        {

            cout << "Задание 5, вариант задания 2\n";
            vector<Book> books;
    int n;
    cout << "Введите количество книг: ";
    cin >> n;
    cin.ignore(); 
    for (int i = 0; i < n; ++i) {
        Book book;
        cout << "Введите данные о книге (Автор Название Год): ";
        getline(cin, book.author, ' ');
        getline(cin, book.title, ' ');
        cin >> book.year;
        cin.ignore(); 
        books.push_back(book);
    }

    writeBooksToFile("books.txt", books);
    string searchAuthor;
    int searchYear;

    cout << "Введите автора для поиска: ";
    getline(cin, searchAuthor);
    cout << "Введите год издания для поиска: ";
    cin >> searchYear;
    findBook(books, searchAuthor, searchYear);
    if (hasPascalInTitle(books)) {
        cout << "Есть книга с названием, содержащим 'Паскаль'." << endl;
    } else {
        cout << "Нет книг с названием, содержащим 'Паскаль'." << endl;
    }

            cout << "Задание 5, вариант задания 34\n";
            string s;
            cout << "Введите строку: ";
            getline(cin, s);

            istringstream iss(s);
            vector<string> words;
            string word;

            while (iss >> word) {
                words.push_back(word);
            }

            if (words.size() >= 5) {
                string fw = words[4];
                string reversedfw = reverseString(fw);
                cout << "Пятое слово в перевернутом виде: " << reversedfw << endl;
            } else {
                cout << "В строке меньше пяти слов" << endl;
            }

        }
        cout << "\nВведите номер задания (1-5): ";
        cin >> dznum;
    }
    return 0;
}

