#include <iostream>
#include <vector>
#include <string>
#include <tuple>

using namespace std;

// Структура данных: банковский счёт
struct BankAccount {
    int accountNumber; // Ключ: 7-значное целое число
    string fullName; // ФИО владельца
    string address; // Адрес

    // Конструктор для удобной инициализации
    BankAccount(int accNum, const string& name, const string& addr)
        : accountNumber(accNum), fullName(name), address(addr) {}
};

// Узел связного списка для цепного хеширования
struct Node {
    BankAccount data; // Данные счёта
    Node* next; // Указатель на следующий узел в цепочке

    // Конструктор узла
    Node(const BankAccount& acc) : data(acc), next(nullptr) {}
};

// Класс хеш-таблицы с цепным хешированием
class BankAccountHashTable {
private:
    vector<Node*> table; // хеш-таблица
    int size; // Текущее количество записей
    int capacity; // Текущий размер таблицы (количество корзин)

    // Простая хеш-функция на основе деления
    int hash(int key) const {
        return key % capacity;
    }

    // Метод рехеширования: увеличивает таблицу в 2 раза и перераспределяет все элементы
    void rehash() {
        vector<Node*> oldTable = table; // Сохраняем старую таблицу
        int oldCapacity = capacity;

        capacity *= 2; // Удваиваем размер
        table.assign(capacity, nullptr); // Создаём новую пустую таблицу
        size = 0; // Сбрасываем счётчик

        // Перевставляем все элементы из старой таблицы в новую
        for (int i = 0; i < oldCapacity; ++i) {
            Node* current = oldTable[i];
            while (current != nullptr) {
                // Вставляем копию данных узла
                insert(current->data.accountNumber, 
                       current->data.fullName, 
                       current->data.address);
                // Освобождаем память старого узла
                Node* toDelete = current;
                current = current->next;
                delete toDelete;
            }
        }
        cout << ">>> Рехеширование завершено. Новый размер таблицы: " << capacity << endl;
    }

public:
    // Конструктор: инициализирует таблицу заданного размера
    BankAccountHashTable(int initialCapacity = 11)
        : capacity(initialCapacity), size(0) {
        table.resize(capacity, nullptr);
    }

    // Деструктор: освобождает выделенную под таблицу память
    ~BankAccountHashTable() {
        for (int i = 0; i < capacity; ++i) {
            Node* current = table[i];
            while (current != nullptr) {
                Node* next = current->next;
                delete current;
                current = next;
            }
        }
    }

    // Вставка нового счёта
    void insert(int accountNumber, const string& fullName, const string& address) {
        // Проверяем коэффициент загрузки: если >= 0.75 — расширяем таблицу
        if (static_cast<double>(size) / capacity >= 0.75) { // static cast чтоб привести к double
            cout << ">>> Предупреждение: коэффициент загрузки >= 0.75. Запуск рехеширования...\n";
            rehash();
        }

        int index = hash(accountNumber); // Вычисляем индекс корзины
        Node* newNode = new Node(BankAccount(accountNumber, fullName, address));
        newNode->next = table[index]; // Вставляем новый узел в начало списка и к нему добавляем старые
        table[index] = newNode;
        size++;
        cout << "Счёт " << accountNumber << " успешно добавлен в корзину " << index << "\n";
    }

    // Поиск счёта по номеру
    BankAccount* search(int accountNumber) {
        int index = hash(accountNumber);
        Node* current = table[index];

        // Проходим по цепочке в поисках нужного ключа
        while (current != nullptr) {
            if (current->data.accountNumber == accountNumber) {
                return &(current->data); // Возвращаем указатель на найденные данные
            }
            current = current->next;
        }
        return nullptr; // Не найден
    }

    // Удаление счёта по номеру
    bool remove(int accountNumber) {
        int index = hash(accountNumber);
        Node* current = table[index];
        Node* prev = nullptr;
        // Ищем узел с нужным ключом
        while (current != nullptr) {
            if (current->data.accountNumber == accountNumber) {
                if (prev == nullptr) {
                    // Удаляем голову списка, если искомый - первый
                    table[index] = current->next;
                } else {
                    prev->next = current->next;
                }
                delete current;
                size--;
                cout << "Счёт " << accountNumber << " удалён из корзины " << index << ".\n";
                return true;
            }
            prev = current;
            current = current->next;
        }

        cout << "Счёт " << accountNumber << " не найден.\n";
        return false;
    }

