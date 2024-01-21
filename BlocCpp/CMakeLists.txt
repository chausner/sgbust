cmake_minimum_required(VERSION 3.12)
project(BlocCpp)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O3 -march=native")

set(SOURCE_FILES
    BlocSolver.cpp
    BlockGrid.cpp
    Scoring.cpp
    main.cpp
    utils.cpp
)

find_package(TBB REQUIRED)

add_executable(BlocCpp ${SOURCE_FILES})

target_link_libraries(BlocCpp PRIVATE TBB::tbb)
