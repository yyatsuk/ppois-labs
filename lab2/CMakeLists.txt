cmake_minimum_required(VERSION 3.14)
project(theatre LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if(MSVC)
    add_compile_options(/utf-8)
endif()

add_library(theatre_lib
    src/people.cpp
    src/repertoire.cpp
    src/venue.cpp
    src/sales.cpp
    src/backstage_finance.cpp
)
target_include_directories(theatre_lib PUBLIC include)

add_executable(theatre_cli cli/main.cpp)
target_link_libraries(theatre_cli PRIVATE theatre_lib)

if(EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/tests/CMakeLists.txt)
    enable_testing()
    add_subdirectory(tests)
endif()