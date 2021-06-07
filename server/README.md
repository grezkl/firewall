A simple linux server based on Asio

## Pre-install
- library: asio, otl 
- database: mysql/mariadb

## 1. import rules(text) to database
```
$ cd src/import_rules

$ ./import_rules.sh pathList.txt
```
## 2. build and start up server
```
$ mkdir build && cd build && cmake .. && make VERBOSE=1

$ ./srv <host> <port>
```

## 3. test
telnet
