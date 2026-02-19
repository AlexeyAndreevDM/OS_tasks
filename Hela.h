#include <iostream>
#include "Asgardian.h"
#pragma once

using namespace std;

class Hela: public Asgardian {
public:
    Hela(): Asgardian("Hela") {cout << "I'm Hela, Goddess of Death." << endl;}
    void greet() override {cout << "Kneel before your queen." << endl;};
    void farewell() override {cout << "Darling, you have no idea what's possible." << endl;};
    string getType() override {return "Hela";};
    void ressurect() {cout << "Rise up my warriors!" << endl;};
};