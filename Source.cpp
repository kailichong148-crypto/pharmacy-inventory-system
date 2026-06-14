#include "Database.h"

#include <iostream>
#include <iomanip>
#include "Manager.h"
#include "Admin.h"
#include "Staff.h"
#include "Auth.h"

using namespace std;


int main() {
    login();   // login handles role + menu routing
    return 0;
}
