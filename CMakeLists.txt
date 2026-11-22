cmake_minimum_required(VERSION 4.4)
project(sgbust)

add_executable(sgbust)

target_sources(sgbust PRIVATE
    FILE_SET SOURCES
        BASE_DIRS src
        FILES
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
    FILE_SET HEADERS
        BASE_DIRS include
        FILES
            include/cli/commands.h
            include/cli/parser.h
            include/cli/utils.h
            include/core/CompactGrid.h
            include/core/Grid.h
            include/core/MemoryUsage.h
            include/core/Polynom.h
            include/core/Scoring.h
            include/core/scorings/GreedyScoring.h
            include/core/scorings/NumBlocksNotInGroupsScoring.h
            include/core/scorings/PotentialScoring.h
            include/core/Solution.h
            include/core/Solver.h
)

target_compile_features(sgbust PRIVATE cxx_std_23)

if (CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
    target_compile_options(sgbust PRIVATE "/Zc:__cplusplus" "$<$<OR:$<CONFIG:Release>,$<CONFIG:RelWithDebInfo>>:/Ob3>")
endif()

if (NOT CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
    find_package(TBB CONFIG REQUIRED)
    target_link_libraries(sgbust PRIVATE TBB::tbb)
endif()

find_package(CLI11 CONFIG REQUIRED)
target_link_libraries(sgbust PRIVATE CLI11::CLI11)

find_package(mimalloc CONFIG REQUIRED)
target_link_libraries(sgbust PRIVATE mimalloc)

find_package(nlohmann_json CONFIG REQUIRED)
target_link_libraries(sgbust PRIVATE nlohmann_json::nlohmann_json)

find_path(PARALLEL_HASHMAP_INCLUDE_DIRS "parallel_hashmap/phmap.h" REQUIRED)
target_include_directories(sgbust PRIVATE ${PARALLEL_HASHMAP_INCLUDE_DIRS})

find_path(WYHASH_INCLUDE_DIRS "wyhash.h" REQUIRED)
target_include_directories(sgbust PRIVATE ${WYHASH_INCLUDE_DIRS})
