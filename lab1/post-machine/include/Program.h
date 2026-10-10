#pragma once
#include <cstddef>
#include <iostream>
#include <map>

/// Тип команды машины Поста.
enum class CommandType { Mark, Erase, Left, Right, Check, Stop };

/**
 * @brief Одна команда программы машины Поста.
 *
 * Текстовый формат: "V n" (поставить метку), "X n" (стереть метку),
 * "R n" (вправо), "L n" (влево), "? a b" (проверка), "!" (остановка).
 * Для Check переход next выполняется, если в ячейке есть метка,
 * а alternative выполняется, если ячейка пуста.
 */
class Command {
public:
    /// Значение перехода, которое означает "перехода нет".
    static constexpr int kNoTarget = 0;

    /// Номер первой допустимой строки программы.
    static constexpr int kFirstLineNumber = 1;

    /// Создаёт команду остановки.
    Command();

    /// Создаёт команду заданного типа с переходами.
    Command(CommandType type, int next = kNoTarget, int alternative = kNoTarget);

    /// Конструктор копирования.
    Command(const Command& other) = default;

    /// Оператор присваивания копированием.
    Command& operator=(const Command& other) = default;

    /// Деструктор.
    ~Command() = default;

    /// Возвращает тип команды.
    CommandType type() const;

    /// Возвращает номер следующей строки (для Check: если в ячейке есть метка).
    int next() const;

    /// Возвращает номер строки для Check, если ячейка пуста.
    int alternative() const;

    /// Сравнивает команды по типу и переходам.
    bool operator==(const Command& other) const;

    /// Возвращает true, если команды различаются.
    bool operator!=(const Command& other) const;

    /// Записывает команду в поток в текстовом формате.
    friend std::ostream& operator<<(std::ostream& os, const Command& command);

    /**
     * @brief Читает команду из потока.
     *
     * При неизвестном символе или неверном переходе поток переходит
     * в состояние failbit, а команда остаётся прежней.
     */
    friend std::istream& operator>>(std::istream& is, Command& command);

private:
    /// Тип команды.
    CommandType type_;

    /// Основной переход.
    int next_;

    /// Дополнительный переход (только для Check).
    int alternative_;
};

/**
 * @brief Программа машины Поста: набор команд, пронумерованных строками.
 *
 * Текстовый формат: сначала количество правил, затем по одной строке
 * "номер команда". Пример:
 *   2
 *   1 V 2
 *   2 !
 */
class Program {
public:
    /// Создаёт пустую программу.
    Program();

    /// Конструктор копирования.
    Program(const Program& other);

    /// Оператор присваивания копированием.
    Program& operator=(const Program& other);

    /// Деструктор (память освобождает std::map).
    ~Program() = default;

    /**
     * @brief Добавляет правило.
     * @return false, если номер меньше 1 или правило с таким номером уже есть.
     */
    bool addRule(int number, const Command& command);

    /// Удаляет правило. Возвращает false, если правила с таким номером нет.
    bool removeRule(int number);

    /// Проверяет, есть ли правило с указанным номером.
    bool hasRule(int number) const;

    /// Возвращает правило по номеру. Бросает std::out_of_range, если его нет.
    const Command& getRule(int number) const;

    /// Возвращает наименьший номер правила или Command::kNoTarget для пустой программы.
    int startNumber() const;

    /// Возвращает количество правил.
    std::size_t size() const;

    /// Возвращает true, если в программе нет правил.
    bool empty() const;

    /// Сравнивает программы по набору правил.
    bool operator==(const Program& other) const;

    /// Возвращает true, если программы различаются.
    bool operator!=(const Program& other) const;

    /// Записывает программу в поток в текстовом формате.
    friend std::ostream& operator<<(std::ostream& os, const Program& program);

    /**
     * @brief Читает программу из потока.
     *
     * При неверных данных (плохая команда, повторный номер, номер меньше 1)
     * поток переходит в состояние failbit, а программа остаётся прежней.
     */
    friend std::istream& operator>>(std::istream& is, Program& program);

private:
    /// Правила: ключ это номер строки, значение это команда.
    std::map<int, Command> rules_;
};