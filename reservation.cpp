#include <iostream>
#include <iomanip>
#include <cstdlib>
#include <cctype>
#include "reservation.h"
#include "train.h"
#include "input.h"

// Sends one SQL command to MySQL. Returns false if it failed.
bool runQuery(MYSQL* conn, string query)
{
    if (mysql_query(conn, query.c_str()) != 0)
    {
        cout << "Database error: " << mysql_error(conn) << endl;
        return false;
    }
    return true;
}

// Undo every change made since START TRANSACTION.
void undoChanges(MYSQL* conn)
{
    runQuery(conn, "ROLLBACK");
    cout << "Operation failed. No changes were saved." << endl;
}

// A valid phone number has exactly 10 digits.
bool isValidPhone(string phone)
{
    if (phone.length() != 10)
    {
        return false;
    }
    for (int i = 0; i < 10; i++)
    {
        if (!isdigit(phone[i]))
        {
            return false;
        }
    }
    return true;
}

// Finds the lowest seat number that is not taken on this train.
// Returns -1 if every seat is taken.
int findFreeSeat(MYSQL* conn, int trainNumber, int totalSeats)
{
    // seatTaken[5] == true means seat number 5 is already booked.
    bool seatTaken[MAX_SEATS + 1];
    for (int i = 0; i <= MAX_SEATS; i++)
    {
        seatTaken[i] = false;
    }

    string query = "SELECT seat_number FROM tickets WHERE train_number = " +
                   to_string(trainNumber) + " AND status = 'CONFIRMED'";
    if (!runQuery(conn, query))
    {
        return -1;
    }

    MYSQL_RES* result = mysql_store_result(conn);
    if (result == NULL)
    {
        return -1;
    }

    // Mark every seat that a confirmed ticket is using.
    MYSQL_ROW row;
    while ((row = mysql_fetch_row(result)) != NULL)
    {
        int seat = atoi(row[0]);
        if (seat >= 1 && seat <= MAX_SEATS)
        {
            seatTaken[seat] = true;
        }
    }
    mysql_free_result(result);

    // Linear search for the first free seat.
    for (int seat = 1; seat <= totalSeats; seat++)
    {
        if (!seatTaken[seat])
        {
            return seat;
        }
    }
    return -1;
}

queue<Ticket> loadWaitingQueue(MYSQL* conn, int trainNumber)
{
    queue<Ticket> waitingQueue;

    // ORDER BY ticket_id: the smallest ticket ID joined the waiting list first.
    string query = "SELECT ticket_id, passenger_id FROM tickets WHERE train_number = " +
                   to_string(trainNumber) +
                   " AND status = 'WAITING' ORDER BY ticket_id";
    if (!runQuery(conn, query))
    {
        return waitingQueue;
    }

    MYSQL_RES* result = mysql_store_result(conn);
    if (result == NULL)
    {
        return waitingQueue;
    }

    MYSQL_ROW row;
    while ((row = mysql_fetch_row(result)) != NULL)
    {
        Ticket ticket;
        ticket.ticketId    = atoi(row[0]);
        ticket.passengerId = atoi(row[1]);
        ticket.trainNumber = trainNumber;
        ticket.seatNumber  = 0;
        ticket.status      = "WAITING";

        waitingQueue.push(ticket);   // Add at the back of the queue.
    }

    mysql_free_result(result);
    return waitingQueue;
}

// Returns the name of a passenger, or "Unknown" if not found.
string getPassengerName(MYSQL* conn, int passengerId)
{
    string name = "Unknown";

    string query = "SELECT name FROM passengers WHERE passenger_id = " +
                   to_string(passengerId);
    if (!runQuery(conn, query))
    {
        return name;
    }

    MYSQL_RES* result = mysql_store_result(conn);
    if (result == NULL)
    {
        return name;
    }

    MYSQL_ROW row = mysql_fetch_row(result);
    if (row != NULL)
    {
        name = row[0];
    }

    mysql_free_result(result);
    return name;
}

