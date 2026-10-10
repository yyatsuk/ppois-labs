#include "Tape.h"
#include <string>

namespace {
const char kMarkSymbol = '1';
const char kEmptySymbol = '0';
const int kInitialCellCount = 1;
const int kStartPosition = 0;

bool isValidSymbol(char symbol) {
    return symbol == kMarkSymbol || symbol == kEmptySymbol;
}

bool parseCells(const std::string& rawCells, std::vector<bool>& cells) {
    cells.clear();
    for (char symbol : rawCells) {
        if (!isValidSymbol(symbol)) {
            return false;
        }
        cells.push_back(symbol == kMarkSymbol);
    }
    return !cells.empty();
}
}

Tape::Tape() : cells_(kInitialCellCount, false), position_(kStartPosition) {
}

Tape::Tape(const Tape& other)
    : cells_(other.cells_), position_(other.position_) {
}

Tape& Tape::operator=(const Tape& other) {
    if (this != &other) {
        cells_ = other.cells_;
        position_ = other.position_;
    }
    return *this;
}

void Tape::mark() {
    cells_[position_] = true;
}

void Tape::erase() {
    cells_[position_] = false;
}

bool Tape::isMarked() const {
    return cells_[position_];
}

void Tape::ensureCellExists() {
    if (position_ >= static_cast<int>(cells_.size())) {
        cells_.push_back(false);
    }
}

Tape& Tape::operator++() {
    ++position_;
    ensureCellExists();
    return *this;
}

Tape& Tape::operator--() {
    if (position_ == kStartPosition) {
        cells_.insert(cells_.begin(), false);
    } else {
        --position_;
    }
    return *this;
}

int Tape::position() const {
    return position_;
}

bool Tape::operator==(const Tape& other) const {
    return cells_ == other.cells_ && position_ == other.position_;
}

bool Tape::operator!=(const Tape& other) const {
    return !(*this == other);
}

std::ostream& operator<<(std::ostream& os, const Tape& tape) {
    for (bool cell : tape.cells_) {
        os << (cell ? kMarkSymbol : kEmptySymbol);
    }
    os << ' ' << tape.position_;
    return os;
}

std::istream& operator>>(std::istream& is, Tape& tape) {
    std::string rawCells;
    int position = kStartPosition;
    if (!(is >> rawCells >> position)) {
        return is;
    }
    std::vector<bool> loadedCells;
    bool cellsAreValid = parseCells(rawCells, loadedCells);
    bool positionIsValid = position >= kStartPosition
        && position < static_cast<int>(loadedCells.size());
    if (!cellsAreValid || !positionIsValid) {
        is.setstate(std::ios::failbit);
        return is;
    }
    tape.cells_ = loadedCells;
    tape.position_ = position;
    return is;
}