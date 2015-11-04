cmake_minimum_required(VERSION 3.3)
project(CPP-RCPSP-OC)

include_directories(/opt/localsolver_6_0/include)
link_directories(/opt/localsolver_6_0/bin)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++14")
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -llocalsolver -lpodofo")

set(SOURCE_FILES main.cpp Utils.h Utils.cpp Project.cpp Project.h ProjectWithOvertime.cpp ProjectWithOvertime.h LSSolver.cpp LSSolver.h ScheduleVisualizer.cpp ScheduleVisualizer.h GeneticAlgorithms/GeneticAlgorithm.h GeneticAlgorithms/TimeWindow.cpp GeneticAlgorithms/TimeWindow.h GeneticAlgorithms/OvertimeBound.cpp GeneticAlgorithms/OvertimeBound.h GeneticAlgorithms/FixedDeadline.cpp GeneticAlgorithms/FixedDeadline.h GeneticAlgorithms/Sampling.cpp GeneticAlgorithms/Sampling.h)
add_executable(CPP-RCPSP-OC ${SOURCE_FILES})