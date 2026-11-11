#include "repertoire.h"

#include <algorithm>

#include "exceptions.h"

namespace {

const double kMinutesPerHour = 60.0;
const int kReadingMinutesPerPage = 2;
const int kMinCondition = 0;
const int kMaxCondition = 100;
const int kMinUsableCondition = 30;
const double kAssemblyMinutesPerKg = 0.5;

void requirePositive(double value, const std::string& message) {
    if (value <= 0) {
        throw TheatreException(message);
    }
}

std::map<std::string, std::string>::iterator findRole(
    std::map<std::string, std::string>& assignments, const Role& role) {
    auto found = assignments.find(role.getName());
    if (found == assignments.end()) {
        throw TheatreException("Такой роли нет в составе");
    }
    return found;
}

}  // namespace

// ----- Genre -----

Genre::Genre(const std::string& name, const std::string& description, int minAge)
    : name_(name), description_(description), minAge_(minAge) {
    if (minAge < 0) {
        throw TheatreException("Возрастное ограничение не может быть отрицательным");
    }
}

bool Genre::isAllowedFor(int age) const {
    return age >= minAge_;
}

const std::string& Genre::getName() const {
    return name_;
}

// ----- Playwright -----

Playwright::Playwright(const std::string& name, const std::string& country)
    : name_(name), country_(country) {}

void Playwright::addPlay(const Play& play) {
    bool alreadyAdded = std::find(plays_.begin(), plays_.end(), play.getTitle()) != plays_.end();
    if (alreadyAdded) {
        throw TheatreException("Пьеса уже есть в списке автора");
    }
    plays_.push_back(play.getTitle());
}

std::size_t Playwright::getPlaysCount() const {
    return plays_.size();
}

const std::string& Playwright::getName() const {
    return name_;
}

// ----- Play -----

Play::Play(const std::string& title, int durationMinutes, const Genre& genre,
           const Playwright& author)
    : title_(title), durationMinutes_(durationMinutes), genre_(genre), author_(author) {
    requirePositive(durationMinutes, "Длительность пьесы должна быть положительной");
}

bool Play::isSuitableFor(int age) const {
    return genre_.isAllowedFor(age);
}

double Play::getDurationHours() const {
    return durationMinutes_ / kMinutesPerHour;
}

const std::string& Play::getTitle() const {
    return title_;
}

int Play::getDurationMinutes() const {
    return durationMinutes_;
}

// ----- Script -----

Script::Script(const Play& play, int version, int pageCount)
    : play_(play), version_(version), pageCount_(pageCount) {
    requirePositive(version, "Номер версии должен быть положительным");
    requirePositive(pageCount, "Число страниц должно быть положительным");
}

void Script::revise(int newPageCount) {
    requirePositive(newPageCount, "Число страниц должно быть положительным");
    pageCount_ = newPageCount;
    ++version_;
}

int Script::estimateReadingTime() const {
    return pageCount_ * kReadingMinutesPerPage;
}

int Script::getVersion() const {
    return version_;
}

const Play& Script::getPlay() const {
    return play_;
}

// ----- Role -----

Role::Role(const std::string& name, int minAge, int maxAge)
    : name_(name), minAge_(minAge), maxAge_(maxAge) {
    validateAgeRange();
}

bool Role::isSuitableForAge(int age) const {
    return age >= minAge_ && age <= maxAge_;
}

void Role::validateAgeRange() const {
    if (minAge_ < 0 || minAge_ > maxAge_) {
        throw TheatreException("Некорректный возрастной диапазон роли");
    }
}

const std::string& Role::getName() const {
    return name_;
}

// ----- Production -----

Production::Production(const Play& play, const Director& director, const Date& premiereDate)
    : play_(play),
      director_(director),
      premiereDate_(premiereDate),
      status_(ProductionStatus::Draft) {}

