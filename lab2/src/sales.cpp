#include "sales.h"

#include <algorithm>
#include <cmath>
#include <ctime>
#include <iomanip>
#include <sstream>

#include "exceptions.h"

namespace {

const double kMaxPercent = 100.0;
const double kPercentDivisor = 100.0;
const double kMoneyEpsilon = 0.005;
const int kMoneyDecimals = 2;
const double kVatRate = 0.2;

const int kFullRefundDays = 7;
const int kHalfRefundDays = 1;
const double kHalfRefundRate = 0.5;

const double kLoyaltyDiscountPercent = 10.0;
const char* const kLoyaltyDiscountName = "Скидка постоянного зрителя";
const char* const kNoDiscountName = "Без скидки";
const char* const kNoExpiryDate = "9999-12-31";

const std::size_t kDateLength = 10;
const std::size_t kYearLength = 4;
const std::size_t kMonthOffset = 5;
const std::size_t kDayOffset = 8;
const std::size_t kPartLength = 2;
const int kTmYearBase = 1900;
const int kTmMonthBase = 1;
const int kNoonHour = 12;
const double kSecondsPerDay = 86400.0;

void requirePositive(double value, const std::string& message) {
    if (value <= 0) {
        throw TheatreException(message);
    }
}

std::string dayOf(const std::string& dateTime) {
    return dateTime.substr(0, kDateLength);
}

/// Время полудня указанной даты ГГГГ-ММ-ДД (полдень защищает от сдвигов из-за перехода на летнее время).
std::time_t toNoonTime(const Date& date) {
    std::tm parts{};
    parts.tm_year = std::stoi(date.substr(0, kYearLength)) - kTmYearBase;
    parts.tm_mon = std::stoi(date.substr(kMonthOffset, kPartLength)) - kTmMonthBase;
    parts.tm_mday = std::stoi(date.substr(kDayOffset, kPartLength));
    parts.tm_hour = kNoonHour;
    parts.tm_isdst = -1;
    return std::mktime(&parts);
}

/// Число дней от from до to (отрицательное, если to раньше from).
int daysBetween(const Date& from, const Date& to) {
    double seconds = std::difftime(toNoonTime(to), toNoonTime(from));
    return static_cast<int>(std::lround(seconds / kSecondsPerDay));
}

}  // namespace

// ----- Discount -----

Discount::Discount(const std::string& name, double percent, const Date& validUntil)
    : name_(name), percent_(percent), validUntil_(validUntil) {
    if (percent < 0 || percent > kMaxPercent) {
        throw TheatreException("Процент скидки должен быть от 0 до 100");
    }
}

double Discount::apply(double price) const {
    return price * (1.0 - percent_ / kPercentDivisor);
}

bool Discount::isValid(const Date& today) const {
    return today <= validUntil_;
}

const std::string& Discount::getName() const {
    return name_;
}

double Discount::getPercent() const {
    return percent_;
}

// ----- PromoCode -----

PromoCode::PromoCode(const std::string& code, const Discount& discount, int usageLimit)
    : code_(code), discount_(discount), usageLimit_(usageLimit), usedCount_(0) {
    requirePositive(usageLimit, "Лимит использований должен быть положительным");
}

const Discount& PromoCode::redeem(const Date& today) {
    if (isExhausted()) {
        throw InvalidPromoCodeException("Промокод исчерпан");
    }
    if (!discount_.isValid(today)) {
        throw InvalidPromoCodeException("Срок действия промокода истёк");
    }
    ++usedCount_;
    return discount_;
}

bool PromoCode::isExhausted() const {
    return usedCount_ >= usageLimit_;
}

const std::string& PromoCode::getCode() const {
    return code_;
}

int PromoCode::getUsedCount() const {
    return usedCount_;
}

// ----- Payment -----

Payment::Payment(int paymentId, double amount, const Spectator& payer)
    : paymentId_(paymentId), amount_(amount), status_(PaymentStatus::Pending), payer_(payer) {
    requirePositive(amount, "Сумма платежа должна быть положительной");
}

void Payment::process() {
    if (status_ == PaymentStatus::Completed) {
        throw TheatreException("Платёж уже проведён");
    }
    status_ = PaymentStatus::Completed;
}

bool Payment::isSuccessful() const {
    return status_ == PaymentStatus::Completed;
}

int Payment::getPaymentId() const {
    return paymentId_;
}

double Payment::getAmount() const {
    return amount_;
}

const Spectator& Payment::getPayer() const {
    return payer_;
}

// ----- Receipt -----

Receipt::Receipt(int number, const Payment& payment)
    : number_(number), payment_(payment), total_(payment.getAmount()) {
    if (!payment.isSuccessful()) {
        throw TheatreException("Чек можно выдать только по успешному платежу");
    }
}

std::string Receipt::generateText() const {
    std::ostringstream text;
    text << "Чек №" << number_ << ", платёж №" << payment_.getPaymentId() << ", итого "
         << std::fixed << std::setprecision(kMoneyDecimals) << total_;
    return text.str();
}

