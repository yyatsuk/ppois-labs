#include "venue.h"

#include <algorithm>

#include "exceptions.h"

namespace {

const int kMinutesPerHour = 60;
const int kMinCondition = 0;
const int kMaxCondition = 100;
const int kPropWearPerUse = 5;

const std::size_t kDateLength = 10;
const std::size_t kHourOffset = 11;
const std::size_t kMinuteOffset = 14;
const std::size_t kTimePartLength = 2;
const std::size_t kDateTimeLength = 16;

void requirePositive(double value, const std::string& message) {
    if (value <= 0) {
        throw TheatreException(message);
    }
}

void requireDateTime(const std::string& dateTime) {
    if (dateTime.size() != kDateTimeLength) {
        throw TheatreException("Дата и время должны быть в формате ГГГГ-ММ-ДД ЧЧ:ММ");
    }
}

std::string dayOf(const std::string& dateTime) {
    return dateTime.substr(0, kDateLength);
}

int minutesOfDay(const std::string& dateTime) {
    int hours = std::stoi(dateTime.substr(kHourOffset, kTimePartLength));
    int minutes = std::stoi(dateTime.substr(kMinuteOffset, kTimePartLength));
    return hours * kMinutesPerHour + minutes;
}

bool intervalsOverlap(const std::string& startA, int durationA, const std::string& startB,
                      int durationB) {
    if (dayOf(startA) != dayOf(startB)) {
        return false;
    }
    int beginA = minutesOfDay(startA);
    int beginB = minutesOfDay(startB);
    return beginA < beginB + durationB && beginB < beginA + durationA;
}

int durationOf(const Performance& performance) {
    return performance.getProduction().getPlay().getDurationMinutes();
}

}  // namespace

// ----- SeatCategory -----

SeatCategory::SeatCategory(const std::string& name, double priceMultiplier,
                           const std::string& description)
    : name_(name), priceMultiplier_(priceMultiplier), description_(description) {
    requirePositive(priceMultiplier, "Ценовой коэффициент должен быть положительным");
}

double SeatCategory::calculatePrice(double basePrice) const {
    return basePrice * priceMultiplier_;
}

const std::string& SeatCategory::getName() const {
    return name_;
}

// ----- Seat -----

Seat::Seat(int number, const SeatCategory& category)
    : number_(number), category_(category), isBooked_(false) {
    requirePositive(number, "Номер места должен быть положительным");
}

void Seat::book() {
    if (isBooked_) {
        throw SeatAlreadyBookedException("Место уже занято");
    }
    isBooked_ = true;
}

void Seat::release() {
    isBooked_ = false;
}

bool Seat::isAvailable() const {
    return !isBooked_;
}

int Seat::getNumber() const {
    return number_;
}

const SeatCategory& Seat::getCategory() const {
    return category_;
}

// ----- Row -----

Row::Row(int number, const std::string& sectionName)
    : number_(number), sectionName_(sectionName) {
    requirePositive(number, "Номер ряда должен быть положительным");
}

void Row::addSeat(const Seat& seat) {
    bool duplicate = std::any_of(seats_.begin(), seats_.end(), [&seat](const Seat& existing) {
        return existing.getNumber() == seat.getNumber();
    });
    if (duplicate) {
        throw TheatreException("Место с таким номером уже есть в ряду");
    }
    seats_.push_back(seat);
}

Seat& Row::findFreeSeat() {
    auto found = std::find_if(seats_.begin(), seats_.end(),
                              [](const Seat& seat) { return seat.isAvailable(); });
    if (found == seats_.end()) {
        throw TheatreException("В ряду нет свободных мест");
    }
    return *found;
}

int Row::getNumber() const {
    return number_;
}

std::vector<Seat>& Row::getSeats() {
    return seats_;
}

const std::vector<Seat>& Row::getSeats() const {
    return seats_;
}

// ----- Stage -----

Stage::Stage(double width, double depth, double maxLoadKg)
    : width_(width), depth_(depth), maxLoadKg_(maxLoadKg) {
    requirePositive(width, "Ширина сцены должна быть положительной");
    requirePositive(depth, "Глубина сцены должна быть положительной");
    requirePositive(maxLoadKg, "Допустимая нагрузка должна быть положительной");
}

bool Stage::canHost(const Decoration& decoration) const {
    return decoration.getWeightKg() <= maxLoadKg_;
}

bool Stage::checkDimensions(double requiredWidth, double requiredDepth) const {
    return width_ >= requiredWidth && depth_ >= requiredDepth;
}

// ----- Hall -----

Hall::Hall(const std::string& name, int capacity, const Stage& stage)
    : name_(name), capacity_(capacity), stage_(stage) {
    requirePositive(capacity, "Вместимость зала должна быть положительной");
}

void Hall::addRow(const Row& row) {
    int totalAfter = countSeats() + static_cast<int>(row.getSeats().size());
    if (totalAfter > capacity_) {
        throw HallOverCapacityException("Превышена вместимость зала");
    }
    rows_.push_back(row);
}

int Hall::getFreeSeatsCount() const {
    int freeSeats = 0;
    for (const Row& row : rows_) {
        const std::vector<Seat>& seats = row.getSeats();
        freeSeats += static_cast<int>(std::count_if(
            seats.begin(), seats.end(), [](const Seat& seat) { return seat.isAvailable(); }));
    }
    return freeSeats;
}

bool Hall::isOverCapacity() const {
    return countSeats() > capacity_;
}

int Hall::countSeats() const {
    int total = 0;
    for (const Row& row : rows_) {
        total += static_cast<int>(row.getSeats().size());
    }
    return total;
}

