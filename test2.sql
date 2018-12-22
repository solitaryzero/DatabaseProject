CREATE DATABASE orderDB_small;

USE orderDB_small;

CREATE TABLE customer(
	id int(10) NOT NULL,
	name varchar(25) NOT NULL,
	gender char(1) NOT NULL,
	PRIMARY KEY (id)
);

INSERT INTO customer VALUES (1, "hello", "M");
INSERT INTO customer VALUES (2, "world", "F");
SELECT * FROM customer;
DELETE FROM customer WHERE id != 2;
SELECT * FROM customer;
INSERT INTO customer VALUES (3, "world", "M");
INSERT INTO customer VALUES (4, "haha", "F");
UPDATE customer SET name = 'test' WHERE name = "world";
SELECT * FROM customer;
SELECT name FROM customer WHERE id != 3;