#ifndef RESERVATION_H
#define RESERVATION_H

#include <string>
#include <queue>
#include "database.h"
using namespace std;

// All the details of one passenger.
struct Passenger
{
    int passengerId;
    string name;
    int age;
    string gender;
    string phone;
};

// One booking: links a passenger to a train.
struct Ticket
{
    int ticketId;
    int passengerId;
    int trainNumber;
    int seatNumber;      // 0 means no seat yet (waiting).
    string status;       // CONFIRMED, WAITING or CANCELLED
};

// Loads the waiting tickets of one train into a queue, oldest first.
queue<Ticket> loadWaitingQueue(MYSQL* conn, int trainNumber);

void bookTicket(MYSQL* conn);
void cancelTicket(MYSQL* conn);
void displayAllTickets(MYSQL* conn);
void displayWaitingList(MYSQL* conn);

#endif