#include <iostream>
#include "database.h"
#include "train.h"
#include "reservation.h"
#include "input.h"
using namespace std;

int main()
{
    MYSQL* conn = connectDatabase();
    if (conn == NULL)
    {
        return 1;   // Cannot continue without the database.
    }

    int choice = -1;
    while (choice != 0)
    {
        cout << endl;
        cout << "===== RAILWAY TICKET RESERVATION SYSTEM =====" << endl;
        cout << "1. Add train" << endl;
        cout << "2. Display all trains (with available seats)" << endl;
        cout << "3. Search train by number" << endl;
        cout << "4. Search train by destination" << endl;
        cout << "5. Display trains sorted by number" << endl;
        cout << "6. Book ticket" << endl;
        cout << "7. Display all tickets and passengers" << endl;
        cout << "8. Cancel ticket" << endl;
        cout << "9. Display waiting list" << endl;
        cout << "0. Exit" << endl;
        choice = readInt("Enter your choice: ");

        switch (choice)
        {
            case 1: addTrain(conn); break;
            case 2: displayAllTrains(conn); break;
            case 3: searchTrainByNumber(conn); break;
            case 4: searchTrainByDestination(conn); break;
            case 5: displayTrainsSorted(conn); break;
            case 6: bookTicket(conn); break;
            case 7: displayAllTickets(conn); break;
            case 8: cancelTicket(conn); break;
            case 9: displayWaitingList(conn); break;
            case 0: cout << "Goodbye." << endl; break;
            default: cout << "Invalid choice. Please try again." << endl;
        }
    }

    closeDatabase(conn);
    return 0;
}