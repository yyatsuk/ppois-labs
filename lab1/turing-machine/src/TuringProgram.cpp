#include "TuringProgram.h"

namespace {
const std::string kMoves = "LRN";
}

TuringRule::TuringRule(const std::string& state, char readSymbol, char writeSymbol,
                       char move, const std::string& nextState)
    : state_(state), readSymbol_(readSymbol), writeSymbol_(writeSymbol),
      move_(move), nextState_(nextState) {}

bool TuringRule::isMove(char move) { return kMoves.find(move) != std::string::npos; }

const std::string& TuringRule::state() const { return state_; }

char TuringRule::readSymbol() const { return readSymbol_; }

char TuringRule::writeSymbol() const { return writeSymbol_; }

char TuringRule::move() const { return move_; }

const std::string& TuringRule::nextState() const { return nextState_; }

bool TuringRule::operator==(const TuringRule& other) const {
    return state_ == other.state_ && readSymbol_ == other.readSymbol_
        && writeSymbol_ == other.writeSymbol_ && move_ == other.move_
        && nextState_ == other.nextState_;
}

bool TuringRule::operator!=(const TuringRule& other) const { return !(*this == other); }

std::ostream& operator<<(std::ostream& os, const TuringRule& rule) {
    return os << rule.state_ << ' ' << rule.readSymbol_ << ' ' << rule.writeSymbol_
              << ' ' << rule.move_ << ' ' << rule.nextState_;
}

std::istream& operator>>(std::istream& is, TuringRule& rule) {
    std::string state;
    std::string nextState;
    char readSymbol = TuringTape::kBlank;
    char writeSymbol = TuringTape::kBlank;
    char move = TuringRule::kStay;
    if (!(is >> state >> readSymbol >> writeSymbol >> move >> nextState)) {
        return is;
    }
    if (!TuringRule::isMove(move)) {
        is.setstate(std::ios::failbit);
        return is;
    }
    rule = TuringRule(state, readSymbol, writeSymbol, move, nextState);
    return is;
}

bool TuringProgram::addRule(const TuringRule& rule) {
    bool isValid = TuringRule::isMove(rule.move()) && !rule.state().empty()
        && !rule.nextState().empty();
    return isValid && rules_.emplace(Key(rule.state(), rule.readSymbol()), rule).second;
}

bool TuringProgram::removeRule(const std::string& state, char readSymbol) {
    return rules_.erase(Key(state, readSymbol)) > 0;
}

const TuringRule* TuringProgram::findRule(const std::string& state, char readSymbol) const {
    auto found = rules_.find(Key(state, readSymbol));
    return found == rules_.end() ? nullptr : &found->second;
}

std::size_t TuringProgram::size() const { return rules_.size(); }

bool TuringProgram::operator==(const TuringProgram& other) const {
    return rules_ == other.rules_;
}

bool TuringProgram::operator!=(const TuringProgram& other) const { return !(*this == other); }

std::ostream& operator<<(std::ostream& os, const TuringProgram& program) {
    os << program.rules_.size() << '\n';
    for (const auto& entry : program.rules_) {
        os << entry.second << '\n';
    }
    return os;
}

std::istream& operator>>(std::istream& is, TuringProgram& program) {
    int count = 0;
    if (!(is >> count)) {
        return is;
    }
    TuringProgram loaded;
    for (int index = 0; index < count && is; ++index) {
        TuringRule rule;
        is >> rule;
        if (is && !loaded.addRule(rule)) {
            is.setstate(std::ios::failbit);
        }
    }
    if (count < 0 || !is) {
        is.setstate(std::ios::failbit);
        return is;
    }
    program = loaded;
    return is;
}