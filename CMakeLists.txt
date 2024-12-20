cmake_minimum_required(VERSION 3.12)

project(diy_page_cache)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Option to enable DEBUG_CACHE
option(DEBUG_CACHE "Enable cache debugging" OFF)

# Define DEBUG_CACHE if the option is enabled
if(DEBUG_CACHE)
    add_definitions(-DDEBUG_CACHE)
endif()

# Include directories for all targets
include_directories(include)

# Find GTest
find_package(GTest REQUIRED)
include_directories(${GTEST_INCLUDE_DIRS})

# Library target
add_library(diy_cache
    src/diy_cache/cache.cpp
    src/diy_cache/lab2_api.cpp
    src/diy_cache/page.cpp
    src/diy_cache/storage.cpp
)
target_include_directories(diy_cache PUBLIC include) # Make headers available to users

# Executable targets
add_executable(app src/app/app.cpp)
target_link_libraries(app diy_cache)

add_executable(optimize src/app/optimize.cpp src/benchmarks/search_benchmark.cpp src/benchmarks/linreg_benchmark.cpp)
target_link_libraries(optimize diy_cache)

add_executable(search_benchmark src/benchmarks/search_ex.cpp src/benchmarks/search_benchmark.cpp)
target_link_libraries(search_benchmark diy_cache)

add_executable(linreg_benchmark src/benchmarks/linreg_ex.cpp src/benchmarks/linreg_benchmark.cpp)
target_link_libraries(linreg_benchmark diy_cache)

add_executable(pure_search_benchmark src/benchmarks/pure_search_benchmark.cpp)
add_executable(pure_linreg_benchmark src/benchmarks/pure_linreg_benchmark.cpp)

# Test target
enable_testing()
add_executable(test_storage test/test_storage.cpp)
target_link_libraries(test_storage diy_cache GTest::GTest GTest::Main)
add_test(NAME TestStorage COMMAND test_storage)

# Install rules (optional)
install(TARGETS diy_cache DESTINATION lib)
install(FILES include/diy_cache/*.h DESTINATION include/diy_cache)

# Adjust compiler flags for more lenient warnings
target_compile_options(diy_cache PUBLIC
    -Wall -Wextra # Enable common warnings
    -Wno-unused-variable # Ignore unused variable warnings
)
