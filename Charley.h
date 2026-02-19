#include <iostream>
#include "Insight.h"

using namespace std;

class Charley: public Insight {
public:
    Charley(): Insight("Charley") {cout << "Charley has been launched.\n";};
    string getType() override {return "IN-02";};
    void target(int n) override {cout << "Charley targets acquired: " << n << endl;};
    ~Charley() {cout << "Charley has been destroyed.\n";};
};