    // Вывод всей таблицы
    void display() const {
        cout << "\n--- Содержимое хеш-таблицы ---\n";
        bool isEmpty = true;
        for (int i = 0; i < capacity; ++i) {
            cout << "Корзина " << i << ": ";
            Node* current = table[i];
            if (current == nullptr) {
                cout << "(пусто)";
            } else {
                isEmpty = false;
                while (current != nullptr) {
                    cout << "[" << current->data.accountNumber << "] ";
                    current = current->next;
                }
            }
            cout << "\n";
        }
        if (isEmpty) {
            cout << "(таблица полностью пуста)\n";
        }
        cout << "Коэффициент загрузки: " << (size * 1.0 / capacity) << "\n";
    }

    // Автоматическое заполнение 5–7 записями
    void autoFill() {
        // Подбираем номера так, чтобы при capacity=11 возникли коллизии:
        // Например: 1000001 % 11 = 2, 1000012 % 11 = 2 - колизия
        vector<tuple<int, string, string>> initialData = {
            {1000001, "Иванов Иван Иванович", "г. Москва, ул. Ленина, д.1"},
            {1000012, "Петров Пётр Петрович", "г. Санкт-Петербург, Невский пр., 10"},
            {1000023, "Сидоров Сидор Сидорович", "г. Новосибирск, ул. Кирова, 25"},
            {1000034, "Кузнецова Анна", "г. Екатеринбург, пр. Ленина, 50"},
            {1000045, "Смирнов Дмитрий", "г. Казань, ул. Баумана, 15"},
            {1000056, "Волкова Елена", "г. Нижний Новгород, пл. Минина, 3"},
            {1000066, "Морозов Алексей", "г. Челябинск, ул. Труда, 40"}
        };

        cout << "Автоматическое заполнение таблицы (7 записями)\n";
        for (const auto& entry : initialData) {
            insert(get<0>(entry), get<1>(entry), get<2>(entry));
        }
        cout << "Произведено автозаполнение\n\n";
    }
};

// Главная функция с текстовым интерфейсом
int main() {
    // Создаём хеш-таблицу с начальным размером
    BankAccountHashTable hashTable(11);

    // Автоматически заполняем таблицу тестовыми данными
    hashTable.autoFill();

    int choice;
    do {
        cout << "\n---------- МЕНЮ ----------\n";
        cout << "1. Добавить новый счёт\n";
        cout << "2. Найти счёт по номеру\n";
        cout << "3. Удалить счёт\n";
        cout << "4. Показать всю хеш-таблицу\n";
        cout << "0. Выйти из программы\n";
        cout << "---------------------\n";
        cout << "Ваш выбор: ";
        cin >> choice;

        switch (choice) {
            case 1: {
                int accNum;
                string name, addr;
                cout << "Введите 7-значный номер счёта: ";
                cin >> accNum;
                cin.ignore(); // Игнорируем символ новой строки после cin
                cout << "Введите ФИО: ";
                getline(cin, name);
                cout << "Введите адрес: ";
                getline(cin, addr);
                hashTable.insert(accNum, name, addr);
                break;
            }
            case 2: {
                int accNum;
                cout << "Введите номер счёта для поиска: ";
                cin >> accNum;
                BankAccount* acc = hashTable.search(accNum);
                if (acc) {
                    cout << "\nНайден счёт:\n";
                    cout << "Номер: " << acc->accountNumber << "\n";
                    cout << "ФИО: " << acc->fullName << "\n";
                    cout << "Адрес: " << acc->address << "\n";
                } else {
                    cout << "\nСчёт с номером " << accNum << " не найден.\n";
                }
                break;
            }
            case 3: {
                int accNum;
                cout << "Введите номер счёта для удаления: ";
                cin >> accNum;
                hashTable.remove(accNum);
                break;
            }
            case 4: {
                hashTable.display();
                break;
            }
            case 0: {
                cout << "Завершение работы прорграммы.\n";
                break;
            }
            default: {
                cout << "Некорректный выбор. Пожалуйста, введите число от 0 до 4.\n";
            }
        }
    } while (choice != 0);

    return 0;
}