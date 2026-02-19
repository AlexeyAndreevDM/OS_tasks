#include <iostream>

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

class Bravo: public Insight {
public:
    Bravo(): Insight("Bravo") {cout << "Bravo has been launched.\n";};
    string getType() override {return "IN-01";};
    void target(int n) override {cout << "Bravo targets acquired: " << n << endl;};
    ~Bravo() {cout << "Bravo has been destroyed.\n";};
};

class Charley: public Insight {
public:
    Charley(): Insight("Charley") {cout << "Charley has been launched.\n";};
    string getType() override {return "IN-02";};
    void target(int n) override {cout << "Charley targets acquired: " << n << endl;};
    ~Charley() {cout << "Charley has been destroyed.\n";};
};

class Alpha: public Insight {
public:
    Alpha(): Insight("Alpha") {cout << "Alpha has been launched.\n";};
    string getType() override {return "IN-03";};
    void target(int n) override {cout << "Alpha targets acquired: " << n << endl;};
    ~Alpha() {cout << "Alpha has been destroyed.\n";};
};

int main() {
    Insight insight("Project Insight");
    Bravo bravo;
    Charley charley;
    Alpha alpha;
    if (bravo.status() && charley.status() && alpha.status()) {
        cout << "Project Insight Launch Status: " << insight.launch() << endl;
    }

    cout << "Bravo Type: " << bravo.getType() << endl;
    cout << "Bravo Status: " << bravo.launch() << endl;

    cout << "Charley Type: " << charley.getType() << endl;
    cout << "Charley Status: " << charley.launch() << endl;

    cout << "Alpha Type: " << alpha.getType() << endl;
    cout << "Alpha Status: " << alpha.launch() << endl;
    
    int targets = 3;
    insight.target(targets);
    bravo.target(targets-1);
    charley.target(targets-1);
    alpha.target(targets-1);

    cout << "Fire Status: " << insight.fire(1) << endl;

    return 0;
}