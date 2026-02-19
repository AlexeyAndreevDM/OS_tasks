#include <iostream>
#include "Asgardian.h"
#pragma once

using namespace std;

class Odin: public Asgardian {
public:
    Odin(): Asgardian("Odin") {cout << "I'm Odin, Allfather of Nine Realms." << endl;};
    void greet() override {cout << "Whosoever holds this hammer, if he be worthy, shall possess the power of Thor." << endl;};
    void farewell() override {cout << "I love you, my sons" << endl;};
    string getType() override {return "Odin";};
    void wisdom() {cout << "A wise king never seeks out war, but... he must always be ready for it." << endl;};
};