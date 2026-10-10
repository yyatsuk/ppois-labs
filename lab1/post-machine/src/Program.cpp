#include "Program.h"

namespace {
const char kMarkSymbol = 'V';
const char kEraseSymbol = 'X';
const char kLeftSymbol = 'L';
const char kRightSymbol = 'R';
const char kCheckSymbol = '?';
const char kStopSymbol = '!';

const int kNoTargets = 0;
const int kOneTarget = 1;
const int kTwoTargets = 2;

char toSymbol(CommandType type) {
    switch (type) {
        case CommandType::Mark: return kMarkSymbol;
        case CommandType::Erase: return kEraseSymbol;
        case CommandType::Left: return kLeftSymbol;
        case CommandType::Right: return kRightSymbol;
        case CommandType::Check: return kCheckSymbol;
        case CommandType::Stop: break;
    }
    return kStopSymbol;
}

bool toType(char symbol, CommandType& type) {
    switch (symbol) {
        case kMarkSymbol: type = CommandType::Mark; return true;
        case kEraseSymbol: type = CommandType::Erase; return true;
        case kLeftSymbol: type = CommandType::Left; return true;
        case kRightSymbol: type = CommandType::Right; return true;
        case kCheckSymbol: type = CommandType::Check; return true;
        case kStopSymbol: type = CommandType::Stop; return true;
        default: return false;
    }
}

int targetCount(CommandType type) {
    if (type == CommandType::Stop) {
        return kNoTargets;
    }
    if (type == CommandType::Check) {
        return kTwoTargets;
    }
    return kOneTarget;
}

bool readType(std::istream& is, CommandType& type) {
    char symbol = kStopSymbol;
    if (!(is >> symbol)) {
        return false;
    }
    if (!toType(symbol, type)) {
        is.setstate(std::ios::failbit);
        return false;
    }
    return true;
}

void readTargets(std::istream& is, int count, int& next, int& alternative) {
    if (count >= kOneTarget) {
        is >> next;
    }
    if (count >= kTwoTargets) {
        is >> alternative;
    }
}

bool areTargetsValid(int count, int next, int alternative) {
    bool nextIsValid = count < kOneTarget || next >= Command::kFirstLineNumber;
    bool alternativeIsValid = count < kTwoTargets
        || alternative >= Command::kFirstLineNumber;
    return nextIsValid && alternativeIsValid;
}

bool readRules(std::istream& is, int count, std::map<int, Command>& rules) {
    for (int index = 0; index < count; ++index) {
        int number = 0;
        Command command;
        if (!(is >> number >> command)) {
            return false;
        }
        if (number < Command::kFirstLineNumber) {
            return false;
        }
        if (!rules.emplace(number, command).second) {
            return false;
        }
    }
    return true;
}
}

Command::Command()
    : type_(CommandType::Stop), next_(kNoTarget), alternative_(kNoTarget) {
}

Command::Command(CommandType type, int next, int alternative)
    : type_(type), next_(next), alternative_(alternative) {
}

CommandType Command::type() const {
    return type_;
}

int Command::next() const {
    return next_;
}

int Command::alternative() const {
    return alternative_;
}

bool Command::operator==(const Command& other) const {
    return type_ == other.type_ && next_ == other.next_
        && alternative_ == other.alternative_;
}

bool Command::operator!=(const Command& other) const {
    return !(*this == other);
}

std::ostream& operator<<(std::ostream& os, const Command& command) {
    int count = targetCount(command.type_);
    os << toSymbol(command.type_);
    if (count >= kOneTarget) {
        os << ' ' << command.next_;
    }
    if (count >= kTwoTargets) {
        os << ' ' << command.alternative_;
    }
    return os;
}

std::istream& operator>>(std::istream& is, Command& command) {
    CommandType type = CommandType::Stop;
    int next = Command::kNoTarget;
    int alternative = Command::kNoTarget;
    if (!readType(is, type)) {
        return is;
    }
    int count = targetCount(type);
    readTargets(is, count, next, alternative);
    if (!is) {
        return is;
    }
    if (!areTargetsValid(count, next, alternative)) {
        is.setstate(std::ios::failbit);
        return is;
    }
    command = Command(type, next, alternative);
    return is;
}

Program::Program() : rules_() {
}

Program::Program(const Program& other) : rules_(other.rules_) {
}

Program& Program::operator=(const Program& other) {
    if (this != &other) {
        rules_ = other.rules_;
    }
    return *this;
}

bool Program::addRule(int number, const Command& command) {
    if (number < Command::kFirstLineNumber) {
        return false;
    }
    return rules_.emplace(number, command).second;
}

bool Program::removeRule(int number) {
    return rules_.erase(number) > 0;
}

bool Program::hasRule(int number) const {
    return rules_.count(number) > 0;
}

const Command& Program::getRule(int number) const {
    return rules_.at(number);
}

int Program::startNumber() const {
    if (rules_.empty()) {
        return Command::kNoTarget;
    }
    // std::map хранит ключи по возрастанию, первый элемент это наименьший номер.
    return rules_.begin()->first;
}

std::size_t Program::size() const {
    return rules_.size();
}

bool Program::empty() const {
    return rules_.empty();
}

bool Program::operator==(const Program& other) const {
    return rules_ == other.rules_;
}

bool Program::operator!=(const Program& other) const {
    return !(*this == other);
}

std::ostream& operator<<(std::ostream& os, const Program& program) {
    os << program.rules_.size() << '\n';
    for (const auto& rule : program.rules_) {
        os << rule.first << ' ' << rule.second << '\n';
    }
    return os;
}

std::istream& operator>>(std::istream& is, Program& program) {
    int ruleCount = 0;
    if (!(is >> ruleCount)) {
        return is;
    }
    std::map<int, Command> loadedRules;
    if (ruleCount < 0 || !readRules(is, ruleCount, loadedRules)) {
        is.setstate(std::ios::failbit);
        return is;
    }
    program.rules_ = loadedRules;
    return is;
}