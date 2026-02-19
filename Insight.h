#include <iostream>
#pragma once

using namespace std;

class Insight {
private:
    string name;
public:
    Insight(const string name) {}; //{cout << "All satelites online.\n";};
    string launch() {return "Active";};
    virtual void target(int n) {cout << "Overall targets count: " << n << endl;}; 
    string fire(int n) {return (n == 1) ? "Fire" : "Declined";};
    virtual string getType() {return "IN-00";};
    ~Insight() {}; //{cout << "All carriers have been destroyed.\n";};
    int status() {return 1;};
};