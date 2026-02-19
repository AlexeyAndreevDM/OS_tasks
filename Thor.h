#include <iostream>
#include "Asgardian.h"
#pragma once

using namespace std;

class Thor: public Asgardian {
public:
    Thor(): Asgardian("Thor") {cout << "I'm Thor, Son of Odin." << endl;}
    void greet() override {cout << "Listen to me well, brother." << endl;}
    void farewell() override {cout << "Farewell, my friends." << endl;}
    string getType() override {return "Thor";}
    void hammer() {cout << "My Hammer - Mjolnir, forged in the heart of a dying star." << endl;}
};
