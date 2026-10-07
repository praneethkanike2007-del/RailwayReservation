#include <iostream>
#include "input.h"

int readInt(string prompt)
{
    int value;
    while (true)
    {
        cout << prompt;
        if (cin >> value)
        {
            cin.ignore(1000, '\n');   // Throw away the rest of the line.
            return value;
        }
        // The user typed something that is not a number, like "abc".
        cout << "Invalid input. Please enter a number." << endl;
        cin.clear();                  // Reset the error state of cin.
        cin.ignore(1000, '\n');
    }
}

string readText(string prompt)
{
    string text;
    while (true)
    {
        cout << prompt;
        getline(cin, text);
        if (text.length() == 0)
        {
            cout << "This cannot be empty." << endl;
        }
        else if (text.length() > 50)
        {
            cout << "Too long. Maximum 50 characters." << endl;
        }
        else
        {
            return text;
        }
    }
}