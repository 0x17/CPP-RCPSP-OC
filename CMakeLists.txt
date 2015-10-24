cmake_minimum_required(VERSION 3.3)
project(CPP-RCPSP-OC)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")

set(SOURCE_FILES main.cpp Utils.h Utils.cpp Project.cpp Project.h ProjectWithOvertime.cpp ProjectWithOvertime.h)
add_executable(CPP-RCPSP-OC ${SOURCE_FILES})