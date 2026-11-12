#include "TuringMachine.h"

namespace {
const char* const kStatusNames[] = {"running", "halted", "failed"};
}

void TuringMachine::reset() {
    currentState_ = startState_;
    status_ = currentState_ == haltState_ ? TuringStatus::Halted : TuringStatus::Running;
}

void TuringMachine::apply(const TuringRule& rule) {
    tape_.write(rule.writeSymbol());
    if (rule.move() == TuringRule::kLeft) {
        --tape_;
    } else if (rule.move() == TuringRule::kRight) {
        ++tape_;
    }
    currentState_ = rule.nextState();
    if (currentState_ == haltState_) {
        status_ = TuringStatus::Halted;
    }
}

bool TuringMachine::step() {
    if (status_ != TuringStatus::Running) {
        return false;
    }
    const TuringRule* rule = program_.findRule(currentState_, tape_.read());
    if (rule == nullptr) {
        // Для пары (состояние, символ) нет правила: машина не может продолжить.
        status_ = TuringStatus::Failed;
        return false;
    }
    apply(*rule);
    return true;
}

bool TuringMachine::run(std::size_t stepLimit) {
    std::size_t executedSteps = 0;
    while (executedSteps < stepLimit && step()) {
        ++executedSteps;
    }
    return status_ == TuringStatus::Halted;
}

const TuringTape& TuringMachine::tape() const { return tape_; }

TuringTape& TuringMachine::tape() { return tape_; }

const TuringProgram& TuringMachine::program() const { return program_; }

TuringProgram& TuringMachine::program() { return program_; }

const std::string& TuringMachine::currentState() const { return currentState_; }

TuringStatus TuringMachine::status() const { return status_; }

bool TuringMachine::operator==(const TuringMachine& other) const {
    return tape_ == other.tape_ && program_ == other.program_
        && startState_ == other.startState_ && haltState_ == other.haltState_
        && currentState_ == other.currentState_ && status_ == other.status_;
}

bool TuringMachine::operator!=(const TuringMachine& other) const { return !(*this == other); }

void TuringMachine::printState(std::ostream& os) const {
    os << tape_ << " | state " << currentState_ << " | " << kStatusNames[static_cast<int>(status_)];
}

std::ostream& operator<<(std::ostream& os, const TuringMachine& machine) {
    return os << machine.startState_ << ' ' << machine.haltState_ << '\n'
              << machine.tape_ << '\n' << machine.program_;
}

std::istream& operator>>(std::istream& is, TuringMachine& machine) {
    std::string startState;
    std::string haltState;
    TuringTape tape;
    TuringProgram program;
    if (!(is >> startState >> haltState >> tape >> program)) {
        return is;
    }
    machine.tape_ = tape;
    machine.program_ = program;
    machine.startState_ = startState;
    machine.haltState_ = haltState;
    machine.reset();
    return is;
}