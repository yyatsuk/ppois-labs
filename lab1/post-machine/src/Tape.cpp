#include "Tape.h"

namespace {
const char kMark = '1';
const char kEmpty = '0';
const char* const kAlphabet = "01";
const std::size_t kInitialSize = 1;
const int kStartPosition = 0;
}

Tape::Tape() : cells_(kInitialSize, kEmpty), position_(kStartPosition) {}

void Tape::mark() { cells_[position_] = kMark; }

void Tape::erase() { cells_[position_] = kEmpty; }

bool Tape::isMarked() const { return cells_[position_] == kMark; }

int Tape::position() const { return position_; }

Tape& Tape::operator++() {
    ++position_;
    if (position_ == static_cast<int>(cells_.size())) {
        cells_ += kEmpty;
    }
    return *this;
}

Tape& Tape::operator--() {
    if (position_ == kStartPosition) {
        // Левее нулевой ячейки ничего нет: вставляем пустую ячейку в начало,
        // каретка остаётся на индексе 0, то есть на новой ячейке.
        cells_.insert(cells_.begin(), kEmpty);
    } else {
        --position_;
    }
    return *this;
}
bool Tape::operator==(const Tape& other) const {
    return cells_ == other.cells_ && position_ == other.position_;
}

bool Tape::operator!=(const Tape& other) const {
    return !(*this == other);
}

std::ostream& operator<<(std::ostream& os, const Tape& tape) {
    return os << tape.cells_ << ' ' << tape.position_;
}

std::istream& operator>>(std::istream& is, Tape& tape) {
    std::string cells;
    int position = kStartPosition;
    if (!(is >> cells >> position)) {
        return is;
    }
    bool isValid = cells.find_first_not_of(kAlphabet) == std::string::npos
        && position >= kStartPosition && position < static_cast<int>(cells.size());
    if (!isValid) {
        is.setstate(std::ios::failbit);
        return is;
    }
    // Меняем ленту только после успешной проверки обоих полей.
    tape.cells_ = cells;
    tape.position_ = position;
    return is;
}