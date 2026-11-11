#pragma once
#include <string>


using Date = std::string;

class Role;
class Production;
class Cast;
class Seat;
class Ticket;
class Payment;
class Receipt;
class Refund;

/// Человек: базовый класс для сотрудников и зрителей.
class Person {
public:
    Person(const std::string& fullName, const Date& birthDate, const std::string& phone);
    virtual ~Person() = default;

    /// Полных лет на дату today.
    int getAge(const Date& today) const;
    /// Достиг ли человек совершеннолетия на дату today.
    bool isAdult(const Date& today) const;

    const std::string& getFullName() const;
    const Date& getBirthDate() const;

private:
    std::string fullName_;
    Date birthDate_;
    std::string phone_;
};

/// Сотрудник театра.
class Employee : public Person {
public:
    Employee(const std::string& fullName, const Date& birthDate, const std::string& phone,
             int employeeId, const std::string& position, const Date& hireDate);

    /// Стаж в полных годах на дату today.
    int calculateSeniority(const Date& today) const;
    /// Меняет должность на новую.
    void promote(const std::string& newPosition);

    int getEmployeeId() const;
    const std::string& getPosition() const;
    const Date& getHireDate() const;

private:
    int employeeId_;
    std::string position_;
    Date hireDate_;
};

/// Трудовой контракт сотрудника.
class Contract {
public:
    Contract(const Employee& employee, const Date& startDate, const Date& endDate);

    /// Действует ли контракт на дату today.
    bool isActive(const Date& today) const;
    /// Продлевает контракт. Бросает ContractExpiredException, если он уже истёк.
    void renew(const Date& newEndDate, const Date& today);
    /// Досрочно завершает контракт с даты today.
    void terminate(const Date& today);

    const Employee& getEmployee() const;
    const Date& getEndDate() const;

private:
    Employee employee_;
    Date startDate_;
    Date endDate_;
};

/// Зарплата сотрудника.
class Salary {
public:
    Salary(const Employee& employee, double baseAmount, double taxRate);

    /// Сумма к выдаче: (оклад + премия) за вычетом налога.
    double calculateNet() const;
    /// Добавляет премию. Сумма должна быть положительной.
    void addBonus(double amount);

    const Employee& getEmployee() const;

private:
    Employee employee_;
    double baseAmount_;
    double bonus_;
    double taxRate_;
};

/// Актёр.
class Actor : public Employee {
public:
    Actor(const Employee& employee, const std::string& stageName, int rank);

    /// Подходит ли актёр на роль по возрасту на дату today и свободен ли он.
    bool canPlay(const Role& role, const Date& today) const;
    /// Повышает разряд на единицу, но не выше максимального.
    void promoteRank();

    bool isAvailable() const;
    void setAvailable(bool available);
    int getRank() const;

private:
    std::string stageName_;
    int rank_;
    bool isAvailable_;
};

/// Режиссёр.
class Director : public Employee {
public:
    Director(const Employee& employee, const std::string& style, int awardsCount,
             int maxProductions);

    /// Берёт постановку в работу. Бросает TheatreException, если лимит исчерпан.
    void assignProduction(const Production& production);
    /// Утверждает состав. Бросает InvalidCastException, если состав неполный.
    void approveCast(const Cast& cast) const;

    int getActiveProductions() const;

private:
    std::string style_;
    int awardsCount_;
    int maxProductions_;
    int activeProductions_;
};

/// Билетёр.
class Usher : public Employee {
public:
    Usher(const Employee& employee, const std::string& hallZone, const std::string& shiftStart,
          const std::string& shiftEnd);

    /// Подсказка зрителю, как пройти к месту.
    std::string showToSeat(const Seat& seat) const;
    /// Проверяет билет на входе.
    bool checkTicket(const Ticket& ticket) const;

private:
    std::string hallZone_;
    std::string shiftStart_;
    std::string shiftEnd_;
};

/// Кассир.
class Cashier : public Employee {
public:
    Cashier(const Employee& employee, int cashierId, int windowNumber);

    /// Проводит платёж и увеличивает сумму в кассе.
    void processPayment(Payment& payment);
    /// Выдаёт чек по успешному платежу.
    Receipt issueReceipt(const Payment& payment) const;

    double getCashBalance() const;

private:
    int cashierId_;
    int windowNumber_;
    double cashBalance_;
};

/// Администратор театра.
class Administrator : public Employee {
public:
    Administrator(const Employee& employee, int accessLevel, const std::string& department);

    /// Одобряет возврат билета.
    void approveRefund(Refund& refund) const;
    /// Контракт можно одобрить, только если он действует на дату today.
    bool approveContract(const Contract& contract, const Date& today) const;

private:
    int accessLevel_;
    std::string department_;
};

/// Зритель.
class Spectator : public Person {
public:
    Spectator(const std::string& fullName, const Date& birthDate, const std::string& phone,
              int spectatorId, double balance);

    /// Пополняет баланс. Сумма должна быть положительной.
    void topUpBalance(double amount);
    /// Списывает сумму с баланса. Бросает InsufficientFundsException, если денег не хватает.
    void pay(double amount);
    /// Начисляет бонусные баллы.
    void addLoyaltyPoints(int points);

    int getSpectatorId() const;
    double getBalance() const;
    int getLoyaltyPoints() const;

private:
    int spectatorId_;
    double balance_;
    int loyaltyPoints_;
};