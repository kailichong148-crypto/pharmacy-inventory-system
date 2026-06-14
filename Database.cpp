#include "Database.h"
#include <iostream>
#include <cppconn/exception.h>

using namespace std;

sql::Connection* connectDB() {
    sql::mysql::MySQL_Driver* driver = sql::mysql::get_mysql_driver_instance();
    sql::Connection* conn = nullptr;

    try {
        conn = driver->connect("tcp://127.0.0.1:3306", "root", "");
        conn->setSchema("pharmacyinventorydb");
    }
    catch (sql::SQLException& e) {
        cout << "DB connection error: " << e.what() << endl;
        exit(1);
    }
    return conn;
}
