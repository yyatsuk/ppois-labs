#include "backstage_finance.h"

#include <algorithm>
#include <iomanip>
#include <sstream>

#include "exceptions.h"

namespace {

const double kCostumePurchaseCost = 100.0;
const int kMinVolume = 0;
const int kMaxVolume = 100;
const int kDefaultVolume = 50;
const int kMinRating = 1;
const int kMaxRating = 5;
const int kMinPositiveRating = 4;
const int kContractDurationYears = 1;
const std::size_t kYearLength = 4;
const std::size_t kMonthLength = 7;
const int kMoneyDecimals = 2;
const char* const kFirstDaySuffix = "-01";
const char* const kPayrollCategory = "Зарплата";

void requirePositive(double value, const std::string& message) {
    if (value <= 0) {
        throw TheatreException(message);
    }
}

int yearOf(const Date& date) {
    return std::stoi(date.substr(0, kYearLength));
}

double periodExpenses(const Budget& budget, const Date& from, const Date& to) {
    double total = 0.0;
    for (const Expense& expense : budget.getExpenses()) {
        if (expense.isInPeriod(from, to)) {
            total += expense.getAmount();
        }
    }
    return total;
}

}  // namespace

// ----- CostumeDepartment -----

CostumeDepartment::CostumeDepartment(const std::string& headName, double budget)
    : headName_(headName), budget_(budget) {
    if (budget < 0) {
        throw TheatreException("Бюджет костюмерной не может быть отрицательным");
    }
}

void CostumeDepartment::addCostume(const Costume& costume) {
    if (budget_ < kCostumePurchaseCost) {
        throw InsufficientFundsException("Недостаточно средств на покупку костюма");
    }
    budget_ -= kCostumePurchaseCost;
    costumes_.push_back(costume);
}

Costume CostumeDepartment::issueCostume(const Role& role) {
    auto found = std::find_if(costumes_.begin(), costumes_.end(), [&role](const Costume& costume) {
        return costume.getRole().getName() == role.getName() && costume.isUsable();
    });
    if (found == costumes_.end()) {
        throw TheatreException("Подходящего костюма для роли нет");
    }
    Costume issued = *found;
    costumes_.erase(found);
    return issued;
}

std::size_t CostumeDepartment::getCostumesCount() const {
    return costumes_.size();
}

double CostumeDepartment::getBudget() const {
    return budget_;
}

// ----- LightingSystem -----

LightingSystem::LightingSystem(int lampsCount) : lampsCount_(lampsCount), isOn_(false) {
    requirePositive(lampsCount, "Число ламп должно быть положительным");
}

void LightingSystem::setScene(const std::string& scene) {
    if (!isOn_) {
        throw TheatreException("Система освещения выключена");
    }
    if (scene.empty()) {
        throw TheatreException("Название световой сцены не может быть пустым");
    }
    currentScene_ = scene;
}

void LightingSystem::toggle() {
    isOn_ = !isOn_;
    if (!isOn_) {
        currentScene_.clear();
    }
}

bool LightingSystem::isOn() const {
    return isOn_;
}

int LightingSystem::getLampsCount() const {
    return lampsCount_;
}

const std::string& LightingSystem::getCurrentScene() const {
    return currentScene_;
}

// ----- SoundSystem -----

SoundSystem::SoundSystem(int microphonesCount)
    : microphonesCount_(microphonesCount), volumeLevel_(kDefaultVolume), isMuted_(false) {
    requirePositive(microphonesCount, "Число микрофонов должно быть положительным");
}

void SoundSystem::setVolume(int level) {
    if (isMuted_) {
        throw TheatreException("Звук выключен");
    }
    if (level < kMinVolume || level > kMaxVolume) {
        throw TheatreException("Громкость должна быть от 0 до 100");
    }
    volumeLevel_ = level;
}

int SoundSystem::assignMicrophone(const Actor& actor) {
    const std::string& name = actor.getFullName();
    bool alreadyAssigned =
        std::find(assignedActors_.begin(), assignedActors_.end(), name) != assignedActors_.end();
    if (alreadyAssigned) {
        throw TheatreException("У актёра уже есть микрофон");
    }
    if (static_cast<int>(assignedActors_.size()) >= microphonesCount_) {
        throw TheatreException("Свободных микрофонов нет");
    }
    assignedActors_.push_back(name);
    return static_cast<int>(assignedActors_.size());
}

void SoundSystem::setMuted(bool muted) {
    isMuted_ = muted;
}

int SoundSystem::getVolumeLevel() const {
    return volumeLevel_;
}

bool SoundSystem::isMuted() const {
    return isMuted_;
}

// ----- Expense -----

Expense::Expense(const std::string& description, double amount, const std::string& category,
                 const Date& date)
    : description_(description), amount_(amount), category_(category), date_(date) {
    requirePositive(amount, "Сумма расхода должна быть положительной");
}

bool Expense::isLargerThan(double threshold) const {
    return amount_ > threshold;
}

bool Expense::isInPeriod(const Date& from, const Date& to) const {
    return date_ >= from && date_ <= to;
}

double Expense::getAmount() const {
    return amount_;
}

const std::string& Expense::getDescription() const {
    return description_;
}

const std::string& Expense::getCategory() const {
    return category_;
}

