add_library(turing_machine STATIC
    src/TuringTape.cpp
    src/TuringProgram.cpp
    src/TuringMachine.cpp
)

target_include_directories(turing_machine PUBLIC include)