const std::string& Hall::getName() const {
    return name_;
}

int Hall::getCapacity() const {
    return capacity_;
}

std::vector<Row>& Hall::getRows() {
    return rows_;
}

const std::vector<Row>& Hall::getRows() const {
    return rows_;
}

const Stage& Hall::getStage() const {
    return stage_;
}

// ----- Rehearsal -----

Rehearsal::Rehearsal(const Production& production, const Stage& stage,
                     const std::string& dateTime, int durationMinutes)
    : production_(production),
      stage_(stage),
      dateTime_(dateTime),
      durationMinutes_(durationMinutes) {
    requireDateTime(dateTime);
    requirePositive(durationMinutes, "Длительность репетиции должна быть положительной");
}

void Rehearsal::reschedule(const std::string& newDateTime,
                           const std::vector<Rehearsal>& scheduled) {
    requireDateTime(newDateTime);
    for (const Rehearsal& other : scheduled) {
        if (&other != this && intervalsOverlap(newDateTime, durationMinutes_, other.dateTime_,
                                               other.durationMinutes_)) {
            throw RehearsalConflictException("Репетиция пересекается с другой");
        }
    }
    dateTime_ = newDateTime;
}

bool Rehearsal::conflictsWith(const Rehearsal& other) const {
    return intervalsOverlap(dateTime_, durationMinutes_, other.dateTime_, other.durationMinutes_);
}

const std::string& Rehearsal::getDateTime() const {
    return dateTime_;
}

// ----- Performance -----

Performance::Performance(const Production& production, const Hall& hall,
                         const std::string& startTime)
    : production_(production),
      hall_(hall),
      startTime_(startTime),
      status_(PerformanceStatus::Scheduled) {
    requireDateTime(startTime);
}

void Performance::start() {
    if (status_ == PerformanceStatus::Cancelled) {
        throw TheatreException("Отменённый спектакль нельзя начать");
    }
    status_ = PerformanceStatus::Started;
}

void Performance::cancel() {
    if (status_ == PerformanceStatus::Started) {
        throw TheatreException("Начатый спектакль нельзя отменить");
    }
    status_ = PerformanceStatus::Cancelled;
}

bool Performance::hasFreeSeats() const {
    return hall_.getFreeSeatsCount() > 0;
}

const Production& Performance::getProduction() const {
    return production_;
}

Hall& Performance::getHall() {
    return hall_;
}

const Hall& Performance::getHall() const {
    return hall_;
}

const std::string& Performance::getStartTime() const {
    return startTime_;
}

PerformanceStatus Performance::getStatus() const {
    return status_;
}

// ----- Schedule -----

Schedule::Schedule(const Season& season) : season_(season) {}

void Schedule::addPerformance(const Performance& performance) {
    if (!season_.isActive(dayOf(performance.getStartTime()))) {
        throw TheatreException("Спектакль выходит за рамки сезона");
    }
    if (hasConflict(performance)) {
        throw TheatreException("Зал в это время занят другим спектаклем");
    }
    performances_.push_back(performance);
}

const Performance& Schedule::findByDate(const Date& date) const {
    for (const Performance& performance : performances_) {
        if (dayOf(performance.getStartTime()) == date) {
            return performance;
        }
    }
    throw PerformanceNotFoundException("На эту дату спектаклей нет");
}

bool Schedule::hasConflict(const Performance& candidate) const {
    return std::any_of(performances_.begin(), performances_.end(),
                       [&candidate](const Performance& existing) {
                           return existing.getHall().getName() == candidate.getHall().getName() &&
                                  intervalsOverlap(existing.getStartTime(), durationOf(existing),
                                                   candidate.getStartTime(),
                                                   durationOf(candidate));
                       });
}

// ----- Prop -----

Prop::Prop(const std::string& name, int quantity, int condition)
    : name_(name), quantity_(quantity), condition_(condition) {
    requirePositive(quantity, "Количество реквизита должно быть положительным");
    if (condition < kMinCondition || condition > kMaxCondition) {
        throw TheatreException("Состояние реквизита должно быть от 0 до 100");
    }
}

void Prop::use() {
    if (condition_ <= kMinCondition) {
        throw TheatreException("Реквизит непригоден к использованию");
    }
    condition_ = std::max(kMinCondition, condition_ - kPropWearPerUse);
}

void Prop::writeOff(int count) {
    requirePositive(count, "Количество для списания должно быть положительным");
    if (count > quantity_) {
        throw TheatreException("Нельзя списать больше, чем есть");
    }
    quantity_ -= count;
}

const std::string& Prop::getName() const {
    return name_;
}

int Prop::getQuantity() const {
    return quantity_;
}

int Prop::getCondition() const {
    return condition_;
}

// ----- Warehouse -----

Warehouse::Warehouse(const std::string& location, int capacity)
    : location_(location), capacity_(capacity) {
    requirePositive(capacity, "Вместимость склада должна быть положительной");
}

void Warehouse::storeProp(const Prop& prop) {
    if (prop.getQuantity() > getFreeSpace()) {
        throw TheatreException("На складе недостаточно места");
    }
    props_.push_back(prop);
}

Prop Warehouse::takeProp(const std::string& name) {
    auto found = std::find_if(props_.begin(), props_.end(),
                              [&name](const Prop& prop) { return prop.getName() == name; });
    if (found == props_.end()) {
        throw TheatreException("Такого реквизита на складе нет");
    }
    Prop taken = *found;
    props_.erase(found);
    return taken;
}

int Warehouse::getFreeSpace() const {
    int occupied = 0;
    for (const Prop& prop : props_) {
        occupied += prop.getQuantity();
    }
    return capacity_ - occupied;
}