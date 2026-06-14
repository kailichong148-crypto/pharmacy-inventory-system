#ifndef LOGIN_H
#define LOGIN_H

#include <string>
#include <cppconn/connection.h>

int chooseRole();
std::string login(sql::Connection* conn, int role);
std::string getHiddenPassword();

#endif

