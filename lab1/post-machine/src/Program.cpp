#include "Program.h"

namespace {
bool isKnownType(char type) {
    switch (type) {
        case Command::kMark:
        case Command::kErase:
        case Command::kLeft:
        case Command::kRight:
        case Command::kCheck:
        case Command::kStop:
            return true;
        default:
            return false;
    }
}

bool hasNext(char type) { return type != Command::kStop; }

bool hasAlternative(char type) { return type == Command::kCheck; }
}

Command::Command(char type, int next, int alternative)
    : type_(type), next_(next), alternative_(alternative) {}

char Command::type() const { return type_; }

int Command::next() const { return next_; }

int Command::alternative() const { return alternative_; }

bool Command::operator==(const Command& other) const {
    return type_ == other.type_ && next_ == other.next_
        && alternative_ == other.alternative_;
}

bool Command::operator!=(const Command& other) const {
    return !(*this == other);
}

std::ostream& operator<<(std::ostream& os, const Command& command) {
    os << command.type_;
    if (hasNext(command.type_)) {
        os << ' ' << command.next_;
    }
    if (hasAlternative(command.type_)) {
        os << ' ' << command.alternative_;
    }
    return os;
}

std::istream& operator>>(std::istream& is, Command& command) {
    char type = Command::kStop;
    int next = Command::kNoTarget;
    int alternative = Command::kNoTarget;
    if (!(is >> type)) {
        return is;
    }
    if (hasNext(type)) {
        is >> next;
    }
    if (hasAlternative(type)) {
        is >> alternative;
    }
    bool isValid = is && isKnownType(type)
        && (!hasNext(type) || next >= Command::kFirstLine)
        && (!hasAlternative(type) || alternative >= Command::kFirstLine);
    if (!isValid) {
        is.setstate(std::ios::failbit);
        return is;
    }
    command = Command(type, next, alternative);
    return is;
}

bool Program::addRule(int number, const Command& command) {
    return number >= Command::kFirstLine && rules_.emplace(number, command).second;
}

bool Program::removeRule(int number) { return rules_.erase(number) > 0; }

const Command* Program::findRule(int number) const {
    auto found = rules_.find(number);
    return found == rules_.end() ? nullptr : &found->second;
}

int Program::firstLine() const {
    // std::map хранит ключи по возрастанию, первый элемент это наименьший номер.
    return rules_.empty() ? Command::kNoTarget : rules_.begin()->first;
}

std::size_t Program::size() const { return rules_.size(); }

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
    int count = 0;
    if (!(is >> count)) {
        return is;
    }
    Program loaded;
    for (int index = 0; index < count && is; ++index) {
        int number = 0;
        Command command;
        is >> number >> command;
        if (is && !loaded.addRule(number, command)) {
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