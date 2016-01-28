cmake_minimum_required(VERSION 3.3)
project(CPP-RCPSP-OC)

include_directories(/opt/localsolver_6_0/include ./CPP-RCPSP-OC-Test/googletest/include)
link_directories(/opt/localsolver_6_0/bin ./CPP-RCPSP-OC-Test/googletest)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++14")
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -llocalsolver -lpodofo -lboost_system -lboost_filesystem -lgtest")

set(SOURCE_FILES_COMMON Utils.h Utils.cpp Project.cpp Project.h ProjectWithOvertime.cpp ProjectWithOvertime.h LSSolver.cpp LSSolver.h Visualization.cpp Visualization.h GeneticAlgorithms/GeneticAlgorithm.h GeneticAlgorithms/TimeWindow.cpp GeneticAlgorithms/TimeWindow.h GeneticAlgorithms/OvertimeBound.cpp GeneticAlgorithms/OvertimeBound.h GeneticAlgorithms/FixedDeadline.cpp GeneticAlgorithms/FixedDeadline.h GeneticAlgorithms/Sampling.cpp GeneticAlgorithms/Sampling.h Stopwatch.cpp Stopwatch.h Matrix.h GeneticAlgorithms/Runners.cpp GeneticAlgorithms/Runners.h BranchAndBound.cpp BranchAndBound.h)

set(SOURCE_FILES ${SOURCE_FILES_COMMON} main.cpp)
add_executable(CPP-RCPSP-OC ${SOURCE_FILES})

set(TEST_SOURCE_FILES ${SOURCE_FILES_COMMON} CPP-RCPSP-OC-Test/testmain.cpp)
add_executable(CPP-RCPSP-OC-Tests ${TEST_SOURCE_FILES})
