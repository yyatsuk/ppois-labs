#pragma once
#include <cstddef>
#include <iostream>
#include <map>

/**
 * @brief Команда машины Поста.
 *
 * Текстовый формат: "V n" (поставить метку), "X n" (стереть), "R n" (вправо),
 * "L n" (влево), "? a b" (есть метка: перейти к a, иначе к b), "!" (остановка).
 */
class Command {
public:
    static constexpr char kMark = 'V';
    static constexpr char kErase = 'X';
    static constexpr char kLeft = 'L';
    static constexpr char kRight = 'R';
    static constexpr char kCheck = '?';
    static constexpr char kStop = '!';
    static constexpr int kNoTarget = 0;    ///< Значение перехода "перехода нет".
    static constexpr int kFirstLine = 1;   ///< Номер первой допустимой строки программы.

    explicit Command(char type = kStop, int next = kNoTarget, int alternative = kNoTarget);
    Command(const Command& other) = default;
    Command& operator=(const Command& other) = default;
    ~Command() = default;

    char type() const;          ///< Символ команды.
    int next() const;           ///< Следующая строка (для '?': если метка есть).
    int alternative() const;    ///< Строка для '?', если ячейка пуста.

    bool operator==(const Command& other) const;  ///< Сравнивает тип и переходы.
    bool operator!=(const Command& other) const;  ///< Возвращает true, если команды различаются.

    /// Записывает команду в текстовом формате.
    friend std::ostream& operator<<(std::ostream& os, const Command& command);

    /// Читает команду. При неверных данных ставит failbit и не меняет команду.
    friend std::istream& operator>>(std::istream& is, Command& command);

private:
    char type_;
    int next_;
    int alternative_;
};

/**
 * @brief Программа машины Поста: команды, пронумерованные строками.
 *
 * Текстовый формат: количество правил, затем строки "номер команда". Пример:
 *   2
 *   1 V 2
 *   2 !
 */
class Program {
public:
    Program() = default;
    Program(const Program& other) = default;
    Program& operator=(const Program& other) = default;
    ~Program() = default;

    /// Добавляет правило. Возвращает false, если номер меньше 1 или уже занят.
    bool addRule(int number, const Command& command);

    /// Удаляет правило. Возвращает false, если такого номера нет.
    bool removeRule(int number);

    /// Возвращает правило по номеру или nullptr, если его нет.
    const Command* findRule(int number) const;

    /// Наименьший номер правила или Command::kNoTarget для пустой программы.
    int firstLine() const;

    /// Количество правил.
    std::size_t size() const;

    bool operator==(const Program& other) const;  ///< Сравнивает наборы правил.
    bool operator!=(const Program& other) const;  ///< Возвращает true, если программы различаются.

    /// Записывает программу в текстовом формате.
    friend std::ostream& operator<<(std::ostream& os, const Program& program);

    /// Читает программу. При неверных данных ставит failbit и не меняет программу.
    friend std::istream& operator>>(std::istream& is, Program& program);

private:
    std::map<int, Command> rules_;   ///< Номер строки и команда.
};