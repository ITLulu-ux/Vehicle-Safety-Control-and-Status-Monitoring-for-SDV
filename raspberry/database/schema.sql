CREATE DATABASE IF NOT EXISTS raspberry_server;
USE raspberry_server;

CREATE TABLE dtc_log (
    id INT AUTO_INCREMENT PRIMARY KEY,
    timestamp DATETIME NOT NULL DEFAULT CURRENT_TIMESTAMP,
    dtc_code VARCHAR(10) NOT NULL,
    description VARCHAR(100) NOT NULL,
    ecu VARCHAR(10) NOT NULL,
    status VARCHAR(10) NOT NULL DEFAULT 'ACTIVE',
    INDEX idx_dtc_code (dtc_code),
    INDEX idx_ecu (ecu),
    INDEX idx_timestamp (timestamp)
);

CREATE TABLE heartbeat_log (
    id INT AUTO_INCREMENT PRIMARY KEY,
    ecu_name VARCHAR(20) NOT NULL UNIQUE,
    last_received DATETIME NOT NULL DEFAULT CURRENT_TIMESTAMP,
    status VARCHAR(15) NOT NULL DEFAULT 'CONNECTED',

    INDEX idx_ecu_name (ecu_name)
);