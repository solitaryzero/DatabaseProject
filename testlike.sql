CREATE DATABASE test_like;
USE test_like;

CREATE TABLE test (id INT(10), name VARCHAR(20));

INSERT INTO test VALUES (1, 'Alice'), (2, 'Bob');

SELECT * FROM test WHERE id = 1;
SELECT * FROM test WHERE name LIKE '%Ali__';
SELECT * FROM test WHERE name LIKE 'A_%';
SELECT * FROM test WHERE name LIKE 'B%_';
SELECT * FROM test WHERE name LIKE 'B_';