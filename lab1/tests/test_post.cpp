#include <catch2/catch_test_macros.hpp>
#include <sstream>
#include "Machine.h"
#include "Program.h"
#include "Tape.h"

// ============ Tape (5 тестов) ============

TEST_CASE("Post Tape: starts empty at position 0", "[post][tape]") {
    Tape tape;
    REQUIRE(tape.position() == 0);
    REQUIRE_FALSE(tape.isMarked());
}

TEST_CASE("Post Tape: mark and erase", "[post][tape]") {
    Tape tape;
    tape.mark();
    REQUIRE(tape.isMarked());
    tape.erase();
    REQUIRE_FALSE(tape.isMarked());
}

TEST_CASE("Post Tape: move right extends tape", "[post][tape]") {
    Tape tape;
    ++tape;
    REQUIRE(tape.position() == 1);
    ++tape;
    REQUIRE(tape.position() == 2);
}

TEST_CASE("Post Tape: move left at zero stays at zero", "[post][tape]") {
    Tape tape;
    --tape;
    REQUIRE(tape.position() == 0);
    REQUIRE_FALSE(tape.isMarked());
}

TEST_CASE("Post Tape: equality and round-trip", "[post][tape]") {
    Tape a;
    a.mark();
    ++a;

    Tape b;
    b.mark();
    ++b;
    REQUIRE(a == b);

    std::stringstream stream;
    stream << a;
    Tape restored;
    stream >> restored;
    REQUIRE(a == restored);
}

// ============ Program + Command (5 тестов) ============

TEST_CASE("Post Program: empty by default", "[post][program]") {
    Program program;
    REQUIRE(program.size() == 0);
    REQUIRE(program.firstLine() == Command::kNoTarget);
}

TEST_CASE("Post Program: add rule", "[post][program]") {
    Program program;
    Command mark(Command::kMark, 2);
    REQUIRE(program.addRule(1, mark));
    REQUIRE(program.size() == 1);
    REQUIRE(program.findRule(1) != nullptr);
}

TEST_CASE("Post Program: cannot add duplicate", "[post][program]") {
    Program program;
    Command mark(Command::kMark, 2);
    REQUIRE(program.addRule(1, mark));
    REQUIRE_FALSE(program.addRule(1, mark));
    REQUIRE(program.size() == 1);
}

TEST_CASE("Post Program: cannot add rule with number below 1", "[post][program]") {
    Program program;
    Command stop(Command::kStop);
    REQUIRE_FALSE(program.addRule(0, stop));
    REQUIRE_FALSE(program.addRule(-5, stop));
}

TEST_CASE("Post Program: remove and round-trip", "[post][program]") {
    Program program;
    program.addRule(1, Command(Command::kMark, 2));
    program.addRule(2, Command(Command::kStop));

    std::stringstream stream;
    stream << program;
    Program restored;
    stream >> restored;
    REQUIRE(program == restored);

    REQUIRE(program.removeRule(1));
    REQUIRE_FALSE(program.removeRule(1));
    REQUIRE(program.size() == 1);
}

// ============ Machine (5 тестов) ============

TEST_CASE("Post Machine: starts running with no rules", "[post][machine]") {
    Machine machine;
    REQUIRE(machine.status() == MachineStatus::Running);
    REQUIRE(machine.currentLine() == Command::kNoTarget);
}

TEST_CASE("Post Machine: step marks cell", "[post][machine]") {
    Machine machine;
    machine.program().addRule(1, Command(Command::kMark, 2));
    machine.reset();

    REQUIRE(machine.step());
    REQUIRE(machine.tape().isMarked());
    REQUIRE(machine.currentLine() == 2);
}

TEST_CASE("Post Machine: step moves carriage right", "[post][machine]") {
    Machine machine;
    machine.program().addRule(1, Command(Command::kRight, 1));
    machine.reset();

    machine.step();
    REQUIRE(machine.tape().position() == 1);
}

TEST_CASE("Post Machine: stop command halts", "[post][machine]") {
    Machine machine;
    machine.program().addRule(1, Command(Command::kStop));
    machine.reset();

    machine.step();
    REQUIRE(machine.status() == MachineStatus::Stopped);
    REQUIRE_FALSE(machine.step());
}

TEST_CASE("Post Machine: run full program and fail on missing line", "[post][machine]") {
    Machine machine;
    machine.program().addRule(1, Command(Command::kMark, 2));
    machine.program().addRule(2, Command(Command::kStop));
    machine.reset();

    REQUIRE(machine.run());
    REQUIRE(machine.status() == MachineStatus::Stopped);

    Machine bad;
    bad.program().addRule(1, Command(Command::kMark, 99));
    bad.reset();
    REQUIRE(bad.step());                    // выполнили V, currentLine_ стал 99
    REQUIRE_FALSE(bad.step());              // строки 99 нет → Failed
    REQUIRE(bad.status() == MachineStatus::Failed);
}