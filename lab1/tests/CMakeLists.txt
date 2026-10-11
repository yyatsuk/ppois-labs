include(FetchContent)

FetchContent_Declare(
    Catch2
    GIT_REPOSITORY https://github.com/catchorg/Catch2.git
    GIT_TAG v3.5.2
)
FetchContent_MakeAvailable(Catch2)

add_executable(test_post test_post.cpp)
target_link_libraries(test_post PRIVATE post_machine Catch2::Catch2WithMain)

add_executable(test_turing test_turing.cpp)
target_link_libraries(test_turing PRIVATE turing_machine Catch2::Catch2WithMain)

list(APPEND CMAKE_MODULE_PATH ${catch2_SOURCE_DIR}/extras)
include(Catch)
catch_discover_tests(test_post)
catch_discover_tests(test_turing)