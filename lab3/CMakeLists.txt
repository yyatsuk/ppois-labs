cmake_minimum_required(VERSION 3.14)
project(weather LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if(MSVC)
    add_compile_options(/utf-8)
endif()

add_library(weather_lib
    src/measurements.cpp
    src/stations.cpp
    src/forecasts.cpp
    src/alerts.cpp
    src/service.cpp
)
target_include_directories(weather_lib PUBLIC include)

add_executable(weather_cli cli/main.cpp)
target_link_libraries(weather_cli PRIVATE weather_lib)

if(EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/tests/CMakeLists.txt)
    enable_testing()
    add_subdirectory(tests)
endif()