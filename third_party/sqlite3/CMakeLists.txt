# CMake entry point
cmake_minimum_required (VERSION 3.14)
project (sqlite3 CXX)

add_library(sqlite3 STATIC
  include/sqlite3/sqlite3.h
  include/sqlite3/CppSQLite3.h	
  src/sqlite3.c
  src/CppSQLite3.cpp	

)

target_include_directories(sqlite3
  PUBLIC
    include
)
