#include <iostream>
#include <vector>
#include "Loki.h"
#include "Thor.h"
#include "Hela.h"
#include "Odin.h"

using namespace std;

void interact(Asgardian& a, Asgardian& b) {
    cout << a.getType() << " говорит " << b.getType() << ": ";
    a.greet();
    cout << b.getType() << " отвечает " << a.getType() << ": ";
    b.greet();
    cout << "---" << std::endl;
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
    cout << "---" << std::endl;

    // Вектор указателей на базовый класс
    // vector<Asgardian*> asgardians = {&loki, &thor, &hela, &odin};
    // for (auto* character : asgardians) {
    //     cout << "Name: " << character->getType() << std::endl;
    //     character->greet();
    //     character->farewell();
    //     std::cout << "---" << std::endl;
    // }

    vector<Asgardian*> asgardians = {&loki, &thor, &hela, &odin};
    for (int i = 0; i < asgardians.size(); ++i) {
        cout << "Name: " << asgardians[i]->getType() << endl;
        asgardians[i]->greet();
        asgardians[i]->farewell();
        cout << "---" << endl;
    }

    // Демонстрация взаимодействия
    interact(loki, thor);
    interact(hela, odin);

    return 0;
}