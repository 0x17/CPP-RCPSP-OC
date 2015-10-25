cmake_minimum_required(VERSION 3.3)
project(CPP-RCPSP-OC)

include_directories(/opt/localsolver_6_0/include)
link_directories(/opt/localsolver_6_0/bin)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -llocalsolver -lpodofo")

set(SOURCE_FILES main.cpp Utils.h Utils.cpp Project.cpp Project.h ProjectWithOvertime.cpp ProjectWithOvertime.h LSSolver.cpp LSSolver.h ScheduleVisualizer.cpp ScheduleVisualizer.h)
add_executable(CPP-RCPSP-OC ${SOURCE_FILES})