CREATE DATABASE orderDB_small;

USE orderDB_small;

CREATE TABLE customer(
	id int(10) NOT NULL,
	name varchar(25) NOT NULL,
	gender char(1) NOT NULL,
	PRIMARY KEY (id)
);

CREATE TABLE book (
  id int(10) NOT NULL,
  title varchar(100) NOT NULL,
  authors varchar(200),
  publisher varchar(100),
  copies int(10),
  PRIMARY KEY (id)
);

CREATE TABLE website(
	id int(10) NOT NULL,
	name varchar(25) NOT NULL,
	url varchar(100),
	PRIMARY KEY (id)
);
