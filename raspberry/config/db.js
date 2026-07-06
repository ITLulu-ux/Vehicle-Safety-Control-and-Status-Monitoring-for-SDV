const mysql = require('mysql2/promise');

const pool = mysql.createPool({
    host: '172.30.1.12',
    port: 3306,
    user: 'lulu_user',              // 또는 네가 만든 계정
    password: 'password123!',
    database: 'raspberry_server',

    waitForConnections: true,
    connectionLimit: 10,
    queueLimit: 0
});

module.exports = pool;