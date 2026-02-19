#include <iostream>
#include "Asgardian.h"
#pragma once

using namespace std;

class Loki: public Asgardian {
public:
    Loki(): Asgardian("Loki") {cout << "I'm Loki, of Asgard, and I am burdened with glorious purpose." << endl;}
    void greet() override {cout << "Hello, brother." << endl;};
    void farewell() override {cout << "I assure you, Brother, the sun will shine on us again." << endl;};
    string getType() override {return "Loki";}
    void trick() {cout << "Hey! It's me! *stabbing Thor*" << endl;};
};