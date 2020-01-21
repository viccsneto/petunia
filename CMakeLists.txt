cmake_minimum_required (VERSION 3.5)
project (petunia)
set (CMAKE_CXX_STANDARD 11)
set_property(GLOBAL PROPERTY USE_FOLDERS ON)

add_subdirectory(third_party)

target_compile_definitions(nanomsg
PUBLIC
  -DNN_STATIC_LIB
) 

set(PETUNIA_SRCS
  src/message.cpp
  src/petunia.cpp
  src/ipc_medium.cpp
  src/ipc_medium_inprocess.cpp
  src/ipc_medium_nanomsg.cpp
  src/ipc_medium_zmq.cpp
  src/osutils.cpp
  include/petunia/ipc_medium.h
  include/petunia/ipc_medium_default.h
  include/petunia/ipc_medium_inprocess.h
  include/petunia/ipc_medium_nanomsg.h
  include/petunia/ipc_medium_zmq.h
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
    ${CMAKE_CURRENT_SOURCE_DIR}/include
  PRIVATE
    src/
)



target_link_libraries(petunia PRIVATE
  nanomsg
  zmqlib
)

add_subdirectory(test)


