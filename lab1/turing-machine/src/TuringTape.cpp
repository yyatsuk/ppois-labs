#include "TuringTape.h"

namespace {
const std::size_t kInitialSize = 1;
const int kStartPosition = 0;
}

TuringTape::TuringTape() : cells_(kInitialSize, kBlank), position_(kStartPosition) {}

char TuringTape::read() const { return cells_[position_]; }

void TuringTape::write(char symbol) { cells_[position_] = symbol; }

int TuringTape::position() const { return position_; }

TuringTape& TuringTape::operator++() {
    ++position_;
    if (position_ == static_cast<int>(cells_.size())) {
        cells_ += kBlank;
    }
    return *this;
}

TuringTape& TuringTape::operator--() {
    if (position_ == kStartPosition) {
        // Левее нулевой ячейки ничего нет: вставляем пустую ячейку в начало,
        // каретка остаётся на индексе 0, то есть на новой ячейке.
        cells_.insert(cells_.begin(), kBlank);
    } else {
        --position_;
    }
    return *this;
}

bool TuringTape::operator==(const TuringTape& other) const {
    return cells_ == other.cells_ && position_ == other.position_;
}

bool TuringTape::operator!=(const TuringTape& other) const { return !(*this == other); }

std::ostream& operator<<(std::ostream& os, const TuringTape& tape) {
    return os << tape.cells_ << ' ' << tape.position_;
}

std::istream& operator>>(std::istream& is, TuringTape& tape) {
    std::string cells;
    int position = kStartPosition;
    if (!(is >> cells >> position)) {
        return is;
    }
    if (position < kStartPosition || position >= static_cast<int>(cells.size())) {
        is.setstate(std::ios::failbit);
        return is;
    }
    // Меняем ленту только после успешной проверки обоих полей.
    tape.cells_ = cells;
    tape.position_ = position;
    return is;
}