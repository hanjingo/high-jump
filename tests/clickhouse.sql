CREATE DATABASE libcpp;
USE libcpp;

CREATE TABLE test (
    `id` UInt64 COMMENT '',
    `age` Int8 COMMENT '',
    `salary` Decimal(6, 2) COMMENT '1234.56',
    `name` String COMMENT 'xx',
    `time` DateTime COMMENT '2019-12-16 20:50:10, for newer version use DateTime64(...)'
) ENGINE =MergeTree() 
  PRIMARY KEY (id) 
  PARTITION BY toYYYYMMDD(time) 
  ORDER BY (id, time); -- Primary key must be a prefix of the sorting key

INSERT INTO test values 
    (1, 30, 1000.50, 'harry', '2019-12-16 20:50:10'), 
    (2, 30, 1000.50, 'li', '2019-12-16 20:50:10');