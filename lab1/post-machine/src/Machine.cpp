#include "Machine.h"

namespace {
const char* const kRunningName = "running";
const char* const kStoppedName = "stopped";
const char* const kFailedName = "failed";

const char* statusName(MachineStatus status) {
    switch (status) {
        case MachineStatus::Running: return kRunningName;
        case MachineStatus::Stopped: return kStoppedName;
        case MachineStatus::Failed: break;
    }
    return kFailedName;
}
}

Machine::Machine()
    : tape_(), program_(), currentLine_(Command::kNoTarget),
      status_(MachineStatus::Running) {
}

Machine::Machine(const Machine& other)
    : tape_(other.tape_), program_(other.program_),
      currentLine_(other.currentLine_), status_(other.status_) {
}

Machine& Machine::operator=(const Machine& other) {
    if (this != &other) {
        tape_ = other.tape_;
        program_ = other.program_;
        currentLine_ = other.currentLine_;
        status_ = other.status_;
    }
    return *this;
}

bool Machine::loadTape(std::istream& is) {
    Tape loadedTape;
    if (!(is >> loadedTape)) {
        return false;
    }
    tape_ = loadedTape;
    return true;
}

bool Machine::loadProgram(std::istream& is) {
    Program loadedProgram;
    if (!(is >> loadedProgram)) {
        return false;
    }
    program_ = loadedProgram;
    reset();
    return true;
}

void Machine::reset() {
    currentLine_ = program_.startNumber();
    status_ = MachineStatus::Running;
}

void Machine::execute(const Command& command) {
    switch (command.type()) {
        case CommandType::Mark: tape_.mark(); break;
        case CommandType::Erase: tape_.erase(); break;
        case CommandType::Left: --tape_; break;
        case CommandType::Right: ++tape_; break;
        case CommandType::Check:
            // Проверка сама выбирает следующую строку в зависимости от метки.
            currentLine_ = tape_.isMarked() ? command.next() : command.alternative();
            return;
        case CommandType::Stop:
            status_ = MachineStatus::Stopped;
            return;
    }
    currentLine_ = command.next();
}

bool Machine::step() {
    if (status_ != MachineStatus::Running) {
        return false;
    }
    if (!program_.hasRule(currentLine_)) {
        // Переход на строку, которой нет в программе, считается ошибкой.
        status_ = MachineStatus::Failed;
        return false;
    }
    execute(program_.getRule(currentLine_));
    return true;
}

bool Machine::run(std::size_t stepLimit) {
    std::size_t executedSteps = 0;
    while (executedSteps < stepLimit && step()) {
        ++executedSteps;
    }
    return status_ == MachineStatus::Stopped;
}

const Tape& Machine::tape() const {
    return tape_;
}

Tape& Machine::tape() {
    return tape_;
}

const Program& Machine::program() const {
    return program_;
}

Program& Machine::program() {
    return program_;
}

int Machine::currentLine() const {
    return currentLine_;
}

MachineStatus Machine::status() const {
    return status_;
}

bool Machine::operator==(const Machine& other) const {
    return tape_ == other.tape_ && program_ == other.program_
        && currentLine_ == other.currentLine_ && status_ == other.status_;
}

bool Machine::operator!=(const Machine& other) const {
    return !(*this == other);
}

void Machine::printState(std::ostream& os) const {
    os << tape_ << " | line " << currentLine_ << " | " << statusName(status_);
}

std::ostream& operator<<(std::ostream& os, const Machine& machine) {
    os << machine.tape_ << '\n' << machine.program_;
    return os;
}

std::istream& operator>>(std::istream& is, Machine& machine) {
    Tape loadedTape;
    Program loadedProgram;
    if (!(is >> loadedTape >> loadedProgram)) {
        return is;
    }
    machine.tape_ = loadedTape;
    machine.program_ = loadedProgram;
    machine.reset();
    return is;
}