#include <algorithm>
#include <iomanip>
#include <iostream>
#include <set>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>

#ifdef _WIN32
#include <windows.h>
#endif

#include "backstage_finance.h"
#include "exceptions.h"

/// Пункты главного меню. Значения совпадают с номерами, которые вводит пользователь.
enum class MenuItem {
    Exit = 0,
    ShowPerformances,
    BuyTicket,
    ReturnTicket,
    TopUpBalance,
    EnrollLoyalty,
    LeaveReview,
    ShowRevenue,
    PayPayroll,
    ShowReport
};

/// Консольное меню театра. С предметной областью работает только через её классы.
class TheatreMenu {
public:
    TheatreMenu();

    /// Главный цикл: показывает меню и выполняет действия до выбора выхода или конца ввода.
    void run();

private:
    /// Запись о проданном билете: по ней при возврате находится место в зале.
    struct Purchase {
        int ticketId;
        std::size_t performanceIndex;
        int rowNumber;
        int seatNumber;
        std::size_t spectatorIndex;
    };

    /// Ряд и место, выбранные пользователем.
    struct SeatChoice {
        int rowNumber;
        int seatNumber;
    };

    void seedData();
    void addPerformance(const Production& production, const std::string& startTime);

    void printMenu();
    bool executeSafely();
    bool handleChoice(MenuItem item);

    void showPerformances();
    void showSpectators();
    void showFreeSeats(const Performance& performance);
    std::size_t chooseIndex(const std::string& prompt, std::size_t count);
    SeatChoice readSeatChoice(const Performance& performance);
    double calculateBasePrice(const Spectator& spectator);
    double applyPromoCode(const std::string& code, double price);
    const Purchase& findPurchase(int ticketId) const;

    void buyTicket();
    void returnTicket();
    void topUpBalance();
    void enrollLoyalty();
    void leaveReview();
    void showRevenue();
    void payPayroll();
    void showReport();

    Theatre theatre_;
    LoyaltyProgram loyalty_;
    PromoCode promoCode_;
    std::vector<Performance> performances_;
    std::vector<Spectator> spectators_;
    std::vector<Employee> staff_;
    std::vector<Purchase> purchases_;
    std::vector<Review> reviews_;
    std::set<std::string> paidMonths_;
};

