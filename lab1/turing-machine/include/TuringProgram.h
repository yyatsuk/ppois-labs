#pragma once
#include <cstddef>
#include <iostream>
#include <map>
#include <string>
#include <utility>
#include "TuringTape.h"

/**
 * @brief Правило машины Тьюринга.
 *
 * Читается так: "в состоянии state при символе readSymbol записать writeSymbol,
 * сдвинуться по move и перейти в nextState".
 * Текстовый формат: "state read write move next", где move это L, R или N.
 * Пример: "q0 1 0 R q1".
 */
class TuringRule {
public:
    static constexpr char kLeft = 'L';   ///< Сдвиг влево.
    static constexpr char kRight = 'R';  ///< Сдвиг вправо.
    static constexpr char kStay = 'N';   ///< Остаться на месте.

    explicit TuringRule(const std::string& state = "", char readSymbol = TuringTape::kBlank,
                        char writeSymbol = TuringTape::kBlank, char move = kStay,
                        const std::string& nextState = "");
    TuringRule(const TuringRule& other) = default;
    TuringRule& operator=(const TuringRule& other) = default;
    ~TuringRule() = default;

    /// Проверяет, что символ является допустимым направлением (L, R или N).
    static bool isMove(char move);

    const std::string& state() const;      ///< Состояние, в котором правило применимо.
    char readSymbol() const;               ///< Символ под кареткой, при котором правило применимо.
    char writeSymbol() const;              ///< Символ, который нужно записать.
    char move() const;                     ///< Направление движения каретки.
    const std::string& nextState() const;  ///< Состояние, в которое переходит машина.

    bool operator==(const TuringRule& other) const;  ///< Сравнивает все поля.
    bool operator!=(const TuringRule& other) const;  ///< Возвращает true, если правила различаются.

    /// Записывает правило в текстовом формате.
    friend std::ostream& operator<<(std::ostream& os, const TuringRule& rule);

    /// Читает правило. При неверном направлении ставит failbit и не меняет правило.
    friend std::istream& operator>>(std::istream& is, TuringRule& rule);

private:
    std::string state_;
    char readSymbol_;
    char writeSymbol_;
    char move_;
    std::string nextState_;
};

/**
 * @brief Программа машины Тьюринга: таблица правил по паре (состояние, символ).
 *
 * Текстовый формат: количество правил, затем по одному правилу в строке.
 */
class TuringProgram {
public:
    TuringProgram() = default;
    TuringProgram(const TuringProgram& other) = default;
    TuringProgram& operator=(const TuringProgram& other) = default;
    ~TuringProgram() = default;

    /// Добавляет правило. false, если пара (состояние, символ) занята или правило неверно.
    bool addRule(const TuringRule& rule);

    /// Удаляет правило. false, если такого правила нет.
    bool removeRule(const std::string& state, char readSymbol);

    /// Возвращает правило для пары (состояние, символ) или nullptr, если его нет.
    const TuringRule* findRule(const std::string& state, char readSymbol) const;

    std::size_t size() const;  ///< Количество правил.

    bool operator==(const TuringProgram& other) const;  ///< Сравнивает наборы правил.
    bool operator!=(const TuringProgram& other) const;  ///< Возвращает true, если программы различаются.

    /// Записывает программу в текстовом формате.
    friend std::ostream& operator<<(std::ostream& os, const TuringProgram& program);

    /// Читает программу. При неверных данных ставит failbit и не меняет программу.
    friend std::istream& operator>>(std::istream& is, TuringProgram& program);

private:
    using Key = std::pair<std::string, char>;  ///< Состояние и читаемый символ.

    std::map<Key, TuringRule> rules_;  ///< Таблица правил.
};