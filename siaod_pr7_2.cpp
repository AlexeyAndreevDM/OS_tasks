#include <iostream>
#include <vector>
#include <climits>

using namespace std;

// Функция для нахождения вершины с минимальным ключом из SM
int findMinKey(const vector<int>& key, const vector<bool>& SP) {
    int min = INT_MAX, min_index = -1;
    
    for (int v = 0; v < key.size(); v++) {
        if (!SP[v] && key[v] < min) {
            min = key[v];
            min_index = v;
        }
    }
    return min_index;
}

// Функция для вывода остовного дерева
void printMST(const vector<int>& parent, const vector<vector<int>>& graph) {
    cout << "\nОстовное дерево (рёбра и веса):" << endl;
    int totalWeight = 0;
    for (int i = 1; i < parent.size(); i++) {
        if (parent[i] != -1) {
            cout << parent[i] + 1 << " - " << i + 1 << " : " << graph[i][parent[i]] << endl;
            totalWeight += graph[i][parent[i]];
        }
    }
    cout << "Общий вес остовного дерева: " << totalWeight << endl;
}

// Алгоритм Прима с матрицей смежности
void primMST(const vector<vector<int>>& graph) {
    int V = graph.size();
    vector<int> parent(V);    // Массив для хранения остовного дерева
    vector<int> key(V, INT_MAX);  // Ключи для выбора минимального веса
    vector<bool> SP(V, false);   // SP - вершины, включенные в остовное дерево
    
    // Начинаем с вершины 0 (вершина 1)
    key[0] = 0;
    parent[0] = -1; // У корня нет родителя
    
    // Построение остовного дерева из V-1 ребер
    for (int count = 0; count < V - 1; count++) {
        // Выбираем вершину с минимальным ключом из SM
        int u = findMinKey(key, SP);
        
        // Добавляем выбранную вершину в SP
        SP[u] = true;
        
        // Обновляем ключи соседних вершин
        for (int v = 0; v < V; v++) {
            // graph[u][v] != 0 - существует ребро
            // !SP[v] - вершина еще не в остовном дереве
            // graph[u][v] < key[v] - найден меньший вес
            if (graph[u][v] != 0 && !SP[v] && graph[u][v] < key[v]) {
                parent[v] = u;
                key[v] = graph[u][v];
            }
        }
    }
    
    printMST(parent, graph);
}

int main() {
    cout << "ПРЕДЛОЖЕННЫЙ ГРАФ 2-ГО ВАРИАНТА" << endl;
    
    // Фиксированный граф из задания
    int V = 6;
    vector<vector<int>> graph(V, vector<int>(V, 0));
    
    // Заполнение матрицы смежности для предложенного графа
    // Вершины: 0=1, 1=2, 2=3, 3=4, 4=5, 5=6
    
    // 1-2:7
    graph[0][1] = 7; graph[1][0] = 7;
    
    // 1-6:4
    graph[0][5] = 4; graph[5][0] = 4;
    
    // 2-3:1
    graph[1][2] = 1; graph[2][1] = 1;
    
    // 3-5:3
    graph[2][4] = 3; graph[4][2] = 3;
    
    // 5-6:8
    graph[4][5] = 8; graph[5][4] = 8;
    
    // 4-1:2
    graph[3][0] = 2; graph[0][3] = 2;
    
    // 4-2:2
    graph[3][1] = 2; graph[1][3] = 2;
    
    // 4-3:2
    graph[3][2] = 2; graph[2][3] = 2;
    
    // 4-5:6
    graph[3][4] = 6; graph[4][3] = 6;
    
    // 4-6:1
    graph[3][5] = 1; graph[5][3] = 1;
    
    // Вывод матрицы смежности предложенного графа
    cout << "Матрица смежности предложенного графа:" << endl << endl;
    cout << "   ";
    for (int i = 0; i < V; i++) {
        cout << i+1 << " ";
    }
    cout << endl << endl;
    
    for (int i = 0; i < V; i++) {
        cout << i+1 << "  ";
        for (int j = 0; j < V; j++) {
            cout << graph[i][j] << " ";
        }
        cout << endl;
    }
    
    // Запуск алгоритма Прима для предложенного графа
    primMST(graph);
    
    cout << "\nПОЛЬЗОВАТЕЛЬСКИЙ ВВОД" << endl;
    
    // Ввод пользовательского графа
    cout << "Введите количество вершин графа: ";
    cin >> V;
    
    
    // Инициализация матрицы смежности VxV
    vector<vector<int>> userGraph(V, vector<int>(V, 0));
    
    cout << "\nВвод рёбер графа (для завершения ввода введите 0):" << endl;
    cout << "Формат: вершина1 вершина2 вес" << endl;
    cout << "Примечание: вершины нумеруются от 1 до " << V << endl;
    
    int v1, v2, weight;
    while (true) {
        cout << "Введите ребро: ";
        cin >> v1;
        
        // Проверка на завершение ввода
        if (v1 == 0) break;
        
        cin >> v2 >> weight;
        
        // Заполнение матрицы смежности (граф неориентированный)
        userGraph[v1-1][v2-1] = weight;
        userGraph[v2-1][v1-1] = weight;
        
        cout << "Добавлено ребро: " << v1 << " - " << v2 << " (вес: " << weight << ")" << endl;
    }
    
    // Вывод матрицы смежности пользовательского графа
    cout << "\nМатрица смежности пользовательского графа:" << endl << endl;
    cout << "   ";
    for (int i = 0; i < V; i++) {
        cout << i+1 << " ";
    }
    cout << endl << endl;
    
    for (int i = 0; i < V; i++) {
        cout << i+1 << "  ";
        for (int j = 0; j < V; j++) {
            cout << userGraph[i][j] << " ";
        }
        cout << endl;
    }
    
    // Запуск алгоритма Прима для пользовательского графа
    primMST(userGraph);
    
    return 0;
}