void bookTicket(MYSQL* conn)
{
    Train trains[MAX_TRAINS];
    int count = loadTrains(conn, trains);

    if (count == 0)
    {
        cout << "No trains found. Add a train first." << endl;
        return;
    }

    Ticket ticket;
    ticket.trainNumber = readInt("Enter train number: ");

    int index = findTrainIndex(trains, count, ticket.trainNumber);
    if (index == -1)
    {
        cout << "Train not found." << endl;
        return;
    }

    // Read and check the passenger details.
    Passenger passenger;
    passenger.name = readText("Enter passenger name: ");

    passenger.age = readInt("Enter age: ");
    if (passenger.age < 1 || passenger.age > 120)
    {
        cout << "Invalid age. It must be between 1 and 120." << endl;
        return;
    }

    passenger.gender = readText("Enter gender (M/F/O): ");
    if (passenger.gender.length() == 1)
    {
        passenger.gender[0] = toupper(passenger.gender[0]);
    }
    if (passenger.gender != "M" && passenger.gender != "F" && passenger.gender != "O")
    {
        cout << "Invalid gender. Enter M, F or O." << endl;
        return;
    }

    passenger.phone = readText("Enter 10-digit phone number: ");
    if (!isValidPhone(passenger.phone))
    {
        cout << "Invalid phone number. It must have exactly 10 digits." << endl;
        return;
    }

    // A transaction groups several changes so that either ALL of them
    // are saved or NONE of them are saved.
    if (!runQuery(conn, "START TRANSACTION"))
    {
        return;
    }

    // 1. Save the passenger.
    string query = "INSERT INTO passengers (name, age, gender, phone) VALUES ('" +
                   escapeText(conn, passenger.name) + "', " +
                   to_string(passenger.age) + ", '" +
                   passenger.gender + "', '" +
                   passenger.phone + "')";
    if (!runQuery(conn, query))
    {
        undoChanges(conn);
        return;
    }

    // MySQL generated the passenger ID automatically. Ask what it was.
    passenger.passengerId = (int)mysql_insert_id(conn);
    ticket.passengerId = passenger.passengerId;

    if (trains[index].availableSeats > 0)
    {
        // 2a. A seat is free: create a CONFIRMED ticket.
        ticket.seatNumber = findFreeSeat(conn, ticket.trainNumber, trains[index].totalSeats);
        if (ticket.seatNumber == -1)
        {
            undoChanges(conn);
            return;
        }
        ticket.status = "CONFIRMED";

        query = "INSERT INTO tickets (passenger_id, train_number, seat_number, status) VALUES (" +
                to_string(ticket.passengerId) + ", " +
                to_string(ticket.trainNumber) + ", " +
                to_string(ticket.seatNumber) + ", 'CONFIRMED')";
        if (!runQuery(conn, query))
        {
            undoChanges(conn);
            return;
        }
        ticket.ticketId = (int)mysql_insert_id(conn);

        // 3. One seat fewer is available now.
        query = "UPDATE trains SET available_seats = available_seats - 1 WHERE train_number = " +
                to_string(ticket.trainNumber);
        if (!runQuery(conn, query))
        {
            undoChanges(conn);
            return;
        }

        if (!runQuery(conn, "COMMIT"))
        {
            undoChanges(conn);
            return;
        }

        cout << "Ticket booked successfully." << endl;
        cout << "Ticket ID: " << ticket.ticketId << endl;
        cout << "Seat number: " << ticket.seatNumber << endl;
    }
    else
    {
        // 2b. Train is full: create a WAITING ticket with no seat.
        ticket.seatNumber = 0;
        ticket.status = "WAITING";

        query = "INSERT INTO tickets (passenger_id, train_number, seat_number, status) VALUES (" +
                to_string(ticket.passengerId) + ", " +
                to_string(ticket.trainNumber) + ", NULL, 'WAITING')";
        if (!runQuery(conn, query))
        {
            undoChanges(conn);
            return;
        }
        ticket.ticketId = (int)mysql_insert_id(conn);

        if (!runQuery(conn, "COMMIT"))
        {
            undoChanges(conn);
            return;
        }

        // The new passenger is at the back, so position = size of the queue.
        queue<Ticket> waitingQueue = loadWaitingQueue(conn, ticket.trainNumber);

        cout << "No seats available. Passenger added to the waiting list." << endl;
        cout << "Ticket ID: " << ticket.ticketId << endl;
        cout << "Waiting position: " << waitingQueue.size() << endl;
    }
}

