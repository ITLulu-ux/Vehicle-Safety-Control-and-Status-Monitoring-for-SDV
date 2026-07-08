CREATE DATABASE IF NOT EXISTS raspberry_server;
USE raspberry_server;

/* ===========================
   DTC Log
=========================== */

CREATE TABLE dtc_log (

    id INT AUTO_INCREMENT PRIMARY KEY,

    timestamp DATETIME DEFAULT CURRENT_TIMESTAMP,

    ecu_id TINYINT NOT NULL,

    dtc_code VARCHAR(10) NOT NULL,

    description VARCHAR(100),

    status ENUM('ACTIVE','CLEARED')
        DEFAULT 'ACTIVE'

);

/* ===========================
   Heartbeat
=========================== */

CREATE TABLE heartbeat_log (

    ecu_id TINYINT PRIMARY KEY,

    last_received DATETIME
        DEFAULT CURRENT_TIMESTAMP,

    status ENUM('CONNECTED','DISCONNECTED')
        DEFAULT 'CONNECTED'

);

/* ===========================
   Sensor Log
=========================== */

CREATE TABLE sensor_log (

    id INT AUTO_INCREMENT PRIMARY KEY,

    timestamp DATETIME
        DEFAULT CURRENT_TIMESTAMP,

    can_id INT NOT NULL,

    temperature TINYINT NULL,

    humidity TINYINT NULL,

    lux SMALLINT NULL,

    speed SMALLINT NULL,

    distance INT NULL

);

/* ===========================
   Gateway Command Log
=========================== */

CREATE TABLE gateway_command_log (

    id INT AUTO_INCREMENT PRIMARY KEY,

    timestamp DATETIME
        DEFAULT CURRENT_TIMESTAMP,

    command_id TINYINT,

    target_ecu TINYINT,

    parameter VARCHAR(50),

    value VARCHAR(50),

    version VARCHAR(20)

);