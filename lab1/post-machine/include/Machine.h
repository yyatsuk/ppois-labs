#pragma once

#include "Tape.h"
#include "Program.h"

class Machine {
private:
    Tape tape;
    Program program;

public:
    Machine() = default;
    // Здесь позже появятся методы step() и run()
};