cmake_minimum_required(VERSION 3.3)
project(CPP-RCPSP-OC)

if(APPLE)
include_directories(/usr/local/opt/boost/include /opt/localsolver_6_0/include ./CPP-RCPSP-OC-Test/googletest/include /Library/gurobi652/mac64/include)
link_directories(/usr/local/opt/boost/lib /opt/localsolver_6_0/bin ./CPP-RCPSP-OC-Test/googletest /Library/gurobi652/mac64/lib)
elseif(UNIX)
include_directories(~/localsolver6/include ./CPP-RCPSP-OC-Test/googletest/include)
link_directories(~/localsolver6/bin ./CPP-RCPSP-OC-Test/googletest)
endif()

if(APPLE)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++14")
elseif(UNIX)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=gnu++0x")
endif()

set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -llocalsolver -lboost_system -lboost_filesystem -lgtest -lgurobi_c++ -lgurobi65")

set(SOURCE_FILES_COMMON Utils.h Utils.cpp Project.cpp Project.h ProjectWithOvertime.cpp ProjectWithOvertime.h Visualization.cpp Visualization.h GeneticAlgorithms/GeneticAlgorithm.h GeneticAlgorithms/TimeWindow.cpp GeneticAlgorithms/TimeWindow.h GeneticAlgorithms/OvertimeBound.cpp GeneticAlgorithms/OvertimeBound.h GeneticAlgorithms/FixedDeadline.cpp GeneticAlgorithms/FixedDeadline.h GeneticAlgorithms/Sampling.cpp GeneticAlgorithms/Sampling.h Stopwatch.cpp Stopwatch.h Matrix.h GeneticAlgorithms/Runners.cpp GeneticAlgorithms/Runners.h BranchAndBound.cpp BranchAndBound.h GeneticAlgorithms/Representations.cpp GeneticAlgorithms/Representations.h LSModels/ListModel.cpp LSModels/ListModel.h LSModels/NaiveModels.cpp LSModels/NaiveModels.h LSModels/OvertimeBoundModels.h LSModels/OvertimeBoundModels.cpp LSModels/TimeWindowModels.cpp LSModels/TimeWindowModels.h LSModels/FixedDeadlineModels.h LSModels/FixedDeadlineModels.cpp GurobiSolver.h GurobiSolver.cpp)

set(SOURCE_FILES ${SOURCE_FILES_COMMON} main.cpp)
add_executable(CPP-RCPSP-OC ${SOURCE_FILES})

set(TEST_SOURCE_FILES ${SOURCE_FILES_COMMON} CPP-RCPSP-OC-Test/testmain.cpp CPP-RCPSP-OC-Test/ProjectTest.cpp CPP-RCPSP-OC-Test/ProjectTest.h CPP-RCPSP-OC-Test/TestHelpers.cpp CPP-RCPSP-OC-Test/TestHelpers.h CPP-RCPSP-OC-Test/UtilsTest.cpp CPP-RCPSP-OC-Test/SamplingTest.cpp CPP-RCPSP-OC-Test/ProjectWithOvertimeTest.cpp CPP-RCPSP-OC-Test/ProjectWithOvertimeTest.h CPP-RCPSP-OC-Test/RepresentationsTest.cpp CPP-RCPSP-OC-Test/RepresentationsTest.h)
add_executable(CPP-RCPSP-OC-Tests ${TEST_SOURCE_FILES})
