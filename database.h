#ifndef DATABASE_H
#define DATABASE_H

#include <mysql.h>
#include <string>
using namespace std;

// Connects to MySQL using the details in db_config.txt.
// Returns NULL if the connection fails.
MYSQL* connectDatabase();

// Closes the connection when the program ends.
void closeDatabase(MYSQL* conn);

// Makes user-typed text safe to place inside an SQL query.
string escapeText(MYSQL* conn, string text);

#endif