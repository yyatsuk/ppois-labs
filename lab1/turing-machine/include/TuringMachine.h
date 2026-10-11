#pragma once
#include <cstddef>
#include <iostream>
#include <string>
#include "TuringProgram.h"
#include "TuringTape.h"

/// Состояние выполнения машины Тьюринга.
enum class TuringStatus {
    Running,  ///< Программа выполняется.
    Halted,   ///< Машина достигла завершающего состояния.
    Failed    ///< Для текущей пары (состояние, символ) нет правила.
};

/**
 * @brief Машина Тьюринга: лента, таблица правил и текущее состояние.
 *
 * Текстовый формат: "начальное_состояние завершающее_состояние", затем лента,
 * затем программа. Пример:
 *   q0 qf
 *   1011_ 0
 *   3
 *   q0 0 1 R q0
 *   q0 1 0 R q0
 *   q0 _ _ N qf
 */
class TuringMachine {
public:
    /// Предел шагов для run(), защищающий от бесконечных программ.
    static constexpr std::size_t kDefaultStepLimit = 1000000;

    TuringMachine() = default;
    TuringMachine(const TuringMachine& other) = default;
    TuringMachine& operator=(const TuringMachine& other) = default;
    ~TuringMachine() = default;

    /// Возвращает машину в начальное состояние и снимает остановку.
    void reset();

    /// Применяет одно правило. Возвращает false, если выполнять больше нечего.
    bool step();

    /// Выполняет программу до остановки, ошибки или предела шагов. true, если машина дошла до завершающего состояния.
    bool run(std::size_t stepLimit = kDefaultStepLimit);

    const TuringTape& tape() const;        ///< Лента (чтение).
    TuringTape& tape();                    ///< Лента (задание и изменение значений).
    const TuringProgram& program() const;  ///< Программа (чтение и просмотр правил).
    TuringProgram& program();              ///< Программа (добавление и удаление правил, после правок вызовите reset()).

    const std::string& currentState() const;  ///< Текущее состояние машины.
    TuringStatus status() const;              ///< Состояние выполнения.

    bool operator==(const TuringMachine& other) const;  ///< Сравнивает ленту, программу, состояния и статус.
    bool operator!=(const TuringMachine& other) const;  ///< Возвращает true, если машины различаются.

    /// Выводит краткое состояние для журнала (флаг -log).
    void printState(std::ostream& os) const;

    /// Записывает машину в формате "состояния, лента, программа".
    friend std::ostream& operator<<(std::ostream& os, const TuringMachine& machine);

    /// Читает машину в формате "состояния, лента, программа". При ошибке ставит failbit и не меняет машину.
    friend std::istream& operator>>(std::istream& is, TuringMachine& machine);

private:
    TuringTape tape_;                                  ///< Лента машины.
    TuringProgram program_;                            ///< Программа машины.
    std::string startState_ = "q0";                    ///< Начальное состояние.
    std::string haltState_ = "qf";                     ///< Завершающее состояние.
    std::string currentState_ = "q0";                  ///< Текущее состояние.
    TuringStatus status_ = TuringStatus::Running;      ///< Состояние выполнения.

    /// Применяет правило: запись символа, движение каретки, смена состояния.
    void apply(const TuringRule& rule);
};