#pragma once
#include <iostream>
#include <string>

/**
 * @brief Лента машины Поста: бесконечная последовательность ячеек ('0' пусто, '1' метка).
 *
 * Текстовый формат: строка из '0' и '1', затем индекс ячейки с кареткой. Пример: "00100 2".
 */
class Tape {
public:
    Tape();
    Tape(const Tape& other) = default;
    Tape& operator=(const Tape& other) = default;
    ~Tape() = default;

    void mark();              ///< Ставит метку в текущей ячейке.
    void erase();             ///< Убирает метку из текущей ячейки.
    bool isMarked() const;    ///< Есть ли метка в текущей ячейке.
    int position() const;     ///< Индекс ячейки, на которой стоит каретка.

    Tape& operator++();       ///< Каретка вправо, лента расширяется при необходимости.
    Tape& operator--();       ///< Каретка влево, лента расширяется при необходимости.

   bool operator==(const Tape& other) const;
   bool operator!=(const Tape& other) const;

    /// Записывает ленту в формате "ячейки позиция".
    friend std::ostream& operator<<(std::ostream& os, const Tape& tape);

    /// Читает ленту. При неверных данных ставит failbit и не меняет ленту.
    friend std::istream& operator>>(std::istream& is, Tape& tape);

private:
    std::string cells_;       ///< Ячейки ленты.
    int position_;            ///< Позиция каретки.
};