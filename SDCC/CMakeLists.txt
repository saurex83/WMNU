cmake_minimum_required(VERSION 2.8) 
project(utests)	

file( GLOB SRCS "core/*.c" "core/*.h" "tests/*.c" "tests/*.h")
#set(SOURCE_EXE tests/utest.c)
include_directories(core)
include_directories(tests)
message ("Common ${SRCS}")
add_executable(main ${SRCS})
