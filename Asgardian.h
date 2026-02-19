#include <iostream>
#pragma once

using namespace std;

class Asgardian {
protected:
    string name;
public:
    //Asgardian(string name) : name(name) {};
    Asgardian(const string& name) : name(name) {}
    virtual void greet() = 0;
    virtual void farewell() = 0;
    virtual string getType() {return "Asgardian";}
    virtual ~Asgardian() = default;
};
