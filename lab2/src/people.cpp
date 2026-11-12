#include "people.h"

#include <algorithm>

#include "exceptions.h"
#include "repertoire.h"
#include "sales.h"
#include "venue.h"

namespace {

const int kAdultAge = 18;
const int kMinActorRank = 1;
const int kMaxActorRank = 5;
const int kRefundAccessLevel = 2;
const double kFullRate = 1.0;

const std::size_t kYearLength = 4;
const std::size_t kMonthDayOffset = 5;

int yearOf(const Date& date) {
    return std::stoi(date.substr(0, kYearLength));
}

std::string monthDayOf(const Date& date) {
    return date.substr(kMonthDayOffset);
}

/// Полных лет между двумя датами формата ГГГГ-ММ-ДД (не меньше нуля).
int fullYearsBetween(const Date& from, const Date& to) {
    int years = yearOf(to) - yearOf(from);
    if (monthDayOf(to) < monthDayOf(from)) {
        --years;
    }
    return std::max(0, years);
}

void requirePositive(double value, const std::string& message) {
    if (value <= 0) {
        throw TheatreException(message);
    }
}

}  // namespace

// ----- Person -----

Person::Person(const std::string& fullName, const Date& birthDate, const std::string& phone)
    : fullName_(fullName), birthDate_(birthDate), phone_(phone) {}

int Person::getAge(const Date& today) const {
    return fullYearsBetween(birthDate_, today);
}

bool Person::isAdult(const Date& today) const {
    return getAge(today) >= kAdultAge;
}

const std::string& Person::getFullName() const {
    return fullName_;
}

const Date& Person::getBirthDate() const {
    return birthDate_;
}

// ----- Employee -----

Employee::Employee(const std::string& fullName, const Date& birthDate, const std::string& phone,
                   int employeeId, const std::string& position, const Date& hireDate)
    : Person(fullName, birthDate, phone),
      employeeId_(employeeId),
      position_(position),
      hireDate_(hireDate) {}

int Employee::calculateSeniority(const Date& today) const {
    return fullYearsBetween(hireDate_, today);
}

void Employee::promote(const std::string& newPosition) {
    if (newPosition.empty()) {
        throw TheatreException("Должность не может быть пустой");
    }
    position_ = newPosition;
}

int Employee::getEmployeeId() const {
    return employeeId_;
}

const std::string& Employee::getPosition() const {
    return position_;
}

const Date& Employee::getHireDate() const {
    return hireDate_;
}

// ----- Contract -----

Contract::Contract(const Employee& employee, const Date& startDate, const Date& endDate)
    : employee_(employee), startDate_(startDate), endDate_(endDate) {
    if (endDate < startDate) {
        throw TheatreException("Дата окончания контракта раньше даты начала");
    }
}

bool Contract::isActive(const Date& today) const {
    return today >= startDate_ && today <= endDate_;
}

void Contract::renew(const Date& newEndDate, const Date& today) {
    if (today > endDate_) {
        throw ContractExpiredException("Контракт уже истёк, продление невозможно");
    }
    if (newEndDate <= endDate_) {
        throw TheatreException("Новая дата окончания должна быть позже текущей");
    }
    endDate_ = newEndDate;
}

void Contract::terminate(const Date& today) {
    endDate_ = today;
}

const Employee& Contract::getEmployee() const {
    return employee_;
}

const Date& Contract::getEndDate() const {
    return endDate_;
}

// ----- Salary -----

Salary::Salary(const Employee& employee, double baseAmount, double taxRate)
    : employee_(employee), baseAmount_(baseAmount), bonus_(0.0), taxRate_(taxRate) {
    if (baseAmount < 0 || taxRate < 0 || taxRate > kFullRate) {
        throw TheatreException("Некорректный оклад или ставка налога");
    }
}

double Salary::calculateNet() const {
    double gross = baseAmount_ + bonus_;
    return gross * (kFullRate - taxRate_);
}

void Salary::addBonus(double amount) {
    requirePositive(amount, "Премия должна быть положительной");
    bonus_ += amount;
}

const Employee& Salary::getEmployee() const {
    return employee_;
}

// ----- Actor -----

Actor::Actor(const Employee& employee, const std::string& stageName, int rank)
    : Employee(employee), stageName_(stageName), rank_(rank), isAvailable_(true) {
    if (rank < kMinActorRank || rank > kMaxActorRank) {
        throw TheatreException("Разряд актёра вне допустимого диапазона");
    }
}