namespace {

const int kMoneyDecimals = 2;
const char* const kInputClosedMessage = "Ввод завершён";
const char* const kIntegerMessage = "Введите целое число";
const char* const kNumberMessage = "Введите число";

void print(const std::string& text) {
    std::cout << text << '\n';
}

std::string formatMoney(double amount) {
    std::ostringstream text;
    text << std::fixed << std::setprecision(kMoneyDecimals) << amount;
    return text.str();
}

std::string readLine(const std::string& prompt) {
    std::cout << prompt << ": ";
    std::string line;
    if (!std::getline(std::cin, line)) {
        throw TheatreException(kInputClosedMessage);
    }
    return line;
}

int readInt(const std::string& prompt) {
    std::string line = readLine(prompt);
    std::size_t parsedLength = 0;
    int value = 0;
    try {
        value = std::stoi(line, &parsedLength);
    } catch (const std::logic_error&) {
        throw TheatreException(kIntegerMessage);
    }
    if (parsedLength != line.size()) {
        throw TheatreException(kIntegerMessage);
    }
    return value;
}

double readDouble(const std::string& prompt) {
    std::string line = readLine(prompt);
    std::size_t parsedLength = 0;
    double value = 0.0;
    try {
        value = std::stod(line, &parsedLength);
    } catch (const std::logic_error&) {
        throw TheatreException(kNumberMessage);
    }
    if (parsedLength != line.size()) {
        throw TheatreException(kNumberMessage);
    }
    return value;
}

const char* const kTheatreName = "Драматический театр";
const char* const kSeasonName = "Сезон 2026/27";
const Date kSeasonStart = "2026-09-01";
const Date kSeasonEnd = "2027-06-30";
const Date kToday = "2026-11-12";
const Date kFirstPremiere = "2026-10-01";
const Date kSecondPremiere = "2026-11-01";
const char* const kFirstShow = "2026-11-14 19:00";
const char* const kSecondShow = "2026-12-10 19:00";

const int kBudgetYear = 2026;
const double kInitialBudget = 50000.0;
const double kBasePrice = 100.0;
const double kBaseSalary = 1500.0;
const double kSalaryTaxRate = 0.13;
const double kSpectatorBalance = 500.0;

const int kHallCapacity = 100;
const int kSeatsPerRow = 5;
const double kStageWidth = 20.0;
const double kStageDepth = 10.0;
const double kStageMaxLoadKg = 5000.0;
const double kParterMultiplier = 1.5;
const double kBalconyMultiplier = 1.0;

const int kPlayMinutes = 120;
const int kGenreMinAge = 12;
const int kDirectorAwards = 2;
const int kMaxProductions = 3;
const int kDirectorId = 1;
const int kCashierId = 2;
const int kActorId = 3;
const int kCashierWindow = 1;

const int kPointsPerTicket = 10;
const int kBonusThreshold = 20;
const char* const kPromoCodeText = "PREMIERE10";
const double kPromoPercent = 10.0;
const Date kPromoValidUntil = "2026-12-31";
const int kPromoUsageLimit = 5;

const char* const kPerformancePrompt = "Номер спектакля";
const char* const kSpectatorPrompt = "Номер зрителя";

struct MenuEntry {
    MenuItem item;
    const char* title;
};

const MenuEntry kMenuEntries[] = {
    {MenuItem::ShowPerformances, "Показать спектакли"},
    {MenuItem::BuyTicket, "Купить билет"},
    {MenuItem::ReturnTicket, "Вернуть билет"},
    {MenuItem::TopUpBalance, "Пополнить баланс зрителя"},
    {MenuItem::EnrollLoyalty, "Вступить в программу лояльности"},
    {MenuItem::LeaveReview, "Оставить отзыв"},
    {MenuItem::ShowRevenue, "Показать выручку и бюджет"},
    {MenuItem::PayPayroll, "Выплатить зарплаты за месяц"},
    {MenuItem::ShowReport, "Финансовый отчёт"},
    {MenuItem::Exit, "Выход"},
};

Row buildRow(int number, const std::string& section, const SeatCategory& category) {
    Row row(number, section);
    for (int seatNumber = 1; seatNumber <= kSeatsPerRow; ++seatNumber) {
        row.addSeat(Seat(seatNumber, category));
    }
    return row;
}

Hall buildHall() {
    Hall hall("Большой зал", kHallCapacity, Stage(kStageWidth, kStageDepth, kStageMaxLoadKg));
    SeatCategory parter("Партер", kParterMultiplier, "Центральные места");
    SeatCategory balcony("Балкон", kBalconyMultiplier, "Верхний ярус");
    hall.addRow(buildRow(1, "Партер", parter));
    hall.addRow(buildRow(2, "Балкон", balcony));
    return hall;
}

Employee buildEmployee(int id, const std::string& name, const std::string& position) {
    return Employee(name, "1985-04-12", "+375291234567", id, position, "2015-09-01");
}

Cashier buildCashier() {
    return Cashier(buildEmployee(kCashierId, "Петрова Анна Сергеевна", "Кассир"), kCashierId,
                   kCashierWindow);
}

Production buildProduction(const std::string& title, const Date& premiere) {
    Playwright author("А. П. Чехов", "Россия");
    Genre genre("Драма", "Классическая драма", kGenreMinAge);
    Play play(title, kPlayMinutes, genre, author);
    Employee directorBase = buildEmployee(kDirectorId, "Иванов Иван Петрович", "Режиссёр");
    Director director(directorBase, "реализм", kDirectorAwards, kMaxProductions);
    Production production(play, director, premiere);
    production.approve();
    return production;
}

Row& findRow(Hall& hall, int number) {
    std::vector<Row>& rows = hall.getRows();
    auto found = std::find_if(rows.begin(), rows.end(),
                              [number](const Row& row) { return row.getNumber() == number; });
    if (found == rows.end()) {
        throw TheatreException("Такого ряда нет");
    }
    return *found;
}

Seat& findSeat(Hall& hall, int rowNumber, int seatNumber) {
    std::vector<Seat>& seats = findRow(hall, rowNumber).getSeats();
    auto found = std::find_if(seats.begin(), seats.end(),
                              [seatNumber](const Seat& seat) { return seat.getNumber() == seatNumber; });
    if (found == seats.end()) {
        throw TheatreException("Такого места нет");
    }
    return *found;
}

}  // namespace

TheatreMenu::TheatreMenu()
    : theatre_(kTheatreName, buildHall(), Season(kSeasonName, kSeasonStart, kSeasonEnd),
               buildCashier(), Budget(kBudgetYear, kInitialBudget)),
      loyalty_(kPointsPerTicket, kBonusThreshold),
      promoCode_(kPromoCodeText, Discount("Премьерная скидка", kPromoPercent, kPromoValidUntil),
                 kPromoUsageLimit) {
    seedData();
}

void TheatreMenu::run() {
    bool running = true;
    while (running && std::cin) {
        printMenu();
        running = executeSafely();
    }
}

