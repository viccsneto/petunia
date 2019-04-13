# CMake entry point
cmake_minimum_required (VERSION 3.0)
project (petunia)

set(PETUNIA_SRCS
	src/sqlite3/sqlite3.c
  src/sqlite3/CppSQLite3.cpp	
  src/sqlite3/sqlite3.h
  src/sqlite3/CppSQLite3.h	
  src/message.cpp
  src/message_handler.cpp
  src/petunia.cpp
  src/osutils.cpp
  include/petunia/message.h
  include/petunia/message_handler.h
  include/petunia/osutils.h
  include/petunia/petunia.h  
)

add_library(petunia STATIC
    ${PETUNIA_SRCS}
)

target_include_directories(petunia
  PUBLIC
    include
  PRIVATE
    src/
)
