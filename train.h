#ifndef TRAIN_H
#define TRAIN_H

#include <string>
#include "database.h"
using namespace std;

const int MAX_TRAINS = 100;
const int MAX_SEATS = 500;

struct Train
{
    int trainNumber;
    string trainName;
    string source;
    string destination;
    int totalSeats;
    int availableSeats;
};

int loadTrains(MYSQL* conn, Train trains[]);

int findTrainIndex(Train trains[], int count, int trainNumber);

void addTrain(MYSQL* conn);
void displayAllTrains(MYSQL* conn);


void searchTrainByNumber(MYSQL* conn);
void searchTrainByDestination(MYSQL* conn);
void displayTrainsSorted(MYSQL* conn);

#endif