double Receipt::getTaxAmount() const {
    return total_ * kVatRate / (1.0 + kVatRate);
}

int Receipt::getNumber() const {
    return number_;
}

double Receipt::getTotal() const {
    return total_;
}

// ----- Ticket -----

Ticket::Ticket(int ticketId, const Performance& performance, const Seat& seat,
               const Spectator& spectator, double price)
    : ticketId_(ticketId),
      performance_(performance),
      seat_(seat),
      spectator_(spectator),
      price_(price),
      status_(TicketStatus::Valid) {
    requirePositive(price, "Цена билета должна быть положительной");
}

void Ticket::validate() const {
    if (status_ == TicketStatus::Cancelled) {
        throw InvalidTicketException("Билет отменён");
    }
    if (status_ == TicketStatus::Used) {
        throw InvalidTicketException("Билет уже использован");
    }
}

void Ticket::cancel() {
    validate();
    status_ = TicketStatus::Cancelled;
}

void Ticket::markUsed() {
    validate();
    status_ = TicketStatus::Used;
}

int Ticket::getTicketId() const {
    return ticketId_;
}

double Ticket::getPrice() const {
    return price_;
}

TicketStatus Ticket::getStatus() const {
    return status_;
}

const Performance& Ticket::getPerformance() const {
    return performance_;
}

const Seat& Ticket::getSeat() const {
    return seat_;
}

const Spectator& Ticket::getSpectator() const {
    return spectator_;
}

// ----- Booking -----

Booking::Booking(int bookingId, const std::vector<Ticket>& tickets, const Spectator& spectator,
                 const Date& expiresAt)
    : bookingId_(bookingId),
      tickets_(tickets),
      spectator_(spectator),
      expiresAt_(expiresAt),
      status_(BookingStatus::Pending) {
    if (tickets.empty()) {
        throw TheatreException("В брони должен быть хотя бы один билет");
    }
}

void Booking::confirm(Payment& payment, const Date& today) {
    if (status_ != BookingStatus::Pending) {
        throw TheatreException("Бронь уже подтверждена или отменена");
    }
    if (isExpired(today)) {
        throw TheatreException("Срок брони истёк");
    }
    if (std::fabs(payment.getAmount() - getTotalPrice()) > kMoneyEpsilon) {
        throw TheatreException("Сумма платежа не совпадает со стоимостью билетов");
    }
    payment.process();
    status_ = BookingStatus::Confirmed;
}

void Booking::cancel() {
    if (status_ == BookingStatus::Cancelled) {
        throw TheatreException("Бронь уже отменена");
    }
    for (Ticket& ticket : tickets_) {
        ticket.cancel();
    }
    status_ = BookingStatus::Cancelled;
}

bool Booking::isExpired(const Date& today) const {
    return status_ == BookingStatus::Pending && today > expiresAt_;
}

double Booking::getTotalPrice() const {
    double total = 0.0;
    for (const Ticket& ticket : tickets_) {
        total += ticket.getPrice();
    }
    return total;
}

BookingStatus Booking::getStatus() const {
    return status_;
}

// ----- Refund -----

Refund::Refund(const Ticket& ticket, int daysBeforePerformance)
    : ticket_(ticket),
      amount_(0.0),
      status_(RefundStatus::Pending) {
    amount_ = calculateAmount(daysBeforePerformance);
}

double Refund::calculateAmount(int daysBeforePerformance) const {
    double price = ticket_.getPrice();
    if (daysBeforePerformance >= kFullRefundDays) {
        return price;
    }
    if (daysBeforePerformance >= kHalfRefundDays) {
        return price * kHalfRefundRate;
    }
    return 0.0;
}

void Refund::approve() {
    if (status_ == RefundStatus::Approved) {
        throw TheatreException("Возврат уже одобрен");
    }
    if (ticket_.getStatus() != TicketStatus::Valid) {
        throw RefundNotAllowedException("Вернуть можно только действующий билет");
    }
    if (amount_ <= 0) {
        throw RefundNotAllowedException("Возврат невозможен: до спектакля осталось слишком мало времени");
    }
    status_ = RefundStatus::Approved;
}

double Refund::getAmount() const {
    return amount_;
}

RefundStatus Refund::getStatus() const {
    return status_;
}

// ----- Subscription -----

Subscription::Subscription(const Spectator& owner, const Date& validFrom, const Date& validTo,
                           int remainingVisits)
    : owner_(owner), validFrom_(validFrom), validTo_(validTo), remainingVisits_(remainingVisits) {
    if (validTo < validFrom) {
        throw TheatreException("Дата окончания абонемента раньше даты начала");
    }
    requirePositive(remainingVisits, "Число посещений должно быть положительным");
}

void Subscription::useVisit(const Date& today) {
    if (!isValid(today)) {
        throw SubscriptionExpiredException("Абонемент недействителен");
    }
    --remainingVisits_;
}

