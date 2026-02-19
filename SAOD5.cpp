#include <iostream>

class Asgardian {
protected:
    string name;
public:
    virtual void greet(name) {
        cout << "I am " << name << ", of Asgard!" << endl;
    };
    virtual void farewell(name) {
        cout << "Farewell, " << name << endl;
    };
    virtual string getType() {
        return "Asgardian";
    };
    virtual ~Asgards_Doom() {
        cout << "Asgard has been destroyed!" << endl;
    };
}

class Thor : public Asgardian {
public:
    
}

int main() {
    std::cout << "Program started." << std::endl;
    return 0;
}