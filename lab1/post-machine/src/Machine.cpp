#include "Machine.h"

namespace {
const char* const kStatusNames[] = {"running", "stopped", "failed"};
}

void Machine::reset() {
    currentLine_ = program_.firstLine();
    status_ = MachineStatus::Running;
}

void Machine::execute(const Command& command) {
    switch (command.type()) {
        case Command::kMark: tape_.mark(); break;
        case Command::kErase: tape_.erase(); break;
        case Command::kLeft: --tape_; break;
        case Command::kRight: ++tape_; break;
        case Command::kCheck:
            // Проверка сама выбирает следующую строку в зависимости от метки.
            currentLine_ = tape_.isMarked() ? command.next() : command.alternative();
            return;
        case Command::kStop:
            status_ = MachineStatus::Stopped;
            return;
    }
    currentLine_ = command.next();
}

bool Machine::step() {
    if (status_ != MachineStatus::Running) {
        return false;
    }
    const Command* command = program_.findRule(currentLine_);
    if (command == nullptr) {
        // Переход на строку, которой нет в программе, считается ошибкой.
        status_ = MachineStatus::Failed;
        return false;
    }
    execute(*command);
    return true;
}

bool Machine::run(std::size_t stepLimit) {
    std::size_t executedSteps = 0;
    while (executedSteps < stepLimit && step()) {
        ++executedSteps;
    }
    return status_ == MachineStatus::Stopped;
}

const Tape& Machine::tape() const { return tape_; }

Tape& Machine::tape() { return tape_; }

const Program& Machine::program() const { return program_; }

Program& Machine::program() { return program_; }

int Machine::currentLine() const { return currentLine_; }

MachineStatus Machine::status() const { return status_; }

bool Machine::operator==(const Machine& other) const {
    return tape_ == other.tape_ && program_ == other.program_
        && currentLine_ == other.currentLine_ && status_ == other.status_;
}

bool Machine::operator!=(const Machine& other) const {
    return !(*this == other);
}

void Machine::printState(std::ostream& os) const {
    os << tape_ << " | line " << currentLine_ << " | " << kStatusNames[static_cast<int>(status_)];
}

std::ostream& operator<<(std::ostream& os, const Machine& machine) {
    return os << machine.tape_ << '\n' << machine.program_;
}

std::istream& operator>>(std::istream& is, Machine& machine) {
    Tape tape;
    Program program;
    if (!(is >> tape >> program)) {
        return is;
    }
    machine.tape_ = tape;
    machine.program_ = program;
    machine.reset();
    return is;
}