#include <iostream>
#include "Insight.h"
#include "Bravo.h"
#include "Charley.h"
#include "Alpha.h"

using namespace std;

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