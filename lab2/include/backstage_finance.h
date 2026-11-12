#pragma once

#include <string>
#include <vector>

#include "sales.h"

/// Костюмерная: хранит костюмы и выдаёт их под роли.
class CostumeDepartment {
public:
    CostumeDepartment(const std::string& headName, double budget);

    /// Покупает костюм за счёт бюджета. Бросает InsufficientFundsException, если денег не хватает.
    void addCostume(const Costume& costume);
    /// Выдаёт пригодный костюм для роли. Бросает TheatreException, если подходящего нет.
    Costume issueCostume(const Role& role);

    std::size_t getCostumesCount() const;
    double getBudget() const;

private:
    std::vector<Costume> costumes_;
    std::string headName_;
    double budget_;
};

/// Система освещения.
class LightingSystem {
public:
    explicit LightingSystem(int lampsCount);

    /// Включает световую сцену. Бросает TheatreException, если система выключена.
    void setScene(const std::string& scene);
    /// Включает или выключает систему; при выключении текущая сцена сбрасывается.
    void toggle();

    bool isOn() const;
    int getLampsCount() const;
    const std::string& getCurrentScene() const;

private:
    int lampsCount_;
    std::string currentScene_;
    bool isOn_;
};

/// Система звука.
class SoundSystem {
public:
    explicit SoundSystem(int microphonesCount);

    /// Устанавливает громкость. Бросает TheatreException, если звук выключен или значение вне 0..100.
    void setVolume(int level);
    /// Выдаёт актёру микрофон и возвращает его номер. Бросает TheatreException,
    /// если микрофоны закончились или у актёра он уже есть.
    int assignMicrophone(const Actor& actor);
    void setMuted(bool muted);

    int getVolumeLevel() const;
    bool isMuted() const;

private:
    int microphonesCount_;
    int volumeLevel_;
    bool isMuted_;
    std::vector<std::string> assignedActors_;
};

/// Статья расходов.
class Expense {
public:
    Expense(const std::string& description, double amount, const std::string& category,
            const Date& date);

    /// Больше ли расход указанной суммы.
    bool isLargerThan(double threshold) const;
    /// Попадает ли расход в период [from, to].
    bool isInPeriod(const Date& from, const Date& to) const;

    double getAmount() const;
    const std::string& getDescription() const;
    const std::string& getCategory() const;
    const Date& getDate() const;

private:
    std::string description_;
    double amount_;
    std::string category_;
    Date date_;
};

/// Бюджет театра на год.
class Budget {
public:
    Budget(int year, double totalAmount);

    /// Записывает расход. Перерасход допускается: его показывает isExceeded.
    void addExpense(const Expense& expense);
    /// Пополняет бюджет (например, взносом спонсора).
    void addFunds(double amount);
    /// Сколько денег осталось (может быть отрицательным при перерасходе).
    double getRemaining() const;
    bool isExceeded() const;

    const std::vector<Expense>& getExpenses() const;

private:
    int year_;
    double totalAmount_;
    std::vector<Expense> expenses_;
};

/// Спонсор театра.
class Sponsor {
public:
    Sponsor(const std::string& name, double contributionAmount, const Date& contractDate);

    /// Перечисляет взнос в бюджет. Бросает TheatreException, если договор не действует.
    void donate(Budget& budget, const Date& today) const;
    /// Действует ли договор на дату today (один календарный год со дня заключения).
    bool isActive(const Date& today) const;

private:
    std::string name_;
    double contributionAmount_;
    Date contractDate_;
};

/// Ведомость зарплат за месяц ("ГГГГ-ММ").
class Payroll {
public:
    explicit Payroll(const std::string& month);

    /// Добавляет зарплату в ведомость. Нельзя менять уже выплаченную ведомость.
    void addSalary(const Salary& salary);
    /// Сумма к выплате по всем сотрудникам.
    double calculateTotal() const;
    /// Выплачивает ведомость из бюджета. Бросает InsufficientFundsException, если денег не хватает.
    void payAll(Budget& budget);

    bool isPaid() const;

private:
    std::string month_;
    std::vector<Salary> salaries_;
    bool isPaid_;
};

/// Финансовый отчёт за период.
class FinancialReport {
public:
    FinancialReport(const Date& periodStart, const Date& periodEnd, const Budget& budget);

    /// Прибыль за период: выручка минус расходы периода.
    double calculateProfit(double revenue) const;
    /// Краткая текстовая сводка.
    std::string generateSummary(double revenue) const;

private:
    Date periodStart_;
    Date periodEnd_;
    Budget budget_;
};

/// Отзыв зрителя о спектакле.
class Review {
public:
    Review(const Spectator& author, const Performance& performance, int rating,
           const std::string& text);

    /// Проверяет оценку (1..5) и текст. Бросает TheatreException при ошибке.
    void validate() const;
    /// Положительный ли отзыв.
    bool isPositive() const;

    int getRating() const;

private:
    Spectator author_;
    Performance performance_;
    int rating_;
    std::string text_;
};

/// Театр: объединяет зал, кассу, бюджет и планирование сезона.
class Theatre {
public:
    Theatre(const std::string& name, const Hall& hall, const Season& season,
            const Cashier& cashier, const Budget& budget);

    /// Добавляет постановку в план сезона.
    void planSeason(const Production& production);
    /// Ставит спектакль в расписание. Бросает TheatreException, если он идёт в другом зале.
    void registerPerformance(const Performance& performance);
    /// Выручка от продажи билетов.
    double getTotalRevenue() const;

    TicketOffice& getTicketOffice();
    Budget& getBudget();
    const Hall& getHall() const;
    const std::string& getName() const;

private:
    std::string name_;
    Hall hall_;
    Season season_;
    TicketOffice ticketOffice_;
    Budget budget_;
};