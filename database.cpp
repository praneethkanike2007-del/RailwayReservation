#include <iostream>
#include <fstream>
#include <string>
#include "database.h"
using namespace std;

MYSQL* connectDatabase()
{
    string host, user, password, databaseName;

    // Read the login details from the config file, one line each.
    ifstream configFile("db_config.txt");
    if (!configFile)
    {
        cout << "Database connection failed: db_config.txt not found." << endl;
        return NULL;
    }
    getline(configFile, host);
    getline(configFile, user);
    getline(configFile, password);
    getline(configFile, databaseName);
    configFile.close();

    MYSQL* conn = mysql_init(NULL);

    if (mysql_real_connect(conn, host.c_str(), user.c_str(), password.c_str(),
                           databaseName.c_str(), 3306, NULL, 0) == NULL)
    {
        cout << "Database connection failed." << endl;
        cout << "Reason: " << mysql_error(conn) << endl;
        mysql_close(conn);
        return NULL;
    }

    return conn;
}

void closeDatabase(MYSQL* conn)
{
    mysql_close(conn);
}

string escapeText(MYSQL* conn, string text)
{
    char buffer[201];

    if (text.length() > 100)
    {
        text = text.substr(0, 100);
    }

    mysql_real_escape_string(conn, buffer, text.c_str(), text.length());

    return string(buffer);
}