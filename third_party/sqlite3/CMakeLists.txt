# CMake entry point
cmake_minimum_required (VERSION 3.5)
project (sqlite3lib CXX)

add_library(sqlite3lib STATIC
  include/sqlite3/sqlite3.h
  include/sqlite3/CppSQLite3.h	
  src/sqlite3.c
  src/CppSQLite3.cpp	

)

target_include_directories(sqlite3lib
  PUBLIC
    include
)
