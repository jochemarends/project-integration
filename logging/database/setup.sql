CREATE DATABASE IF NOT EXISTS PI; -- (Project Integration)

USE PI;

CREATE TABLE IF NOT EXISTS Patient (
    id   INT PRIMARY KEY AUTO_INCREMENT,
    name VARCHAR(256)
);

CREATE TABLE IF NOT EXISTS Log (
    id         INT PRIMARY KEY AUTO_INCREMENT,
    kind       VARCHAR(32),
    message    VARCHAR(256),
    patient_id INT,
    FOREIGN KEY (patient_id) REFERENCES Patient (id)
);

