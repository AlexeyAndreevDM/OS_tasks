#include <iostream>
#include <vector>

using namespace std;

class Asgardian {
protected:
    string name;
public:
    //Asgardian(string name) : name(name) {};
    Asgardian(const std::string& name) : name(name) {}
    virtual void greet() = 0;
    virtual void farewell() = 0;
    virtual string getType() {return "Asgardian";}
    virtual ~Asgardian() = default;
};


class Thor: public Asgardian {
public:
    Thor(): Asgardian("Thor") {cout << "I'm Thor, Son of Odin." << endl;}
    void greet() override {cout << "Listen to me well, brother." << endl;}
    void farewell() override {cout << "Farewell, my friends." << endl;}
    string getType() override {return "Thor";}
    void hammer() {cout << "My Hammer - Mjolnir, forged in the heart of a dying star." << endl;}
};

class Loki: public Asgardian {
public:
    Loki(): Asgardian("Loki") {cout << "I'm Loki, of Asgard, and I am burdened with glorious purpose." << endl;}
    void greet() override {cout << "Hello, brother." << endl;};
    void farewell() override {cout << "I assure you, Brother, the sun will shine on us again." << endl;};
    string getType() override {return "Loki";}
    void trick() {cout << "Hey! It's me! *stabbing Thor*" << endl;};
};

class Hela: public Asgardian {
public:
    Hela(): Asgardian("Hela") {cout << "I'm Hela, Goddess of Death." << endl;}
    void greet() override {cout << "Kneel before your queen." << endl;};
    void farewell() override {cout << "Darling, you have no idea what's possible." << endl;};
    string getType() override {return "Hela";};
    void ressurect() {cout << "Rise up my warriors!" << endl;};
};

class Odin: public Asgardian {
public:
    Odin(): Asgardian("Odin") {cout << "I'm Odin, Allfather of Nine Realms." << endl;};
    void greet() override {cout << "Whosoever holds this hammer, if he be worthy, shall possess the power of Thor." << endl;};
    void farewell() override {cout << "I love you, my sons" << endl;};
    string getType() override {return "Odin";};
    void wisdom() {cout << "A wise king never seeks out war, but... he must always be ready for it." << endl;};
};

void interact(Asgardian& a, Asgardian& b) {
    std::cout << a.getType() << " говорит " << b.getType() << ": ";
    a.greet();
    std::cout << b.getType() << " отвечает " << a.getType() << ": ";
    b.greet();
    std::cout << "---" << std::endl;
}

int main() {
    // Создание объектов
    // Asgardian asgardian;
    // Asgardian* ptr = &asgardian;
    // cout << "Type: " << ptr->getType() << endl
    
    Loki loki;
    loki.trick();
    Thor thor;
    thor.hammer();
    Hela hela;
    hela.ressurect();
    Odin odin;
    odin.wisdom();
    // Вызов уникальных методов
    std::cout << "---" << std::endl;

    // Вектор указателей на базовый класс
    std::vector<Asgardian*> asgardians = {&loki, &thor, &hela, &odin};
    for (auto* character : asgardians) {
        std::cout << "Name: " << character->getType() << std::endl;
        character->greet();
        character->farewell();
        std::cout << "---" << std::endl;
    }

    // Демонстрация взаимодействия
    interact(loki, thor);
    interact(hela, odin);

    return 0;
}