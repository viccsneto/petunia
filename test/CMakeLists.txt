cmake_minimum_required(VERSION 3.0)

set(THIS PetuniaTests)

set(SOURCES
    PetuniaTests.cpp
  )

add_executable(${THIS} ${SOURCES})

target_link_libraries(${THIS}
  PUBLIC
    gtest_main
    petunia
)

add_test(
  NAME ${THIS}
  COMMAND ${THIS}
)

