cmake_minimum_required(VERSION 3.5)

project(srv)

set(CMAKE_CXX_STANDARD 11)
SET(CMAKE_BUILD_TYPE "Debug")

add_executable(srv
    src/main.cc
    src/tcpserver.cc
    src/tcpconnection.cc
    src/sqlconnection.cc
    )

target_link_libraries(srv
    PUBLIC
    pthread
    odbc
    )
