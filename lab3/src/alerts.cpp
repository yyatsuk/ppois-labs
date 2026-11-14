#include "alerts.h"

#include <algorithm>
#include <iomanip>
#include <numeric>
#include <sstream>

#include "exceptions.h"

namespace {

const int kMinLevel = 1;
const int kMaxLevel = 4;
const int kDangerousLevel = 3;
const int kDelaysMinutes[] = {60, 30, 10, 0};
const int kDateLength = 10;
const double kStormRadiusKm = 50.0;
const double kMajorDamage = 1000000.0;
const double kSignificantDamage = 100000.0;
const char* const kEndOfDay = " 23:59";
const int kDecimals = 1;

std::string number(double value) {
    std::ostringstream text;
    text << std::fixed << std::setprecision(kDecimals) << value;
    return text.str();
}

void requirePositive(double value, const std::string& message) {
    if (value <= 0) {
        throw InvalidAlertException(message);
    }
}

}  // namespace

// ----- Severity -----

Severity::Severity(int level, const std::string& label) : level_(level), label_(label) {
    if (level < kMinLevel || level > kMaxLevel) {
        throw InvalidAlertException("Степень опасности должна быть от 1 до 4");
    }
}

bool Severity::isDangerous() const {
    return level_ >= kDangerousLevel;
}

int Severity::notificationDelayMinutes() const {
    return kDelaysMinutes[level_ - kMinLevel];
}

// ----- AlertRule -----

AlertRule::AlertRule(const std::string& name, double windThresholdMs, double rainThresholdMmH,
                     const Severity& severity)
    : name_(name), windThresholdMs_(windThresholdMs), rainThresholdMmH_(rainThresholdMmH),
      severity_(severity) {
    requirePositive(windThresholdMs, "Порог ветра должен быть положительным");
    requirePositive(rainThresholdMmH, "Порог осадков должен быть положительным");
}

bool AlertRule::matches(const WeatherSnapshot& snapshot) const {
    return snapshot.getWind().getSpeedMs() >= windThresholdMs_ ||
           snapshot.getPrecipitation().intensity() >= rainThresholdMmH_;
}

std::string AlertRule::describe(const WeatherSnapshot& snapshot) const {
    return "Правило «" + name_ + "»: ветер " + number(snapshot.getWind().getSpeedMs()) + " м/с (порог " +
           number(windThresholdMs_) + "), осадки " + number(snapshot.getPrecipitation().intensity()) +
           " мм/ч (порог " + number(rainThresholdMmH_) + ")";
}

// ----- Alert -----

Alert::Alert(const std::string& id, const std::string& message, const Severity& severity,
             const Region& region, const Date& validUntil)
    : id_(id), message_(message), severity_(severity), region_(region), validUntil_(validUntil) {
    if (id.empty() || message.empty()) {
        throw InvalidAlertException("Идентификатор и текст предупреждения не должны быть пустыми");
    }
    if (static_cast<int>(validUntil.size()) < kDateLength) {
        throw InvalidAlertException("Срок действия должен быть в формате ГГГГ-ММ-ДД");
    }
}

bool Alert::isActive(const Date& now) const {
    return now <= validUntil_;
}

std::string Alert::headline() const {
    return "[" + severity_.getLabel() + "] " + message_ + " — " + region_.getName();
}

// ----- StormWarning -----

StormWarning::StormWarning(const std::string& id, const Severity& severity, const Region& region,
                           const Date& validUntil, const City& center, double maxGustMs)
    : Alert(id, "Штормовое предупреждение", severity, region, validUntil),
      center_(center), maxGustMs_(maxGustMs) {}

std::string StormWarning::headline() const {
    return Alert::headline() + ", порывы до " + number(maxGustMs_) + " м/с, центр: " + center_.getName();
}

bool StormWarning::threatens(const City& other) const {
    return center_.getLocation().distanceTo(other.getLocation()) <= kStormRadiusKm;
}

// ----- FrostWarning -----

FrostWarning::FrostWarning(const std::string& id, const Severity& severity, const Region& region,
                           const Date& validUntil, double thresholdC, int nights)
    : Alert(id, "Заморозки", severity, region, validUntil), thresholdC_(thresholdC), nights_(nights) {}

std::string FrostWarning::headline() const {
    return Alert::headline() + ", до " + number(thresholdC_) + " °C, ночей: " + std::to_string(nights_);
}

