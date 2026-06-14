#include "InputHelper.h"
#include <iostream>
using namespace std;

/* Read integer，double,string input.
   Return false if user enters 0 (go back). */
bool inputInt(int& value) {
    cin >> value;
    if (value == 0) return false;
    return true;
}

bool inputDouble(double& value) {
    cin >> value;
    if (value == 0) return false;
    return true;
}

bool inputString(string& value) {
    cin >> ws;
    getline(cin, value);
    if (value == "0") return false;
    return true;
}

void showBackInstruction() {
    cout << "(Enter 0 to return to  menu)\n\n";
}

void pauseScreen() {
    cout << "\nPress Enter to continue...";
    cin.clear();                 // Clear error flags
    cin.ignore(10000, '\n');     // Discard remaining input
    cin.get();                    // Wait for Enter key
}
