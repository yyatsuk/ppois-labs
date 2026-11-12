#pragma once

#include <set>
#include <string>
#include <vector>

#include "people.h"
#include "venue.h"

/// Скидка в процентах с ограниченным сроком действия.
class Discount {
public:
    Discount(const std::string& name, double percent, const Date& validUntil);

    /// Цена после применения скидки.
    double apply(double price) const;
    /// Действует ли скидка на дату today.
    bool isValid(const Date& today) const;

    const std::string& getName() const;
    double getPercent() const;

private:
    std::string name_;
    double percent_;
    Date validUntil_;
};

/// Промокод, дающий скидку ограниченное число раз.
class PromoCode {
public:
    PromoCode(const std::string& code, const Discount& discount, int usageLimit);

    /// Использует промокод. Бросает InvalidPromoCodeException, если он исчерпан или просрочен.
    const Discount& redeem(const Date& today);
    bool isExhausted() const;

    const std::string& getCode() const;
    int getUsedCount() const;

private:
    std::string code_;
    Discount discount_;
    int usageLimit_;
    int usedCount_;
};

/// Состояние платежа.
enum class PaymentStatus { Pending, Completed };

/// Платёж зрителя.
class Payment {
public:
    Payment(int paymentId, double amount, const Spectator& payer);

    /// Проводит платёж. Бросает TheatreException, если он уже проведён.
    void process();
    bool isSuccessful() const;

    int getPaymentId() const;
    double getAmount() const;
    const Spectator& getPayer() const;

private:
    int paymentId_;
    double amount_;
    PaymentStatus status_;
    Spectator payer_;
};

/// Чек по успешному платежу.
class Receipt {
public:
    Receipt(int number, const Payment& payment);

    /// Текст чека для печати.
    std::string generateText() const;
    /// Сумма НДС, включённая в итог.
    double getTaxAmount() const;

    int getNumber() const;
    double getTotal() const;

private:
    int number_;
    Payment payment_;
    double total_;
};

/// Состояние билета.
enum class TicketStatus { Valid, Used, Cancelled };

/// Билет на спектакль.
class Ticket {
public:
    Ticket(int ticketId, const Performance& performance, const Seat& seat,
           const Spectator& spectator, double price);

    /// Проверяет билет. Бросает InvalidTicketException, если он отменён или использован.
    void validate() const;
    /// Отменяет действующий билет.
    void cancel();
    /// Отмечает билет использованным (проход в зал).
    void markUsed();

    int getTicketId() const;
    double getPrice() const;
    TicketStatus getStatus() const;
    const Performance& getPerformance() const;
    const Seat& getSeat() const;
    const Spectator& getSpectator() const;

private:
    int ticketId_;
    Performance performance_;
    Seat seat_;
    Spectator spectator_;
    double price_;
    TicketStatus status_;
};

/// Состояние брони.
enum class BookingStatus { Pending, Confirmed, Cancelled };

/// Бронь нескольких билетов с ограниченным сроком оплаты.
class Booking {
public:
    Booking(int bookingId, const std::vector<Ticket>& tickets, const Spectator& spectator,
            const Date& expiresAt);

    /// Подтверждает бронь оплатой. Бросает TheatreException, если бронь просрочена
    /// или сумма платежа не совпадает с суммой билетов.
    void confirm(Payment& payment, const Date& today);
    /// Отменяет бронь вместе со всеми билетами.
    void cancel();
    /// Истёк ли срок неоплаченной брони на дату today.
    bool isExpired(const Date& today) const;

    double getTotalPrice() const;
    BookingStatus getStatus() const;

private:
    int bookingId_;
    std::vector<Ticket> tickets_;
    Spectator spectator_;
    Date expiresAt_;
    BookingStatus status_;
};

/// Состояние возврата.
enum class RefundStatus { Pending, Approved };

/// Возврат денег за билет.
class Refund {
public:
    /// daysBeforePerformance: за сколько дней до спектакля оформляется возврат.
    Refund(const Ticket& ticket, int daysBeforePerformance);

    /// Сумма возврата: полная, половина или ноль, в зависимости от срока.
    double calculateAmount(int daysBeforePerformance) const;
    /// Одобряет возврат. Бросает RefundNotAllowedException, если возврат невозможен.
    void approve();

    double getAmount() const;
    RefundStatus getStatus() const;

private:
    Ticket ticket_;
    double amount_;
    RefundStatus status_;
};

/// Абонемент зрителя.
class Subscription {
public:
    Subscription(const Spectator& owner, const Date& validFrom, const Date& validTo,
                 int remainingVisits);

    /// Списывает одно посещение. Бросает SubscriptionExpiredException, если абонемент недействителен.
    void useVisit(const Date& today);
    /// Продлевает абонемент и добавляет посещения.
    void renew(const Date& newValidTo, int additionalVisits);
    bool isValid(const Date& today) const;

    int getRemainingVisits() const;

private:
    Spectator owner_;
    Date validFrom_;
    Date validTo_;
    int remainingVisits_;
};

/// Программа лояльности: баллы за билеты и скидка постоянным зрителям.
class LoyaltyProgram {
public:
    LoyaltyProgram(int pointsPerTicket, int bonusThreshold);

    /// Регистрирует зрителя в программе. Бросает TheatreException при повторной регистрации.
    void enroll(const Spectator& spectator);
    /// Начисляет баллы за билет. Зритель должен быть участником, билет не отменён.
    void awardPoints(Spectator& spectator, const Ticket& ticket) const;
    /// Скидка зрителя: бонусная, если накоплено достаточно баллов, иначе нулевая.
    Discount getDiscountFor(const Spectator& spectator) const;
    bool isMember(const Spectator& spectator) const;

private:
    int pointsPerTicket_;
    int bonusThreshold_;
    std::set<int> members_;
};

/// Билетная касса: продажа и возврат билетов.
class TicketOffice {
public:
    TicketOffice(const Schedule& schedule, const Cashier& cashier);

    /// Продаёт билет на место спектакля. Бросает SeatAlreadyBookedException,
    /// InsufficientFundsException, PerformanceNotFoundException или TheatreException.
    Ticket sellTicket(Performance& performance, Seat& seat, Spectator& spectator,
                      double basePrice);
    /// Возвращает билет, освобождает место и возвращает деньги. Возвращает сумму возврата.
    double returnTicket(int ticketId, Seat& seat, Spectator& spectator, const Date& today);
    /// Выручка по неотменённым билетам.
    double getRevenue() const;

    std::size_t getSoldTicketsCount() const;
    Schedule& getSchedule();
    const Cashier& getCashier() const;

private:
    void requireSellable(const Performance& performance) const;
    int nextTicketId() const;
    Ticket& findTicket(int ticketId);

    Schedule schedule_;
    Cashier cashier_;
    std::vector<Ticket> soldTickets_;
};