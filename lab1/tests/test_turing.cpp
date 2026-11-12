#include <catch2/catch_test_macros.hpp>
#include <sstream>
#include "TuringMachine.h"
#include "TuringProgram.h"
#include "TuringTape.h"

// ============ Tape (5 тестов) ============

TEST_CASE("Turing Tape: starts blank at position 0", "[turing][tape]") {
    TuringTape tape;
    REQUIRE(tape.position() == 0);
    REQUIRE(tape.read() == TuringTape::kBlank);
}

TEST_CASE("Turing Tape: write and read", "[turing][tape]") {
    TuringTape tape;
    tape.write('A');
    REQUIRE(tape.read() == 'A');
    tape.write('_');
    REQUIRE(tape.read() == TuringTape::kBlank);
}

TEST_CASE("Turing Tape: move right extends tape", "[turing][tape]") {
    TuringTape tape;
    ++tape;
    REQUIRE(tape.position() == 1);
    REQUIRE(tape.read() == TuringTape::kBlank);
}

TEST_CASE("Turing Tape: move left at zero stays at zero", "[turing][tape]") {
    TuringTape tape;
    --tape;
    REQUIRE(tape.position() == 0);
}

TEST_CASE("Turing Tape: equality and round-trip", "[turing][tape]") {
    TuringTape a;
    a.write('X');
    ++a;
    a.write('Y');

    TuringTape b = a;
    REQUIRE(a == b);

    std::stringstream stream;
    stream << a;
    TuringTape restored;
    stream >> restored;
    REQUIRE(a == restored);

    b.write('Z');
    REQUIRE(a != b);
}

// ============ Rule + Program (5 тестов) ============

TEST_CASE("Turing Rule: default construction", "[turing][program]") {
    TuringRule rule;
    REQUIRE(rule.state().empty());
    REQUIRE(rule.nextState().empty());
    REQUIRE(rule.move() == TuringRule::kStay);
    REQUIRE(rule.readSymbol() == TuringTape::kBlank);
}

TEST_CASE("Turing Rule: isMove accepts L, R, N only", "[turing][program]") {
    REQUIRE(TuringRule::isMove('L'));
    REQUIRE(TuringRule::isMove('R'));
    REQUIRE(TuringRule::isMove('N'));
    REQUIRE_FALSE(TuringRule::isMove('X'));
    REQUIRE_FALSE(TuringRule::isMove('l'));
}

TEST_CASE("Turing Rule: equality compares all fields", "[turing][program]") {
    TuringRule a("q0", '0', '1', 'R', "q1");
    TuringRule b("q0", '0', '1', 'R', "q1");
    REQUIRE(a == b);

    TuringRule c("q0", '0', '1', 'L', "q1");
    REQUIRE(a != c);
}

TEST_CASE("Turing Program: add and cannot add duplicate key", "[turing][program]") {
    TuringProgram program;
    REQUIRE(program.addRule(TuringRule("q0", '0', '1', 'R', "q1")));
    REQUIRE(program.size() == 1);
    REQUIRE_FALSE(program.addRule(TuringRule("q0", '0', '0', 'L', "q2")));
    REQUIRE(program.size() == 1);
}

TEST_CASE("Turing Program: find, remove and round-trip", "[turing][program]") {
    TuringProgram program;
    program.addRule(TuringRule("q0", '0', '1', 'R', "q1"));
    program.addRule(TuringRule("q1", '1', '0', 'L', "q0"));

    REQUIRE(program.findRule("q0", '0') != nullptr);
    REQUIRE(program.findRule("q0", '1') == nullptr);

    std::stringstream stream;
    stream << program;
    TuringProgram restored;
    stream >> restored;
    REQUIRE(program == restored);

    REQUIRE(program.removeRule("q0", '0'));
    REQUIRE_FALSE(program.removeRule("q0", '0'));
    REQUIRE(program.size() == 1);
}

// ============ Machine (5 тестов) ============

TEST_CASE("Turing Machine: starts running in q0", "[turing][machine]") {
    TuringMachine machine;
    REQUIRE(machine.status() == TuringStatus::Running);
    REQUIRE(machine.currentState() == "q0");
}

TEST_CASE("Turing Machine: step writes symbol", "[turing][machine]") {
    TuringMachine machine;
    machine.program().addRule(TuringRule("q0", '_', 'X', 'N', "q1"));
    machine.reset();

    REQUIRE(machine.step());
    REQUIRE(machine.tape().read() == 'X');
}

TEST_CASE("Turing Machine: step moves and changes state", "[turing][machine]") {
    TuringMachine machine;
    machine.program().addRule(TuringRule("q0", '_', '_', 'R', "q1"));
    machine.program().addRule(TuringRule("q1", '_', '_', 'N', "qf"));
    machine.reset();

    machine.step();
    REQUIRE(machine.tape().position() == 1);
    REQUIRE(machine.currentState() == "q1");

    machine.step();
    REQUIRE(machine.status() == TuringStatus::Halted);
}

TEST_CASE("Turing Machine: fails when no rule for pair", "[turing][machine]") {
    TuringMachine machine;
    machine.reset();
    REQUIRE_FALSE(machine.step());
    REQUIRE(machine.status() == TuringStatus::Failed);
}

TEST_CASE("Turing Machine: run inverts bits", "[turing][machine]") {
    TuringMachine machine;
    machine.program().addRule(TuringRule("q0", '0', '1', 'R', "q0"));
    machine.program().addRule(TuringRule("q0", '1', '0', 'R', "q0"));
    machine.program().addRule(TuringRule("q0", '_', '_', 'N', "qf"));
    machine.reset();

    machine.tape().write('1');
    ++machine.tape();
    machine.tape().write('0');

    REQUIRE(machine.run());
    REQUIRE(machine.status() == TuringStatus::Halted);
    REQUIRE(machine.tape().read() == TuringTape::kBlank);
}