void TheatreMenu::seedData() {
    Production firstProduction = buildProduction("Чайка", kFirstPremiere);
    Production secondProduction = buildProduction("Вишнёвый сад", kSecondPremiere);
    theatre_.planSeason(firstProduction);
    theatre_.planSeason(secondProduction);
    addPerformance(firstProduction, kFirstShow);
    addPerformance(secondProduction, kSecondShow);
    spectators_.push_back(Spectator("Сидоров Пётр", "2000-01-01", "+375291112233", 1, kSpectatorBalance));
    spectators_.push_back(Spectator("Кузнецова Мария", "1998-06-15", "+375294445566", 2, kSpectatorBalance));
    staff_.push_back(buildEmployee(kDirectorId, "Иванов Иван Петрович", "Режиссёр"));
    staff_.push_back(buildEmployee(kCashierId, "Петрова Анна Сергеевна", "Кассир"));
    staff_.push_back(buildEmployee(kActorId, "Смирнов Олег Игоревич", "Актёр"));
}

void TheatreMenu::addPerformance(const Production& production, const std::string& startTime) {
    Performance performance(production, buildHall(), startTime);
    theatre_.registerPerformance(performance);
    performances_.push_back(performance);
}

void TheatreMenu::printMenu() {
    print(std::string("\n=== ") + kTheatreName + " ===");
    for (const MenuEntry& entry : kMenuEntries) {
        print(std::to_string(static_cast<int>(entry.item)) + ". " + entry.title);
    }
}

bool TheatreMenu::executeSafely() {
    try {
        int choice = readInt("Выберите пункт");
        return handleChoice(static_cast<MenuItem>(choice));
    } catch (const TheatreException& error) {
        print(std::string("Ошибка: ") + error.what());
        return true;
    }
}

bool TheatreMenu::handleChoice(MenuItem item) {
    switch (item) {
        case MenuItem::ShowPerformances: showPerformances(); break;
        case MenuItem::BuyTicket: buyTicket(); break;
        case MenuItem::ReturnTicket: returnTicket(); break;
        case MenuItem::TopUpBalance: topUpBalance(); break;
        case MenuItem::EnrollLoyalty: enrollLoyalty(); break;
        case MenuItem::LeaveReview: leaveReview(); break;
        case MenuItem::ShowRevenue: showRevenue(); break;
        case MenuItem::PayPayroll: payPayroll(); break;
        case MenuItem::ShowReport: showReport(); break;
        case MenuItem::Exit: return false;
        default: print("Такого пункта нет");
    }
    return true;
}

void TheatreMenu::showPerformances() {
    for (std::size_t index = 0; index < performances_.size(); ++index) {
        const Performance& performance = performances_[index];
        const std::string& title = performance.getProduction().getPlay().getTitle();
        int freeSeats = performance.getHall().getFreeSeatsCount();
        print(std::to_string(index + 1) + ". " + title + ", " + performance.getStartTime() +
                       ", свободных мест: " + std::to_string(freeSeats));
    }
}

void TheatreMenu::showSpectators() {
    for (std::size_t index = 0; index < spectators_.size(); ++index) {
        const Spectator& spectator = spectators_[index];
        print(std::to_string(index + 1) + ". " + spectator.getFullName() + ", баланс: " +
                       formatMoney(spectator.getBalance()));
    }
}

void TheatreMenu::showFreeSeats(const Performance& performance) {
    for (const Row& row : performance.getHall().getRows()) {
        std::string line = "Ряд " + std::to_string(row.getNumber()) + ", свободные места:";
        for (const Seat& seat : row.getSeats()) {
            if (seat.isAvailable()) {
                line += " " + std::to_string(seat.getNumber());
            }
        }
        print(line);
    }
}

std::size_t TheatreMenu::chooseIndex(const std::string& prompt, std::size_t count) {
    int number = readInt(prompt);
    if (number < 1 || number > static_cast<int>(count)) {
        throw TheatreException("Нет элемента с таким номером");
    }
    return static_cast<std::size_t>(number) - 1;
}

TheatreMenu::SeatChoice TheatreMenu::readSeatChoice(const Performance& performance) {
    showFreeSeats(performance);
    int rowNumber = readInt("Ряд");
    int seatNumber = readInt("Место");
    return SeatChoice{rowNumber, seatNumber};
}

double TheatreMenu::calculateBasePrice(const Spectator& spectator) {
    double price = kBasePrice;
    if (loyalty_.isMember(spectator)) {
        price = loyalty_.getDiscountFor(spectator).apply(price);
    }
    std::string code = readLine("Промокод (Enter, чтобы пропустить)");
    if (!code.empty()) {
        price = applyPromoCode(code, price);
    }
    return price;
}

double TheatreMenu::applyPromoCode(const std::string& code, double price) {
    if (code != promoCode_.getCode()) {
        throw InvalidPromoCodeException("Неверный промокод");
    }
    return promoCode_.redeem(kToday).apply(price);
}

