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
 * Текстовый формат: сначала лента ("ячейки позиция"), затем программа.
 * Пример:
 *   0000 0
 *   2
 *   1 V 2
 *   2 !
 */
class Machine {
public:
    /// Предел шагов для run(), защищающий от бесконечных программ.
    static constexpr std::size_t kDefaultStepLimit = 1000000;

    /// Создаёт машину с пустой лентой и пустой программой.
    Machine();

    /// Конструктор копирования.
    Machine(const Machine& other);

    /// Оператор присваивания копированием.
    Machine& operator=(const Machine& other);

    /// Деструктор.
    ~Machine() = default;

    /// Загружает ленту из потока. При ошибке чтения машина не меняется.
    bool loadTape(std::istream& is);

    /// Загружает программу из потока и сбрасывает машину на первую строку.
    bool loadProgram(std::istream& is);

    /// Делает текущей первую строку программы и снимает остановку.
    void reset();

    /// Выполняет одну команду. Возвращает false, если выполнять больше нечего.
    bool step();

    /**
     * @brief Выполняет программу до остановки, ошибки или предела шагов.
     * @return true, если программа завершилась командой остановки.
     */
    bool run(std::size_t stepLimit = kDefaultStepLimit);

    /// Лента (для чтения).
    const Tape& tape() const;

    /// Лента (для задания и изменения значений).
    Tape& tape();

    /// Программа (для чтения и просмотра правил).
    const Program& program() const;

    /// Программа (для добавления и удаления правил). После правок вызовите reset().
    Program& program();

    /// Номер строки, которая будет выполнена следующей.
    int currentLine() const;

    /// Состояние выполнения.
    MachineStatus status() const;

    /// Сравнивает машины по ленте, программе, текущей строке и состоянию.
    bool operator==(const Machine& other) const;

    /// Возвращает true, если машины различаются.
    bool operator!=(const Machine& other) const;

    /// Выводит краткое состояние для журнала (флаг -log).
    void printState(std::ostream& os) const;

    /// Записывает машину в поток в формате "лента, затем программа".
    friend std::ostream& operator<<(std::ostream& os, const Machine& machine);

    /**
     * @brief Читает машину из потока в формате "лента, затем программа".
     *
     * При неверных данных поток переходит в состояние failbit,
     * а машина остаётся прежней.
     */
    friend std::istream& operator>>(std::istream& is, Machine& machine);

private:
    /// Лента машины.
    Tape tape_;

    /// Программа машины.
    Program program_;

    /// Номер строки, которая будет выполнена следующей.
    int currentLine_;

    /// Состояние выполнения.
    MachineStatus status_;

    /// Выполняет одну команду и вычисляет следующую строку.
    void execute(const Command& command);
};