include(FetchContent)
FetchContent_Declare(
    Catch2
    GIT_REPOSITORY https://github.com/catchorg/Catch2.git
    GIT_TAG        v3.5.2
)
FetchContent_MakeAvailable(Catch2)

add_executable(weather_tests test_weather.cpp)
target_link_libraries(weather_tests PRIVATE weather_lib Catch2::Catch2WithMain)

include(Catch)
catch_discover_tests(weather_tests)