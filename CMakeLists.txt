cmake_minimum_required(VERSION 3.9)
project(CPP-RCPSP-OC)

set(CMAKE_CXX_STANDARD 14)

find_package(Boost COMPONENTS system filesystem REQUIRED)
find_package(GTest REQUIRED)

if(APPLE)
    set(GUROBI_INCLUDE_DIRS /Library/gurobi752/mac64/include)
    set(LOCALSOLVER_INCLUDE_DIRS /opt/localsolver_8_0/include)
    set(GUROBI_LIB_DIRS /Library/gurobi752/mac64/lib)
    set(LOCALSOLVER_LIB_DIRS /opt/localsolver_8_0/bin)
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -llocalsolver -lgurobi_c++ -lgurobi75")
else()
    set(GUROBI_INCLUDE_DIRS ~/gurobi752/linux64/include)
    set(LOCALSOLVER_INCLUDE_DIRS /opt/localsolver_8_0/include ~/localsolver_8_0/include)
    set(GUROBI_LIB_DIRS ~/gurobi752/linux64/lib)
    set(LOCALSOLVER_LIB_DIRS /opt/localsolver_8_0/bin ~/localsolver_8_0/bin)
    #set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++1y")
    add_definitions(-DDISABLE_GUROBI)
    add_definitions(-DDISABLE_LOCALSOLVER)
    #set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -static-libgcc -static-libstdc++")
endif()

include_directories(${Boost_INCLUDE_DIRS} ${GTEST_INCLUDE_DIRS} ${GUROBI_INCLUDE_DIRS} ${LOCALSOLVER_INCLUDE_DIRS})
link_directories(${GUROBI_LIB_DIRS} ${LOCALSOLVER_LIB_DIRS})

set(SOURCE_FILES_COMMON Utils.h Utils.cpp Project.cpp Project.h ProjectWithOvertime.cpp ProjectWithOvertime.h GeneticAlgorithms/GeneticAlgorithm.h GeneticAlgorithms/GeneticAlgorithm.cpp GeneticAlgorithms/TimeWindow.cpp GeneticAlgorithms/TimeWindow.h GeneticAlgorithms/OvertimeBound.cpp GeneticAlgorithms/OvertimeBound.h GeneticAlgorithms/FixedDeadline.cpp GeneticAlgorithms/FixedDeadline.h GeneticAlgorithms/Sampling.cpp GeneticAlgorithms/Sampling.h Stopwatch.cpp Stopwatch.h Matrix.h Runners.cpp Runners.h BranchAndBound.cpp BranchAndBound.h GeneticAlgorithms/Representations.cpp GeneticAlgorithms/Representations.h LSModels/ListModel.cpp LSModels/ListModel.h LSModels/PartitionModels.cpp LSModels/PartitionModels.h LSModels/NaiveModels.cpp LSModels/NaiveModels.h LSModels/OvertimeBoundModels.h LSModels/OvertimeBoundModels.cpp LSModels/TimeWindowModels.cpp LSModels/TimeWindowModels.h LSModels/FixedDeadlineModels.h LSModels/FixedDeadlineModels.cpp GurobiSolver.h GurobiSolver.cpp Libraries/json11.hpp Libraries/json11.cpp BasicSolverParameters.cpp BasicSolverParameters.h Logger.cpp Logger.h JsonUtils.cpp JsonUtils.h GeneticAlgorithms/Partition.cpp GeneticAlgorithms/Partition.h LSModels/SimpleModel.cpp LSModels/SimpleModel.h SensitivityAnalysis.cpp SensitivityAnalysis.h)

set(SOURCE_FILES ${SOURCE_FILES_COMMON} main.cpp)
add_executable(CPP-RCPSP-OC ${SOURCE_FILES})
target_link_libraries(CPP-RCPSP-OC -llocalsolver -lgurobi_c++ -lgurobi75 -lpthread ${Boost_LIBRARIES})

enable_testing()
set(TEST_SOURCE_FILES ${SOURCE_FILES_COMMON} CPP-RCPSP-OC-Test/testmain.cpp CPP-RCPSP-OC-Test/ProjectTest.cpp CPP-RCPSP-OC-Test/ProjectTest.h CPP-RCPSP-OC-Test/TestHelpers.cpp CPP-RCPSP-OC-Test/TestHelpers.h CPP-RCPSP-OC-Test/UtilsTest.cpp CPP-RCPSP-OC-Test/SamplingTest.cpp CPP-RCPSP-OC-Test/ProjectWithOvertimeTest.cpp CPP-RCPSP-OC-Test/ProjectWithOvertimeTest.h CPP-RCPSP-OC-Test/RepresentationsTest.cpp CPP-RCPSP-OC-Test/RepresentationsTest.h CPP-RCPSP-OC-Test/PaperConsistencyTest.h CPP-RCPSP-OC-Test/PaperConsistencyTest.cpp CPP-RCPSP-OC-Test/MatrixTest.h CPP-RCPSP-OC-Test/MatrixTest.cpp CPP-RCPSP-OC-Test/ParticleSwarmTest.cpp CPP-RCPSP-OC-Test/SerializationTest.cpp CPP-RCPSP-OC-Test/JsonUtilsTest.cpp)
add_executable(CPP-RCPSP-OC-Tests ${TEST_SOURCE_FILES})
target_link_libraries(CPP-RCPSP-OC-Tests -llocalsolver -lgurobi_c++ -lgurobi75 -lpthread ${Boost_LIBRARIES} ${GTEST_BOTH_LIBRARIES})