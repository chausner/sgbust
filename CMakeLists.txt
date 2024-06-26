cmake_minimum_required(VERSION 3.18)
project(sgbust)

add_executable(sgbust
    src/cli/commands.cpp
    src/cli/parser.cpp
    src/cli/utils.cpp
    src/core/CompactGrid.cpp
    src/core/Grid.cpp
    src/core/MemoryUsage.cpp
    src/core/Polynom.cpp
    src/core/scorings/GreedyScoring.cpp
    src/core/scorings/NumBlocksNotInGroupsScoring.cpp
    src/core/scorings/PotentialScoring.cpp
    src/core/Solution.cpp
    src/core/Solver.cpp
    src/main.cpp
)

target_compile_features(sgbust PRIVATE cxx_std_20)

if (CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
    target_compile_options(sgbust PRIVATE "/Zc:__cplusplus")
endif()

target_include_directories(sgbust PRIVATE include)

if (NOT CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
    find_package(TBB CONFIG REQUIRED)
    target_link_libraries(sgbust PRIVATE TBB::tbb)
endif()

find_package(CLI11 CONFIG REQUIRED)
target_link_libraries(sgbust PRIVATE CLI11::CLI11)

find_package(mdspan CONFIG REQUIRED)
target_link_libraries(sgbust PRIVATE std::mdspan)

find_package(mimalloc CONFIG REQUIRED)
target_link_libraries(sgbust PRIVATE mimalloc)

find_path(PARALLEL_HASHMAP_INCLUDE_DIRS "parallel_hashmap/phmap.h" REQUIRED)
target_include_directories(sgbust PRIVATE ${PARALLEL_HASHMAP_INCLUDE_DIRS})

find_path(WYHASH_INCLUDE_DIRS "wyhash.h" REQUIRED)
target_include_directories(sgbust PRIVATE ${WYHASH_INCLUDE_DIRS})
