#pragma once

#include <string>
#include <vector>

#include "repertoire.h"

/// Категория мест с ценовым коэффициентом.
class SeatCategory {
public:
    SeatCategory(const std::string& name, double priceMultiplier, const std::string& description);

    /// Цена места этой категории при базовой цене basePrice.
    double calculatePrice(double basePrice) const;

    const std::string& getName() const;

private:
    std::string name_;
    double priceMultiplier_;
    std::string description_;
};

/// Место в зале.
class Seat {
public:
    Seat(int number, const SeatCategory& category);

    /// Бронирует место. Бросает SeatAlreadyBookedException, если оно занято.
    void book();
    void release();
    bool isAvailable() const;

    int getNumber() const;
    const SeatCategory& getCategory() const;

private:
    int number_;
    SeatCategory category_;
    bool isBooked_;
};

/// Ряд мест.
class Row {
public:
    Row(int number, const std::string& sectionName);

    /// Добавляет место. Бросает TheatreException, если номер уже есть в ряду.
    void addSeat(const Seat& seat);
    /// Первое свободное место. Бросает TheatreException, если свободных нет.
    Seat& findFreeSeat();

    int getNumber() const;
    std::vector<Seat>& getSeats();
    const std::vector<Seat>& getSeats() const;

private:
    int number_;
    std::vector<Seat> seats_;
    std::string sectionName_;
};

/// Сцена.
class Stage {
public:
    Stage(double width, double depth, double maxLoadKg);

    /// Выдержит ли сцена вес декорации.
    bool canHost(const Decoration& decoration) const;
    /// Достаточно ли сцена по ширине и глубине.
    bool checkDimensions(double requiredWidth, double requiredDepth) const;

private:
    double width_;
    double depth_;
    double maxLoadKg_;
};

/// Зрительный зал.
class Hall {
public:
    Hall(const std::string& name, int capacity, const Stage& stage);

    /// Добавляет ряд. Бросает HallOverCapacityException, если мест станет больше вместимости.
    void addRow(const Row& row);
    int getFreeSeatsCount() const;
    bool isOverCapacity() const;

    const std::string& getName() const;
    int getCapacity() const;
    std::vector<Row>& getRows();
    const std::vector<Row>& getRows() const;
    const Stage& getStage() const;

private:
    int countSeats() const;

    std::string name_;
    int capacity_;
    std::vector<Row> rows_;
    Stage stage_;
};

/// Репетиция. Дата и время хранятся строкой "ГГГГ-ММ-ДД ЧЧ:ММ".
class Rehearsal {
public:
    Rehearsal(const Production& production, const Stage& stage, const std::string& dateTime,
              int durationMinutes);

    /// Переносит репетицию. Бросает RehearsalConflictException при пересечении с другой.
    void reschedule(const std::string& newDateTime, const std::vector<Rehearsal>& scheduled);
    /// Пересекается ли репетиция по времени с другой.
    bool conflictsWith(const Rehearsal& other) const;

    const std::string& getDateTime() const;

private:
    Production production_;
    Stage stage_;
    std::string dateTime_;
    int durationMinutes_;
};

/// Состояние спектакля.
enum class PerformanceStatus { Scheduled, Started, Cancelled };

/// Спектакль: постановка, показанная в конкретном зале в конкретное время.
class Performance {
public:
    Performance(const Production& production, const Hall& hall, const std::string& startTime);

    /// Начинает спектакль. Отменённый начать нельзя.
    void start();
    /// Отменяет спектакль. Начатый отменить нельзя.
    void cancel();
    bool hasFreeSeats() const;

    const Production& getProduction() const;
    Hall& getHall();
    const Hall& getHall() const;
    const std::string& getStartTime() const;
    PerformanceStatus getStatus() const;

private:
    Production production_;
    Hall hall_;
    std::string startTime_;
    PerformanceStatus status_;
};

/// Расписание спектаклей в рамках сезона.
class Schedule {
public:
    explicit Schedule(const Season& season);

    /// Добавляет спектакль. Бросает TheatreException, если он вне сезона или зал занят.
    void addPerformance(const Performance& performance);
    /// Спектакль на дату (ГГГГ-ММ-ДД). Бросает PerformanceNotFoundException, если его нет.
    const Performance& findByDate(const Date& date) const;
    /// Занят ли зал в то же время другим спектаклем.
    bool hasConflict(const Performance& candidate) const;

private:
    std::vector<Performance> performances_;
    Season season_;
};

/// Реквизит.
class Prop {
public:
    Prop(const std::string& name, int quantity, int condition);

    /// Использует реквизит: состояние ухудшается. Бросает TheatreException, если он негоден.
    void use();
    /// Списывает count единиц реквизита.
    void writeOff(int count);

    const std::string& getName() const;
    int getQuantity() const;
    int getCondition() const;

private:
    std::string name_;
    int quantity_;
    int condition_;
};

/// Склад реквизита.
class Warehouse {
public:
    Warehouse(const std::string& location, int capacity);

    /// Кладёт реквизит на склад. Бросает TheatreException, если не хватает места.
    void storeProp(const Prop& prop);
    /// Забирает реквизит со склада по названию.
    Prop takeProp(const std::string& name);
    /// Сколько единиц реквизита ещё поместится.
    int getFreeSpace() const;

private:
    std::string location_;
    int capacity_;
    std::vector<Prop> props_;
};