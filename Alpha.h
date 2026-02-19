#include <iostream>
#include "Insight.h"

using namespace std;

class Alpha: public Insight {
public:
    Alpha(): Insight("Alpha") {cout << "Alpha has been launched.\n";};
    string getType() override {return "IN-03";};
    void target(int n) override {cout << "Alpha targets acquired: " << n << endl;};
    ~Alpha() {cout << "Alpha has been destroyed.\n";};
};