const TheatreMenu::Purchase& TheatreMenu::findPurchase(int ticketId) const {
    auto found = std::find_if(purchases_.begin(), purchases_.end(),
                              [ticketId](const Purchase& purchase) { return purchase.ticketId == ticketId; });
    if (found == purchases_.end()) {
        throw InvalidTicketException("Билет с таким номером не найден");
    }
    return *found;
}

void TheatreMenu::buyTicket() {
    showPerformances();
    std::size_t performanceIndex = chooseIndex(kPerformancePrompt, performances_.size());
    showSpectators();
    std::size_t spectatorIndex = chooseIndex(kSpectatorPrompt, spectators_.size());
    Performance& performance = performances_[performanceIndex];
    Spectator& spectator = spectators_[spectatorIndex];
    SeatChoice choice = readSeatChoice(performance);
    Seat& seat = findSeat(performance.getHall(), choice.rowNumber, choice.seatNumber);
    double basePrice = calculateBasePrice(spectator);
    Ticket ticket = theatre_.getTicketOffice().sellTicket(performance, seat, spectator, basePrice);
    purchases_.push_back(Purchase{ticket.getTicketId(), performanceIndex, choice.rowNumber,
                                  choice.seatNumber, spectatorIndex});
    if (loyalty_.isMember(spectator)) {
        loyalty_.awardPoints(spectator, ticket);
    }
    print("Билет №" + std::to_string(ticket.getTicketId()) + " куплен за " +
                   formatMoney(ticket.getPrice()) + ", баланс: " +
                   formatMoney(spectator.getBalance()));
}

void TheatreMenu::returnTicket() {
    int ticketId = readInt("Номер билета");
    const Purchase& purchase = findPurchase(ticketId);
    Performance& performance = performances_[purchase.performanceIndex];
    Seat& seat = findSeat(performance.getHall(), purchase.rowNumber, purchase.seatNumber);
    Spectator& spectator = spectators_[purchase.spectatorIndex];
    double refund = theatre_.getTicketOffice().returnTicket(ticketId, seat, spectator, kToday);
    print("Билет возвращён, возврат: " + formatMoney(refund));
}

void TheatreMenu::topUpBalance() {
    showSpectators();
    Spectator& spectator = spectators_[chooseIndex(kSpectatorPrompt, spectators_.size())];
    double amount = readDouble("Сумма пополнения");
    spectator.topUpBalance(amount);
    print("Баланс: " + formatMoney(spectator.getBalance()));
}

void TheatreMenu::enrollLoyalty() {
    showSpectators();
    Spectator& spectator = spectators_[chooseIndex(kSpectatorPrompt, spectators_.size())];
    loyalty_.enroll(spectator);
    print(spectator.getFullName() + " участвует в программе лояльности");
}

void TheatreMenu::leaveReview() {
    showPerformances();
    std::size_t performanceIndex = chooseIndex(kPerformancePrompt, performances_.size());
    showSpectators();
    std::size_t spectatorIndex = chooseIndex(kSpectatorPrompt, spectators_.size());
    int rating = readInt("Оценка от 1 до 5");
    std::string text = readLine("Текст отзыва");
    Review review(spectators_[spectatorIndex], performances_[performanceIndex], rating, text);
    reviews_.push_back(review);
    print(review.isPositive() ? "Спасибо за положительный отзыв!"
                                       : "Спасибо за отзыв, мы учтём замечания.");
}

void TheatreMenu::showRevenue() {
    print("Выручка от билетов: " + formatMoney(theatre_.getTotalRevenue()));
    print("Остаток бюджета: " + formatMoney(theatre_.getBudget().getRemaining()));
}

void TheatreMenu::payPayroll() {
    std::string month = readLine("Месяц (ГГГГ-ММ)");
    if (paidMonths_.count(month) > 0) {
        throw TheatreException("За этот месяц зарплата уже выплачена");
    }
    Payroll payroll(month);
    for (const Employee& employee : staff_) {
        payroll.addSalary(Salary(employee, kBaseSalary, kSalaryTaxRate));
    }
    payroll.payAll(theatre_.getBudget());
    paidMonths_.insert(month);
    print("Зарплата выплачена, сумма: " + formatMoney(payroll.calculateTotal()));
}

void TheatreMenu::showReport() {
    FinancialReport report(kSeasonStart, kToday, theatre_.getBudget());
    print(report.generateSummary(theatre_.getTotalRevenue()));
}

/// Точка входа: запускает консольное меню театра.
int main() {
#ifdef _WIN32
    SetConsoleOutputCP(CP_UTF8);
    SetConsoleCP(CP_UTF8);
#endif
    TheatreMenu menu;
    menu.run();
    return 0;
}