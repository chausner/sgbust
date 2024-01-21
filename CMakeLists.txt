cmake_minimum_required(VERSION 3.12)
project(BlocCpp)

set(CMAKE_CXX_STANDARD 20)

set(SOURCE_FILES
    BlocCpp/BlockGrid.cpp
    BlocCpp/BlocSolver.cpp
    BlocCpp/CompactBlockGrid.cpp
    BlocCpp/main.cpp
    BlocCpp/MemoryUsage.cpp
    BlocCpp/Scoring.cpp
    BlocCpp/utils.cpp
)

add_executable(BlocCpp ${SOURCE_FILES})

target_include_directories(BlocCpp PRIVATE BlocCpp)

if (NOT CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
    find_package(TBB REQUIRED)
    target_link_libraries(BlocCpp PRIVATE TBB::tbb)
endif()

find_package(CLI11 CONFIG REQUIRED)
target_link_libraries(BlocCpp PRIVATE CLI11::CLI11)

find_path(PARALLEL_HASHMAP_INCLUDE_DIRS "parallel_hashmap/phmap.h")
target_include_directories(BlocCpp PRIVATE ${PARALLEL_HASHMAP_INCLUDE_DIRS})

find_path(WYHASH_INCLUDE_DIRS "wyhash.h")
target_include_directories(BlocCpp PRIVATE ${WYHASH_INCLUDE_DIRS})