void Production::approve() {
    if (status_ == ProductionStatus::Cancelled) {
        throw TheatreException("Отменённую постановку нельзя утвердить");
    }
    status_ = ProductionStatus::Approved;
}

void Production::cancel() {
    status_ = ProductionStatus::Cancelled;
}

bool Production::isPremiered(const Date& today) const {
    return status_ == ProductionStatus::Approved && today >= premiereDate_;
}

const Play& Production::getPlay() const {
    return play_;
}

const Director& Production::getDirector() const {
    return director_;
}

const Date& Production::getPremiereDate() const {
    return premiereDate_;
}

ProductionStatus Production::getStatus() const {
    return status_;
}

// ----- Cast -----

Cast::Cast(const Production& production, const std::vector<Role>& roles)
    : production_(production), approved_(false) {
    if (roles.empty()) {
        throw TheatreException("В составе должна быть хотя бы одна роль");
    }
    for (const Role& role : roles) {
        assignments_[role.getName()] = "";
    }
}

void Cast::assignActor(const Role& role, const Actor& actor, const Date& today) {
    if (approved_) {
        throw TheatreException("Состав уже утверждён");
    }
    auto assignment = findRole(assignments_, role);
    if (!assignment->second.empty()) {
        throw TheatreException("Роль уже занята");
    }
    if (!actor.canPlay(role, today)) {
        throw ActorNotAvailableException("Актёр недоступен или не подходит на роль");
    }
    assignment->second = actor.getFullName();
}

void Cast::removeActor(const Role& role) {
    if (approved_) {
        throw TheatreException("Состав уже утверждён");
    }
    findRole(assignments_, role)->second.clear();
}

bool Cast::isComplete() const {
    return std::all_of(assignments_.begin(), assignments_.end(),
                       [](const auto& assignment) { return !assignment.second.empty(); });
}

void Cast::approve() {
    if (!isComplete()) {
        throw InvalidCastException("Состав неполный, утверждение невозможно");
    }
    approved_ = true;
}

bool Cast::isApproved() const {
    return approved_;
}

const Production& Cast::getProduction() const {
    return production_;
}

// ----- Season -----

Season::Season(const std::string& name, const Date& startDate, const Date& endDate)
    : name_(name), startDate_(startDate), endDate_(endDate) {
    if (endDate < startDate) {
        throw TheatreException("Дата окончания сезона раньше даты начала");
    }
}

void Season::addProduction(const Production& production) {
    const Date& premiere = production.getPremiereDate();
    if (premiere < startDate_ || premiere > endDate_) {
        throw TheatreException("Премьера выходит за рамки сезона");
    }
    productions_.push_back(production);
}

bool Season::isActive(const Date& today) const {
    return today >= startDate_ && today <= endDate_;
}

std::size_t Season::getProductionsCount() const {
    return productions_.size();
}

// ----- Costume -----

Costume::Costume(const std::string& name, const std::string& size, int condition,
                 const Role& role)
    : name_(name), size_(size), condition_(condition), role_(role) {
    if (condition < kMinCondition || condition > kMaxCondition) {
        throw TheatreException("Состояние костюма должно быть от 0 до 100");
    }
}

void Costume::repair() {
    condition_ = kMaxCondition;
}

bool Costume::isUsable() const {
    return condition_ >= kMinUsableCondition;
}

const std::string& Costume::getName() const {
    return name_;
}

const Role& Costume::getRole() const {
    return role_;
}

// ----- Decoration -----

Decoration::Decoration(const std::string& name, double weightKg, const Production& production)
    : name_(name), weightKg_(weightKg), production_(production) {
    requirePositive(weightKg, "Вес декорации должен быть положительным");
}

std::string Decoration::assemble() const {
    return "Декорация «" + name_ + "» собрана за " +
           std::to_string(calculateAssemblyTime()) + " мин";
}

double Decoration::calculateAssemblyTime() const {
    return weightKg_ * kAssemblyMinutesPerKg;
}

double Decoration::getWeightKg() const {
    return weightKg_;
}