const Date& Expense::getDate() const {
    return date_;
}

// ----- Budget -----

Budget::Budget(int year, double totalAmount) : year_(year), totalAmount_(totalAmount) {
    if (totalAmount < 0) {
        throw TheatreException("Бюджет не может быть отрицательным");
    }
}

void Budget::addExpense(const Expense& expense) {
    expenses_.push_back(expense);
}

void Budget::addFunds(double amount) {
    requirePositive(amount, "Сумма пополнения должна быть положительной");
    totalAmount_ += amount;
}

double Budget::getRemaining() const {
    double spent = 0.0;
    for (const Expense& expense : expenses_) {
        spent += expense.getAmount();
    }
    return totalAmount_ - spent;
}

bool Budget::isExceeded() const {
    return getRemaining() < 0;
}

const std::vector<Expense>& Budget::getExpenses() const {
    return expenses_;
}

// ----- Sponsor -----

Sponsor::Sponsor(const std::string& name, double contributionAmount, const Date& contractDate)
    : name_(name), contributionAmount_(contributionAmount), contractDate_(contractDate) {
    requirePositive(contributionAmount, "Сумма взноса должна быть положительной");
}

void Sponsor::donate(Budget& budget, const Date& today) const {
    if (!isActive(today)) {
        throw TheatreException("Договор со спонсором не действует");
    }
    budget.addFunds(contributionAmount_);
}

bool Sponsor::isActive(const Date& today) const {
    bool started = today >= contractDate_;
    bool notFinished = yearOf(today) < yearOf(contractDate_) + kContractDurationYears;
    return started && notFinished;
}

// ----- Payroll -----

Payroll::Payroll(const std::string& month) : month_(month), isPaid_(false) {
    if (month.size() != kMonthLength) {
        throw TheatreException("Месяц должен быть в формате ГГГГ-ММ");
    }
}

void Payroll::addSalary(const Salary& salary) {
    if (isPaid_) {
        throw TheatreException("Ведомость уже выплачена");
    }
    salaries_.push_back(salary);
}

double Payroll::calculateTotal() const {
    double total = 0.0;
    for (const Salary& salary : salaries_) {
        total += salary.calculateNet();
    }
    return total;
}

void Payroll::payAll(Budget& budget) {
    if (isPaid_) {
        throw TheatreException("Ведомость уже выплачена");
    }
    double total = calculateTotal();
    if (budget.getRemaining() < total) {
        throw InsufficientFundsException("В бюджете недостаточно средств на зарплаты");
    }
    budget.addExpense(Expense("Зарплата за " + month_, total, kPayrollCategory, month_ + kFirstDaySuffix));
    isPaid_ = true;
}

bool Payroll::isPaid() const {
    return isPaid_;
}

// ----- FinancialReport -----

FinancialReport::FinancialReport(const Date& periodStart, const Date& periodEnd,
                                 const Budget& budget)
    : periodStart_(periodStart), periodEnd_(periodEnd), budget_(budget) {
    if (periodEnd < periodStart) {
        throw TheatreException("Конец периода раньше его начала");
    }
}

double FinancialReport::calculateProfit(double revenue) const {
    return revenue - periodExpenses(budget_, periodStart_, periodEnd_);
}

std::string FinancialReport::generateSummary(double revenue) const {
    double expenses = periodExpenses(budget_, periodStart_, periodEnd_);
    std::ostringstream summary;
    summary << std::fixed << std::setprecision(kMoneyDecimals) << "Период " << periodStart_
            << " — " << periodEnd_ << ": выручка " << revenue << ", расходы " << expenses
            << ", прибыль " << (revenue - expenses);
    return summary.str();
}

// ----- Review -----

Review::Review(const Spectator& author, const Performance& performance, int rating,
               const std::string& text)
    : author_(author), performance_(performance), rating_(rating), text_(text) {
    validate();
}

void Review::validate() const {
    if (rating_ < kMinRating || rating_ > kMaxRating) {
        throw TheatreException("Оценка должна быть от 1 до 5");
    }
    if (text_.empty()) {
        throw TheatreException("Текст отзыва не может быть пустым");
    }
}

bool Review::isPositive() const {
    return rating_ >= kMinPositiveRating;
}

int Review::getRating() const {
    return rating_;
}

// ----- Theatre -----

Theatre::Theatre(const std::string& name, const Hall& hall, const Season& season,
                 const Cashier& cashier, const Budget& budget)
    : name_(name),
      hall_(hall),
      season_(season),
      ticketOffice_(Schedule(season), cashier),
      budget_(budget) {}

void Theatre::planSeason(const Production& production) {
    season_.addProduction(production);
}

void Theatre::registerPerformance(const Performance& performance) {
    if (performance.getHall().getName() != hall_.getName()) {
        throw TheatreException("Спектакль назначен в другой зал");
    }
    ticketOffice_.getSchedule().addPerformance(performance);
}

double Theatre::getTotalRevenue() const {
    return ticketOffice_.getRevenue();
}

TicketOffice& Theatre::getTicketOffice() {
    return ticketOffice_;
}

Budget& Theatre::getBudget() {
    return budget_;
}

const Hall& Theatre::getHall() const {
    return hall_;
}

const std::string& Theatre::getName() const {
    return name_;
}