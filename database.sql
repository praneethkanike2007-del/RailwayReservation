-- Railway Ticket Reservation System: database setup script
-- Run this once on any computer to create the database and tables.

CREATE DATABASE IF NOT EXISTS railway_reservation;
USE railway_reservation;

-- One row = one train
CREATE TABLE IF NOT EXISTS trains (
    train_number    INT PRIMARY KEY,
    train_name      VARCHAR(50) NOT NULL,
    source          VARCHAR(50) NOT NULL,
    destination     VARCHAR(50) NOT NULL,
    total_seats     INT NOT NULL,
    available_seats INT NOT NULL
);

-- One row = one passenger
CREATE TABLE IF NOT EXISTS passengers (
    passenger_id INT AUTO_INCREMENT PRIMARY KEY,
    name         VARCHAR(50) NOT NULL,
    age          INT NOT NULL,
    gender       VARCHAR(10) NOT NULL,
    phone        VARCHAR(15) NOT NULL
);

-- One row = one booking (status: CONFIRMED, WAITING or CANCELLED)
CREATE TABLE IF NOT EXISTS tickets (
    ticket_id    INT AUTO_INCREMENT PRIMARY KEY,
    passenger_id INT NOT NULL,
    train_number INT NOT NULL,
    seat_number  INT NULL,
    status       VARCHAR(15) NOT NULL,
    FOREIGN KEY (passenger_id) REFERENCES passengers(passenger_id),
    FOREIGN KEY (train_number) REFERENCES trains(train_number)
);