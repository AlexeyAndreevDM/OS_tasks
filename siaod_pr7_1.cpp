#include <iostream>
#include <queue>
#include <algorithm>

using namespace std;

struct Node {
    int data;      
    Node* left;   
    Node* right;  
    int height;

    Node(int value) {
        data = value;
        left = right = nullptr;
        height = 1;
    }
};

int getHeight(Node* node) {
    return node ? node->height : 0;
}

int getBalance(Node* node) {
    return node ? getHeight(node->left) - getHeight(node->right) : 0;
}

Node* rotateRight(Node* y) {
    Node* x = y->left;
    Node* T2 = x->right;

    x->right = y;
    y->left = T2;

    y->height = max(getHeight(y->left), getHeight(y->right)) + 1;
    x->height = max(getHeight(x->left), getHeight(x->right)) + 1;

    return x;
}

Node* rotateLeft(Node* x) {
    Node* y = x->right;
    Node* T2 = y->left;

    y->left = x;
    x->right = T2;

    x->height = max(getHeight(x->left), getHeight(x->right)) + 1;
    y->height = max(getHeight(y->left), getHeight(y->right)) + 1;

    return y;
}

Node* insert(Node* root, int value) {
    if (!root) return new Node(value);

    if (value < root->data)
        root->left = insert(root->left, value);
    else if (value > root->data)
        root->right = insert(root->right, value);
    else
        return root;

    root->height = 1 + max(getHeight(root->left), getHeight(root->right));
    int balance = getBalance(root);

    if (balance > 1 && value < root->left->data)
        return rotateRight(root);

    if (balance < -1 && value > root->right->data)
        return rotateLeft(root);

    if (balance > 1 && value > root->left->data) {
        root->left = rotateLeft(root->left);
        return rotateRight(root);
    }

    if (balance < -1 && value < root->right->data) {
        root->right = rotateRight(root->right);
        return rotateLeft(root);
    }

    return root;
}

// Функция для вывода дерева в древовидной форме
void printTree(Node* root, int space = 0, int gap = 4) {
    if (!root) return;
    
    // Увеличиваем отступ для правого поддерева
    space += gap;
    
    // Сначала выводим правое поддерево
    printTree(root->right, space);
    
    // Выводим текущий узел
    cout << endl;
    for (int i = gap; i < space; i++)
        cout << " ";
    cout << root->data << endl;
    
    // Затем выводим левое поддерево
    printTree(root->left, space);
}

void inOrderTraversal(Node* root) {
    if (root) {
        inOrderTraversal(root->left);
        cout << root->data << " ";
        inOrderTraversal(root->right);
    }
}

int findPathLength(Node* root, int value) {
    if (!root) return -1;
    if (root->data == value) return 0;

    if (value < root->data) {
        int leftPath = findPathLength(root->left, value);
        return (leftPath != -1) ? 1 + leftPath : -1;
    } else {
        int rightPath = findPathLength(root->right, value);
        return (rightPath != -1) ? 1 + rightPath : -1;
    }
}

int getTreeHeight(Node* root) {
    return getHeight(root);
}

void printMenu() {
    cout << "\n Меню АВЛ-дерева " << endl;
    cout << "1. Добавить элемент" << endl;
    cout << "2. Симметричный обход" << endl;
    cout << "3. Найти длину пути до значения" << endl;
    cout << "4. Найти высоту дерева" << endl;
    cout << "5. Вывести дерево" << endl;
    cout << "6. Выход" << endl;
    cout << "Выберите опцию: ";
}

int main() {
    Node* root = nullptr;
    int choice, value;

    do {
        printMenu();
        cin >> choice;

        switch (choice) {
            case 1:
                cout << "Введите значение для добавления: ";
                cin >> value;
                root = insert(root, value);
                cout << "Элемент " << value << " успешно добавлен." << endl;
                // Выводим баланс корня после вставки
                int balance;
                if (root) {
                    balance = getBalance(root);
                    cout << "Баланс корня: " << balance;
                    if (balance > 1) cout << " (Левое поддерево перевешивает)";
                    else if (balance < -1) cout << " (Правое поддерево перевешивает)";
                    else cout << " (Баланс в норме)";
                    cout << endl;
                }
                break;
            case 2:
                cout << "Симметричный обход дерева: ";
                inOrderTraversal(root);
                cout << endl;
                break;
            case 3:
                cout << "Введите значение для поиска длины пути: ";
                cin >> value;
                {
                    int pathLength = findPathLength(root, value);
                    if (pathLength != -1)
                        cout << "Длина пути от корня до значения " << value << ": " << pathLength << endl;
                    else
                        cout << "Значение " << value << " не найдено." << endl;
                }
                break;
            case 4:
                cout << "Высота дерева: " << getTreeHeight(root) << endl;
                break;
            case 5:
                cout << "Структура дерева:" << endl;
                if (root) {
                    printTree(root);
                } else {
                    cout << "Дерево пустое" << endl;
                }
                break;
            case 6:
                cout << "Завершение работы программы..." << endl;
                break;
            default:
                cout << "Неверная опция." << endl;
        }
    } while (choice != 6);

    return 0;
}