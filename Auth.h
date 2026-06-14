#ifndef AUTH_H
#define AUTH_H

#include <string>
using namespace std;

/* ================= UI / DISPLAY ================= */
void printTitle();
void printHeader(const string& role = "");
void showWelcomeScreen();
void showLoginHeader(const string& role);
void showLoginSuccess(const string& role, const string& id);
void showLoginFail(int attempts);

/* ================= LOGIN UTILITIES ================= */
string getHiddenPassword();
int chooseRole();

/* ================= MAIN LOGIN ================= */
void login();

#endif