bool FrostWarning::confirmedBy(const Temperature& observed) const {
    return observed.toCelsius() <= thresholdC_;
}

// ----- HeatWarning -----

HeatWarning::HeatWarning(const std::string& id, const Severity& severity, const Region& region,
                         const Date& validUntil, double thresholdC, int days)
    : Alert(id, "Сильная жара", severity, region, validUntil), thresholdC_(thresholdC), days_(days) {}

std::string HeatWarning::headline() const {
    return Alert::headline() + ", от " + number(thresholdC_) + " °C, дней: " + std::to_string(days_);
}

bool HeatWarning::confirmedBy(const Temperature& observed) const {
    return observed.toCelsius() >= thresholdC_;
}

// ----- FloodWarning -----

FloodWarning::FloodWarning(const std::string& id, const Severity& severity, const Region& region,
                           const Date& validUntil, double rainfallLimitMm, double riverLevelCm)
    : Alert(id, "Угроза наводнения", severity, region, validUntil),
      rainfallLimitMm_(rainfallLimitMm), riverLevelCm_(riverLevelCm) {}

std::string FloodWarning::headline() const {
    return Alert::headline() + ", уровень реки " + number(riverLevelCm_) + " см, осадки от " +
           number(rainfallLimitMm_) + " мм";
}

bool FloodWarning::confirmedBy(const Precipitation& observed) const {
    return observed.getAmountMm() >= rainfallLimitMm_;
}

// ----- AlertCenter -----

AlertCenter::AlertCenter(const std::string& name) : name_(name), nextId_(1) {}

void AlertCenter::addRule(const AlertRule& rule) {
    rules_.push_back(rule);
}

std::vector<AlertPtr> AlertCenter::evaluate(const WeatherSnapshot& snapshot, const Region& region,
                                            const Date& now) {
    std::vector<AlertPtr> issued;
    Date validUntil = now.substr(0, kDateLength) + kEndOfDay;
    for (const AlertRule& rule : rules_) {
        if (rule.matches(snapshot)) {
            std::string id = "A" + std::to_string(nextId_++);
            issued.push_back(std::make_shared<Alert>(id, rule.describe(snapshot), rule.getSeverity(),
                                                     region, validUntil));
        }
    }
    alerts_.insert(alerts_.end(), issued.begin(), issued.end());
    return issued;
}

void AlertCenter::publish(const AlertPtr& alert) {
    if (!alert) {
        throw InvalidAlertException("Нельзя опубликовать пустое предупреждение");
    }
    alerts_.push_back(alert);
}

std::vector<AlertPtr> AlertCenter::activeAlerts(const Date& now) const {
    std::vector<AlertPtr> active;
    std::copy_if(alerts_.begin(), alerts_.end(), std::back_inserter(active),
                 [&now](const AlertPtr& alert) { return alert->isActive(now); });
    return active;
}

// ----- WeatherEvent -----

WeatherEvent::WeatherEvent(const std::string& name, const Date& occurredAt, const Severity& severity,
                           double damage)
    : name_(name), occurredAt_(occurredAt), severity_(severity), damage_(damage) {
    if (damage < 0) {
        throw InvalidAlertException("Ущерб не может быть отрицательным");
    }
}

bool WeatherEvent::isMajor() const {
    return severity_.isDangerous() || damage_ >= kMajorDamage;
}

std::string WeatherEvent::classify() const {
    if (damage_ >= kMajorDamage) {
        return "катастрофическое";
    }
    return damage_ >= kSignificantDamage ? "значительное" : "незначительное";
}

// ----- EventArchive -----

EventArchive::EventArchive(const Region& region, int capacity) : region_(region), capacity_(capacity) {
    requirePositive(capacity, "Ёмкость архива должна быть положительной");
}

void EventArchive::record(const WeatherEvent& event) {
    if (getEventCount() >= capacity_) {
        throw CapacityExceededException("Архив региона " + region_.getName() + " заполнен");
    }
    events_.push_back(event);
}

std::vector<WeatherEvent> EventArchive::majorEvents() const {
    std::vector<WeatherEvent> major;
    std::copy_if(events_.begin(), events_.end(), std::back_inserter(major),
                 [](const WeatherEvent& event) { return event.isMajor(); });
    return major;
}

double EventArchive::totalDamage() const {
    return std::accumulate(events_.begin(), events_.end(), 0.0,
                           [](double sum, const WeatherEvent& event) { return sum + event.getDamage(); });
}