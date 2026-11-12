add_library(post_machine
    src/Tape.cpp
    src/Program.cpp
    src/Machine.cpp
)

target_include_directories(post_machine PUBLIC include)