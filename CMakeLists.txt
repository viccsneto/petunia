cmake_minimum_required (VERSION 3.0)
project (petunia)

set (CMAKE_CXX_STANDARD 11)

set(PETUNIA_SRCS
	src/sqlite3/sqlite3.c  
  src/sqlite3/sqlite3.h
  src/CppSQLite3/CppSQLite3.cpp	
  src/CppSQLite3/CppSQLite3.h	
  src/message.cpp
  src/petunia.cpp
  src/ipc_medium.cpp
  src/ipc_medium_default.cpp
  src/osutils.cpp
  include/petunia/ipc_medium.h
  include/petunia/ipc_medium_default.h
  include/petunia/message.h
  include/petunia/osutils.h
  include/petunia/petunia.h
  include/petunia/types.h
)


enable_testing()

add_subdirectory(googletest)

add_library(petunia STATIC
    ${PETUNIA_SRCS}
)

target_include_directories(petunia
  PUBLIC
    include
  PRIVATE
    src/
)

add_subdirectory(test)