void Subscription::renew(const Date& newValidTo, int additionalVisits) {
    if (newValidTo <= validTo_) {
        throw TheatreException("Новая дата окончания должна быть позже текущей");
    }
    requirePositive(additionalVisits, "Число добавляемых посещений должно быть положительным");
    validTo_ = newValidTo;
    remainingVisits_ += additionalVisits;
}

bool Subscription::isValid(const Date& today) const {
    return remainingVisits_ > 0 && today >= validFrom_ && today <= validTo_;
}

int Subscription::getRemainingVisits() const {
    return remainingVisits_;
}

// ----- LoyaltyProgram -----

LoyaltyProgram::LoyaltyProgram(int pointsPerTicket, int bonusThreshold)
    : pointsPerTicket_(pointsPerTicket), bonusThreshold_(bonusThreshold) {
    requirePositive(pointsPerTicket, "Баллы за билет должны быть положительными");
    requirePositive(bonusThreshold, "Порог бонуса должен быть положительным");
}

void LoyaltyProgram::enroll(const Spectator& spectator) {
    bool added = members_.insert(spectator.getSpectatorId()).second;
    if (!added) {
        throw TheatreException("Зритель уже участвует в программе лояльности");
    }
}

void LoyaltyProgram::awardPoints(Spectator& spectator, const Ticket& ticket) const {
    if (!isMember(spectator)) {
        throw TheatreException("Зритель не участвует в программе лояльности");
    }
    if (ticket.getStatus() == TicketStatus::Cancelled) {
        throw InvalidTicketException("За отменённый билет баллы не начисляются");
    }
    spectator.addLoyaltyPoints(pointsPerTicket_);
}

Discount LoyaltyProgram::getDiscountFor(const Spectator& spectator) const {
    if (!isMember(spectator)) {
        throw TheatreException("Зритель не участвует в программе лояльности");
    }
    if (spectator.getLoyaltyPoints() >= bonusThreshold_) {
        return Discount(kLoyaltyDiscountName, kLoyaltyDiscountPercent, kNoExpiryDate);
    }
    return Discount(kNoDiscountName, 0.0, kNoExpiryDate);
}

bool LoyaltyProgram::isMember(const Spectator& spectator) const {
    return members_.count(spectator.getSpectatorId()) > 0;
}

// ----- TicketOffice -----

TicketOffice::TicketOffice(const Schedule& schedule, const Cashier& cashier)
    : schedule_(schedule), cashier_(cashier) {}

Ticket TicketOffice::sellTicket(Performance& performance, Seat& seat, Spectator& spectator,
                                double basePrice) {
    requirePositive(basePrice, "Базовая цена должна быть положительной");
    requireSellable(performance);
    double price = seat.getCategory().calculatePrice(basePrice);
    if (spectator.getBalance() < price) {
        throw InsufficientFundsException("Недостаточно средств для покупки билета");
    }
    seat.book();
    int ticketId = nextTicketId();
    Payment payment(ticketId, price, spectator);
    cashier_.processPayment(payment);
    spectator.pay(price);
    Ticket ticket(ticketId, performance, seat, spectator, price);
    soldTickets_.push_back(ticket);
    return ticket;
}

double TicketOffice::returnTicket(int ticketId, Seat& seat, Spectator& spectator,
                                  const Date& today) {
    Ticket& ticket = findTicket(ticketId);
    Date performanceDay = dayOf(ticket.getPerformance().getStartTime());
    Refund refund(ticket, daysBetween(today, performanceDay));
    refund.approve();
    ticket.cancel();
    seat.release();
    spectator.topUpBalance(refund.getAmount());
    return refund.getAmount();
}

double TicketOffice::getRevenue() const {
    double revenue = 0.0;
    for (const Ticket& ticket : soldTickets_) {
        if (ticket.getStatus() != TicketStatus::Cancelled) {
            revenue += ticket.getPrice();
        }
    }
    return revenue;
}

std::size_t TicketOffice::getSoldTicketsCount() const {
    return soldTickets_.size();
}

Schedule& TicketOffice::getSchedule() {
    return schedule_;
}

const Cashier& TicketOffice::getCashier() const {
    return cashier_;
}

void TicketOffice::requireSellable(const Performance& performance) const {
    if (performance.getStatus() == PerformanceStatus::Cancelled) {
        throw TheatreException("Спектакль отменён, продажа билетов невозможна");
    }
    schedule_.findByDate(dayOf(performance.getStartTime()));
}

int TicketOffice::nextTicketId() const {
    return static_cast<int>(soldTickets_.size()) + 1;
}

Ticket& TicketOffice::findTicket(int ticketId) {
    auto found = std::find_if(soldTickets_.begin(), soldTickets_.end(),
                              [ticketId](const Ticket& ticket) {
                                  return ticket.getTicketId() == ticketId;
                              });
    if (found == soldTickets_.end()) {
        throw InvalidTicketException("Билет с таким номером не найден");
    }
    return *found;
}