void cancelTicket(MYSQL* conn)
{
    int ticketId = readInt("Enter ticket ID to cancel: ");
    if (ticketId <= 0)
    {
        cout << "Invalid ticket ID." << endl;
        return;
    }

    // Step 1: find the ticket.
    string query = "SELECT train_number, seat_number, status FROM tickets WHERE ticket_id = " +
                   to_string(ticketId);
    if (!runQuery(conn, query))
    {
        return;
    }

    MYSQL_RES* result = mysql_store_result(conn);
    if (result == NULL)
    {
        return;
    }

    MYSQL_ROW row = mysql_fetch_row(result);
    if (row == NULL)
    {
        cout << "Ticket not found." << endl;
        mysql_free_result(result);
        return;
    }

    int trainNumber = atoi(row[0]);
    int seatNumber = 0;
    if (row[1] != NULL)
    {
        seatNumber = atoi(row[1]);
    }
    string status = row[2];
    mysql_free_result(result);

    if (status == "CANCELLED")
    {
        cout << "This ticket is already cancelled." << endl;
        return;
    }

    // Step 2: see who is waiting for this train (FIFO order).
    queue<Ticket> waitingQueue = loadWaitingQueue(conn, trainNumber);

    // Step 3: make all changes inside one transaction.
    if (!runQuery(conn, "START TRANSACTION"))
    {
        return;
    }

    query = "UPDATE tickets SET status = 'CANCELLED', seat_number = NULL WHERE ticket_id = " +
            to_string(ticketId);
    if (!runQuery(conn, query))
    {
        undoChanges(conn);
        return;
    }

    if (status == "WAITING")
    {
        // A waiting ticket holds no seat, so nothing else changes.
        if (!runQuery(conn, "COMMIT"))
        {
            undoChanges(conn);
            return;
        }
        cout << "Waiting ticket cancelled." << endl;
        return;
    }

    // From here the cancelled ticket was CONFIRMED, so a seat became free.
    if (waitingQueue.empty())
    {
        // Nobody is waiting: the seat goes back to the available count.
        query = "UPDATE trains SET available_seats = available_seats + 1 WHERE train_number = " +
                to_string(trainNumber);
        if (!runQuery(conn, query))
        {
            undoChanges(conn);
            return;
        }

        if (!runQuery(conn, "COMMIT"))
        {
            undoChanges(conn);
            return;
        }

        cout << "Ticket cancelled. Seat " << seatNumber << " is now available." << endl;
    }
    else
    {
        // Give the freed seat to the passenger at the FRONT of the queue.
        Ticket nextTicket = waitingQueue.front();
        waitingQueue.pop();

        query = "UPDATE tickets SET status = 'CONFIRMED', seat_number = " +
                to_string(seatNumber) + " WHERE ticket_id = " +
                to_string(nextTicket.ticketId);
        if (!runQuery(conn, query))
        {
            undoChanges(conn);
            return;
        }

        if (!runQuery(conn, "COMMIT"))
        {
            undoChanges(conn);
            return;
        }

        cout << "Ticket cancelled." << endl;
        cout << "Waiting passenger " << getPassengerName(conn, nextTicket.passengerId)
             << " (Ticket ID " << nextTicket.ticketId << ") is now CONFIRMED with seat "
             << seatNumber << "." << endl;
    }
}

void displayAllTickets(MYSQL* conn)
{
    // JOIN combines each ticket with the passenger it belongs to.
    string query = "SELECT t.ticket_id, p.name, p.age, p.gender, p.phone, "
                   "t.train_number, t.seat_number, t.status "
                   "FROM tickets t JOIN passengers p "
                   "ON t.passenger_id = p.passenger_id "
                   "ORDER BY t.ticket_id";
    if (!runQuery(conn, query))
    {
        return;
    }

    MYSQL_RES* result = mysql_store_result(conn);
    if (result == NULL)
    {
        return;
    }

    if (mysql_num_rows(result) == 0)
    {
        cout << "No tickets found." << endl;
        mysql_free_result(result);
        return;
    }

    cout << left << setw(8) << "Ticket" << setw(22) << "Name" << setw(5) << "Age"
         << setw(8) << "Gender" << setw(13) << "Phone" << setw(8) << "Train"
         << setw(6) << "Seat" << setw(10) << "Status" << endl;

    MYSQL_ROW row;
    while ((row = mysql_fetch_row(result)) != NULL)
    {
        // A waiting or cancelled ticket has no seat, so MySQL sends NULL.
        string seat = "-";
        if (row[6] != NULL)
        {
            seat = row[6];
        }

        cout << left << setw(8) << row[0] << setw(22) << row[1] << setw(5) << row[2]
             << setw(8) << row[3] << setw(13) << row[4] << setw(8) << row[5]
             << setw(6) << seat << setw(10) << row[7] << endl;
    }

    mysql_free_result(result);
}

void displayWaitingList(MYSQL* conn)
{
    Train trains[MAX_TRAINS];
    int count = loadTrains(conn, trains);

    int trainNumber = readInt("Enter train number: ");
    if (findTrainIndex(trains, count, trainNumber) == -1)
    {
        cout << "Train not found." << endl;
        return;
    }

    queue<Ticket> waitingQueue = loadWaitingQueue(conn, trainNumber);

    if (waitingQueue.empty())
    {
        cout << "The waiting list for this train is empty." << endl;
        return;
    }

    cout << left << setw(10) << "Position" << setw(10) << "Ticket" << setw(22) << "Name" << endl;

    // Take passengers out from the front, one by one, in FIFO order.
    int position = 1;
    while (!waitingQueue.empty())
    {
        Ticket ticket = waitingQueue.front();
        waitingQueue.pop();

        cout << left << setw(10) << position << setw(10) << ticket.ticketId
             << setw(22) << getPassengerName(conn, ticket.passengerId) << endl;
        position++;
    }
}