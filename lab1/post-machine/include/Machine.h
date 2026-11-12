#pragma once
#include <cstddef>
#include <iostream>
#include "Program.h"
#include "Tape.h"

/// Состояние выполнения машины.
enum class MachineStatus {
    Running,  ///< Программа выполняется.
    Stopped,  ///< Выполнена команда остановки.
    Failed    ///< Переход на несуществующую строку программы.
};

/**
 * @brief Машина Поста: лента, программа и номер текущей строки.
 *
 * Текстовый формат: лента ("ячейки позиция"), затем программа. Пример:
 *   0000 0
 *   2
 *   1 V 2
 *   2 !
 */
class Machine {
public:
    /// Предел шагов для run(), защищающий от бесконечных программ.
    static constexpr std::size_t kDefaultStepLimit = 1000000;

    Machine() = default;
    Machine(const Machine& other) = default;
    Machine& operator=(const Machine& other) = default;
    ~Machine() = default;

    /// Делает текущей первую строку программы и снимает остановку.
    void reset();

    /// Выполняет одну команду. Возвращает false, если выполнять больше нечего.
    bool step();

    /// Выполняет программу до остановки, ошибки или предела шагов. true, если машина остановилась командой.
    bool run(std::size_t stepLimit = kDefaultStepLimit);

    const Tape& tape() const;       ///< Лента (чтение).
    Tape& tape();                   ///< Лента (задание и изменение значений).
    const Program& program() const; ///< Программа (чтение и просмотр правил).
    Program& program();             ///< Программа (добавление и удаление правил, после правок вызовите reset()).

    int currentLine() const;        ///< Номер строки, которая будет выполнена следующей.
    MachineStatus status() const;   ///< Состояние выполнения.

    bool operator==(const Machine& other) const;  ///< Сравнивает ленту, программу, строку и состояние.
    bool operator!=(const Machine& other) const;  ///< Возвращает true, если машины различаются.

    /// Выводит краткое состояние для журнала (флаг -log).
    void printState(std::ostream& os) const;

    /// Записывает машину в формате "лента, затем программа".
    friend std::ostream& operator<<(std::ostream& os, const Machine& machine);

    /// Читает машину в формате "лента, затем программа". При ошибке ставит failbit и не меняет машину.
    friend std::istream& operator>>(std::istream& is, Machine& machine);

private:
    Tape tape_;                                              ///< Лента машины.
    Program program_;                                        ///< Программа машины.
    int currentLine_ = Command::kNoTarget;                   ///< Следующая выполняемая строка.
    MachineStatus status_ = MachineStatus::Running;          ///< Состояние выполнения.

    /// Выполняет одну команду и вычисляет следующую строку.
    void execute(const Command& command);
};