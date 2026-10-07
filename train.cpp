#include <iostream>
#include <iomanip>
#include <cstdlib>
#include "train.h"
#include "input.h"
#include <cctype>

int loadTrains(MYSQL* conn, Train trains[])
{
    string query = "SELECT train_number, train_name, source, destination, "
                   "total_seats, available_seats FROM trains";

    if (mysql_query(conn, query.c_str()) != 0)
    {
        cout << "Could not read trains. Reason: " << mysql_error(conn) << endl;
        return 0;
    }

    // The result is like a table of rows sent back by MySQL.
    MYSQL_RES* result = mysql_store_result(conn);
    if (result == NULL)
    {
        return 0;
    }

    int count = 0;
    MYSQL_ROW row;

    // Copy each row from MySQL into the next free slot of the array.
    while (count < MAX_TRAINS && (row = mysql_fetch_row(result)) != NULL)
    {
        trains[count].trainNumber    = atoi(row[0]);
        trains[count].trainName      = row[1];
        trains[count].source         = row[2];
        trains[count].destination    = row[3];
        trains[count].totalSeats     = atoi(row[4]);
        trains[count].availableSeats = atoi(row[5]);
        count++;
    }

    mysql_free_result(result);
    return count;
}

int findTrainIndex(Train trains[], int count, int trainNumber)
{
    // Linear search: check every train one by one.
    for (int i = 0; i < count; i++)
    {
        if (trains[i].trainNumber == trainNumber)
        {
            return i;
        }
    }
    return -1;   // Not found.
}

void addTrain(MYSQL* conn)
{
    Train trains[MAX_TRAINS];
    int count = loadTrains(conn, trains);

    if (count >= MAX_TRAINS)
    {
        cout << "Cannot add more trains. The limit is " << MAX_TRAINS << "." << endl;
        return;
    }

    Train newTrain;

    newTrain.trainNumber = readInt("Enter train number: ");
    if (newTrain.trainNumber <= 0)
    {
        cout << "Invalid train number. It must be greater than 0." << endl;
        return;
    }

    // Duplicate check using linear search.
    if (findTrainIndex(trains, count, newTrain.trainNumber) != -1)
    {
        cout << "A train with this number already exists." << endl;
        return;
    }

    newTrain.trainName   = readText("Enter train name: ");
    newTrain.source      = readText("Enter source station: ");
    newTrain.destination = readText("Enter destination station: ");

    newTrain.totalSeats = readInt("Enter total seats: ");
        if (newTrain.totalSeats <= 0 || newTrain.totalSeats > MAX_SEATS)
    {
        cout << "Invalid seat count. It must be between 1 and " << MAX_SEATS << "." << endl;
        return;
    }
    newTrain.availableSeats = newTrain.totalSeats;

    // Build the INSERT query and send it to MySQL.
    string query = "INSERT INTO trains VALUES (" +
                   to_string(newTrain.trainNumber) + ", '" +
                   escapeText(conn, newTrain.trainName) + "', '" +
                   escapeText(conn, newTrain.source) + "', '" +
                   escapeText(conn, newTrain.destination) + "', " +
                   to_string(newTrain.totalSeats) + ", " +
                   to_string(newTrain.availableSeats) + ")";

    if (mysql_query(conn, query.c_str()) != 0)
    {
        cout << "Could not add train. Reason: " << mysql_error(conn) << endl;
        return;
    }

    cout << "Train added successfully." << endl;
}

// Prints the column titles of the train table.
void printTrainHeader()
{
    cout << left << setw(10) << "Number" << setw(25) << "Name"
         << setw(18) << "Source" << setw(18) << "Destination"
         << setw(8) << "Total" << setw(10) << "Available" << endl;
}

// Prints the details of one train in a single line.
void printTrainRow(Train train)
{
    cout << left << setw(10) << train.trainNumber
         << setw(25) << train.trainName
         << setw(18) << train.source
         << setw(18) << train.destination
         << setw(8) << train.totalSeats
         << setw(10) << train.availableSeats << endl;
}

// Converts text to small letters so "delhi" matches "Delhi".
string toLowerText(string text)
{
    for (int i = 0; i < (int)text.length(); i++)
    {
        text[i] = tolower(text[i]);
    }
    return text;
}

void displayAllTrains(MYSQL* conn)
{
    Train trains[MAX_TRAINS];
    int count = loadTrains(conn, trains);

    if (count == 0)
    {
        cout << "No trains found." << endl;
        return;
    }

    printTrainHeader();
    for (int i = 0; i < count; i++)
    {
        printTrainRow(trains[i]);
    }
}

void searchTrainByNumber(MYSQL* conn)
{
    Train trains[MAX_TRAINS];
    int count = loadTrains(conn, trains);

    int trainNumber = readInt("Enter train number to search: ");

    // Linear search through the array.
    int index = findTrainIndex(trains, count, trainNumber);
    if (index == -1)
    {
        cout << "Train not found." << endl;
        return;
    }

    printTrainHeader();
    printTrainRow(trains[index]);
}

void searchTrainByDestination(MYSQL* conn)
{
    Train trains[MAX_TRAINS];
    int count = loadTrains(conn, trains);

    string destination = toLowerText(readText("Enter destination to search: "));
    int matches = 0;

    // Linear search: compare the destination of every train.
    for (int i = 0; i < count; i++)
    {
        if (toLowerText(trains[i].destination) == destination)
        {
            if (matches == 0)
            {
                printTrainHeader();
            }
            printTrainRow(trains[i]);
            matches++;
        }
    }

    if (matches == 0)
    {
        cout << "No trains found for this destination." << endl;
    }
}

void displayTrainsSorted(MYSQL* conn)
{
    Train trains[MAX_TRAINS];
    int count = loadTrains(conn, trains);

    if (count == 0)
    {
        cout << "No trains found." << endl;
        return;
    }

    // Bubble sort by train number, smallest first.
    // Compare neighbours and swap them if they are in the wrong order.
    for (int i = 0; i < count - 1; i++)
    {
        for (int j = 0; j < count - 1 - i; j++)
        {
            if (trains[j].trainNumber > trains[j + 1].trainNumber)
            {
                Train temp = trains[j];
                trains[j] = trains[j + 1];
                trains[j + 1] = temp;
            }
        }
    }

    printTrainHeader();
    for (int i = 0; i < count; i++)
    {
        printTrainRow(trains[i]);
    }
}