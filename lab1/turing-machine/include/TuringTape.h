#pragma once
#include <iostream>
#include <string>

/**
 * @brief Лента машины Тьюринга: бесконечная последовательность ячеек с символами.
 *
 * Текстовый формат: строка символов без пробелов, затем индекс ячейки с кареткой.
 * Пример: "1011_ 0". Символ kBlank обозначает пустую ячейку.
 */
class TuringTape {
public:
    static constexpr char kBlank = '_';  ///< Символ пустой ячейки.

    TuringTape();
    TuringTape(const TuringTape& other) = default;
    TuringTape& operator=(const TuringTape& other) = default;
    ~TuringTape() = default;

    char read() const;           ///< Символ в текущей ячейке.
    void write(char symbol);     ///< Записывает символ в текущую ячейку.
    int position() const;        ///< Индекс ячейки, на которой стоит каретка.

    TuringTape& operator++();    ///< Каретка вправо, лента расширяется при необходимости.
    TuringTape& operator--();    ///< Каретка влево, лента расширяется при необходимости.

    bool operator==(const TuringTape& other) const;  ///< Сравнивает ячейки и позицию каретки.
    bool operator!=(const TuringTape& other) const;  ///< Возвращает true, если ленты различаются.

    /// Записывает ленту в формате "ячейки позиция".
    friend std::ostream& operator<<(std::ostream& os, const TuringTape& tape);

    /// Читает ленту. При неверных данных ставит failbit и не меняет ленту.
    friend std::istream& operator>>(std::istream& is, TuringTape& tape);

private:
    std::string cells_;  ///< Ячейки ленты.
    int position_;       ///< Позиция каретки.
};