bool Actor::canPlay(const Role& role, const Date& today) const {
    return isAvailable_ && role.isSuitableForAge(getAge(today));
}

void Actor::promoteRank() {
    if (rank_ < kMaxActorRank) {
        ++rank_;
    }
}

bool Actor::isAvailable() const {
    return isAvailable_;
}

void Actor::setAvailable(bool available) {
    isAvailable_ = available;
}

int Actor::getRank() const {
    return rank_;
}

// ----- Director -----

Director::Director(const Employee& employee, const std::string& style, int awardsCount,
                   int maxProductions)
    : Employee(employee),
      style_(style),
      awardsCount_(awardsCount),
      maxProductions_(maxProductions),
      activeProductions_(0) {
    if (maxProductions <= 0) {
        throw TheatreException("Лимит постановок должен быть положительным");
    }
}

void Director::assignProduction(const Production&) {
    if (activeProductions_ >= maxProductions_) {
        throw TheatreException("Режиссёр загружен: достигнут лимит постановок");
    }
    ++activeProductions_;
}

void Director::approveCast(const Cast& cast) const {
    if (!cast.isComplete()) {
        throw InvalidCastException("Состав неполный, утверждение невозможно");
    }
}

int Director::getActiveProductions() const {
    return activeProductions_;
}

// ----- Usher -----

Usher::Usher(const Employee& employee, const std::string& hallZone,
             const std::string& shiftStart, const std::string& shiftEnd)
    : Employee(employee), hallZone_(hallZone), shiftStart_(shiftStart), shiftEnd_(shiftEnd) {}

std::string Usher::showToSeat(const Seat& seat) const {
    return "Зона " + hallZone_ + ", место " + std::to_string(seat.getNumber());
}

bool Usher::checkTicket(const Ticket& ticket) const {
    try {
        ticket.validate();
        return true;
    } catch (const InvalidTicketException&) {
        return false;
    }
}

// ----- Cashier -----

Cashier::Cashier(const Employee& employee, int cashierId, int windowNumber)
    : Employee(employee), cashierId_(cashierId), windowNumber_(windowNumber), cashBalance_(0.0) {}

void Cashier::processPayment(Payment& payment) {
    payment.process();
    if (payment.isSuccessful()) {
        cashBalance_ += payment.getAmount();
    }
}

Receipt Cashier::issueReceipt(const Payment& payment) const {
    if (!payment.isSuccessful()) {
        throw TheatreException("Чек можно выдать только по успешному платежу");
    }
    return Receipt(payment.getPaymentId(), payment);
}

double Cashier::getCashBalance() const {
    return cashBalance_;
}

// ----- Administrator -----

Administrator::Administrator(const Employee& employee, int accessLevel,
                             const std::string& department)
    : Employee(employee), accessLevel_(accessLevel), department_(department) {}

void Administrator::approveRefund(Refund& refund) const {
    if (accessLevel_ < kRefundAccessLevel) {
        throw TheatreException("Недостаточно прав для одобрения возврата");
    }
    refund.approve();
}

bool Administrator::approveContract(const Contract& contract, const Date& today) const {
    return contract.isActive(today);
}

// ----- Spectator -----

Spectator::Spectator(const std::string& fullName, const Date& birthDate, const std::string& phone,
                     int spectatorId, double balance)
    : Person(fullName, birthDate, phone),
      spectatorId_(spectatorId),
      balance_(balance),
      loyaltyPoints_(0) {
    if (balance < 0) {
        throw TheatreException("Начальный баланс не может быть отрицательным");
    }
}

void Spectator::topUpBalance(double amount) {
    requirePositive(amount, "Сумма пополнения должна быть положительной");
    balance_ += amount;
}

void Spectator::pay(double amount) {
    requirePositive(amount, "Сумма оплаты должна быть положительной");
    if (balance_ < amount) {
        throw InsufficientFundsException("Недостаточно средств на балансе");
    }
    balance_ -= amount;
}

void Spectator::addLoyaltyPoints(int points) {
    requirePositive(points, "Количество баллов должно быть положительным");
    loyaltyPoints_ += points;
}

int Spectator::getSpectatorId() const {
    return spectatorId_;
}

double Spectator::getBalance() const {
    return balance_;
}

int Spectator::getLoyaltyPoints() const {
    return loyaltyPoints_;
}