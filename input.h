#ifndef INPUT_H
#define INPUT_H

#include <string>
using namespace std;

// Keeps asking until the user types a whole number.
int readInt(string prompt);

// Keeps asking until the user types non-empty text (max 50 characters).
string readText(string prompt);

#endif