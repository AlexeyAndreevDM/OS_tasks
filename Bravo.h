#include <iostream>
#include "Insight.h"
#pragma once

using namespace std;

class Bravo: public Insight {
public:
    Bravo(): Insight("Bravo") {cout << "Bravo has been launched.\n";};
    string getType() override {return "IN-01";};
    void target(int n) override {cout << "Bravo targets acquired: " << n << endl;};
    ~Bravo() {cout << "Bravo has been destroyed.\n";};
};