# Railway Ticket Reservation System

Group 4. A menu-driven C++ application with a MySQL backend.

## Features

- Add, display, search and sort trains
- Book tickets with automatic ticket ID and seat number
- Cancel tickets
- Waiting list using a queue (FIFO), with automatic promotion on cancellation
- Display tickets, passengers, available seats and the waiting list
- Input validation and database error handling

## DSA concepts used

| Concept | Where |
|---|---|
| Structures | `Train`, `Passenger`, `Ticket` |
| Arrays | Array of `Train`; boolean seat array in `findFreeSeat` |
| Queue (STL) | Waiting list in `loadWaitingQueue`, `cancelTicket` |
| Strings | Names, destinations, SQL queries |
| Searching | Linear search in `findTrainIndex`, `searchTrainByDestination` |
| Sorting | Bubble sort in `displayTrainsSorted` |

## Files

| File | Purpose |
|---|---|
| `main.cpp` | Menu |
| `database.h/.cpp` | Connect to and disconnect from MySQL |
| `train.h/.cpp` | Train structure, add, display, search, sort |
| `reservation.h/.cpp` | Passengers, tickets, booking, cancellation, waiting queue |
| `input.h/.cpp` | Safe number and text input |
| `database.sql` | Creates the database and tables |
| `db_config.example.txt` | Template for your private `db_config.txt` |

## Setup on a new Windows computer

Each computer needs its own MySQL server and its own copy of the database.
The project folder does not contain the database.

1. Install MSYS2 from https://www.msys2.org (keep the default folder `C:\msys64`).
2. Open the "MSYS2 UCRT64" terminal and run:
```
   pacman -S mingw-w64-ucrt-x86_64-gcc mingw-w64-ucrt-x86_64-libmariadbclient
```
3. Add `C:\msys64\ucrt64\bin` to the Windows user PATH variable.
4. Install MySQL Community Server from https://dev.mysql.com/downloads/mysql/
   and remember the root password you set.
5. Install VS Code and the "C/C++" extension by Microsoft.
6. Create the database. Open the MySQL command line client, log in, and run:
```
   source C:/RailwayReservation/database.sql
```
7. Copy `db_config.example.txt` to a new file named `db_config.txt` and put
   your own MySQL password on line 3. The four lines are: host, user,
   password, database name.

## Build and run

Open the folder in VS Code and press `Ctrl+Shift+B`, or run in the terminal:

```
g++ main.cpp database.cpp train.cpp input.cpp reservation.cpp -o railway -IC:/msys64/ucrt64/include/mariadb -lmariadb
.\railway
```

## Common problems

| Problem | Fix |
|---|---|
| `g++ is not recognized` | Step 3 was missed. Add the PATH entry and reopen VS Code. |
| `mysql.h: No such file or directory` | The `-I...` part is missing from the compile command, or step 2 was missed. |
| `undefined reference to WinMain` | `main.cpp` is empty or not saved. |
| Program prints nothing and exits | Open the terminal from inside VS Code with this folder open, so `.vscode/settings.json` applies. |
| `db_config.txt not found` | Step 7 was missed, or the program was run from a different folder. |
| `Access denied for user 'root'` | Wrong password on line 3 of `db_config.txt`. |
| `Unknown database 'railway_reservation'` | Step 6 was missed. |

## Notes

- Developed and tested with g++ 16.2.0 (MSYS2 UCRT64) and MySQL Server 8.0.45 on Windows 10.
- The C++ code talks to MySQL through the MySQL C API (`mysql.h`), provided by
  the MySQL-compatible MariaDB Connector/C library.
- `db_config.txt` holds a private password and must not be shared or uploaded.