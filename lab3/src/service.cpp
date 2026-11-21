#include "service.h"

#include <algorithm>
#include <cmath>
#include <iomanip>
#include <numeric>
#include <sstream>

#include "exceptions.h"

namespace {

const double kTwoPi = 6.28318530717958647692;
const int kHoursInDay = 24;
const int kWarmingStartHour = 9;
const double kDailySwingC = 5.0;
const double kRainChanceFactor = 0.6;
const int kHourDigits = 2;

void requirePositive(int value, const std::string& message) {
    if (value <= 0) {
        throw InvalidUserDataException(message);
    }
}

HourlyForecast hourlyFrom(const WeatherSnapshot& start, const Date& date, int hour) {
    std::ostringstream stamp;
    stamp << date << ' ' << std::setfill('0') << std::setw(kHourDigits) << hour << ":00";
    double swing = kDailySwingC * std::sin(kTwoPi * (hour - kWarmingStartHour) / kHoursInDay);
    Temperature temperature(start.getTemperature().toCelsius() + swing, TemperatureUnit::Celsius, stamp.str());
    double chance = start.getCloudCover().getPercent() * kRainChanceFactor;
    return HourlyForecast(stamp.str(), temperature, start.getWind(), chance);
}

}  // namespace

// ----- User -----

User::User(const std::string& id, const std::string& name, const std::string& email, const City& city,
           const Date& registeredAt)
    : id_(id), name_(name), email_(email), city_(city), registeredAt_(registeredAt) {
    if (id.empty() || name.empty() || email.find('@') == std::string::npos) {
        throw InvalidUserDataException("Нужны идентификатор, имя и адрес почты с символом «@»");
    }
}

std::string User::greeting() const {
    return "Здравствуйте, " + name_ + "! Ваш город: " + city_.getName();
}

std::string User::emailDomain() const {
    return email_.substr(email_.find('@') + 1);
}

// ----- Subscription -----

Subscription::Subscription(const User& user, const Region& region, const Severity& minSeverity)
    : user_(user), region_(region), minSeverity_(minSeverity), active_(true) {}

bool Subscription::wants(const Alert& alert) const {
    return active_ && alert.getRegion().getName() == region_.getName() &&
           alert.getSeverity().getLevel() >= minSeverity_.getLevel();
}

void Subscription::pause() {
    active_ = false;
}

void Subscription::resume() {
    active_ = true;
}

// ----- Notification -----

Notification::Notification(const User& recipient, const Alert& alert, const Date& createdAt)
    : recipient_(recipient), text_(alert.headline()), createdAt_(createdAt), delivered_(false) {}

void Notification::deliver() {
    if (delivered_) {
        throw NotificationFailedException("Уведомление для " + recipient_.getEmail() + " уже доставлено");
    }
    delivered_ = true;
}

std::string Notification::format() const {
    return "Для " + recipient_.getName() + " (" + createdAt_ + "): " + text_;
}

// ----- DataSource -----

DataSource::DataSource(const std::string& name, const Region& coverage, bool online)
    : name_(name), coverage_(coverage), online_(online) {}

bool DataSource::covers(const Coordinates& point) const {
    return coverage_.contains(point);
}

void DataSource::ensureOnline() const {
    if (!online_) {
        throw DataSourceUnavailableException("Источник " + name_ + " отключён");
    }
}

// ----- DataProvider -----

DataProvider::DataProvider(const std::string& name) : name_(name), requestsServed_(0) {}

void DataProvider::addSource(const DataSource& source) {
    sources_.push_back(source);
}

const DataSource& DataProvider::sourceFor(const Coordinates& point) {
    auto found = std::find_if(sources_.begin(), sources_.end(),
                              [&point](const DataSource& source) { return source.covers(point); });
    if (found == sources_.end()) {
        throw DataSourceUnavailableException("Ни один источник не покрывает эту точку");
    }
    found->ensureOnline();
    ++requestsServed_;
    return *found;
}

// ----- ForecastService -----

ForecastService::ForecastService(const std::string& name, const StationNetwork& network,
                                 const ForecastModel& model, const AlertCenter& alertCenter)
    : name_(name), network_(network), model_(model), alertCenter_(alertCenter) {}

WeatherSnapshot ForecastService::observe(const Coordinates& point, const RawReadings& raw,
                                         const Precipitation& precipitation, const CloudCover& cloudCover,
                                         const Date& now) const {
    return network_.nearestStation(point).measure(raw, precipitation, cloudCover, now);
}

DailyForecast ForecastService::forecastDay(const WeatherSnapshot& start, const Date& date) const {
    if (!model_.supportsHorizon(kHoursInDay)) {
        throw UnsupportedHorizonException("Модель " + model_.getName() + " не считает на сутки");
    }
    DailyForecast day(date, model_, date + " 00:00");
    for (int hour = 0; hour < kHoursInDay; ++hour) {
        day.addHour(hourlyFrom(start, date, hour));
    }
    return day;
}

std::vector<AlertPtr> ForecastService::checkAlerts(const WeatherSnapshot& snapshot, const Region& region,
                                                   const Date& now) {
    return alertCenter_.evaluate(snapshot, region, now);
}

// ----- SubscriptionManager -----

SubscriptionManager::SubscriptionManager(int maxPerUser) : maxPerUser_(maxPerUser) {
    requirePositive(maxPerUser, "Лимит подписок должен быть положительным");
}

void SubscriptionManager::subscribe(const Subscription& subscription) {
    if (countFor(subscription.getUser()) >= maxPerUser_) {
        throw CapacityExceededException("У пользователя " + subscription.getUser().getId() +
                                        " слишком много подписок");
    }
    subscriptions_.push_back(subscription);
}

std::vector<Notification> SubscriptionManager::dispatch(const Alert& alert, const Date& now) {
    std::vector<Notification> sent;
    for (const Subscription& subscription : subscriptions_) {
        if (subscription.wants(alert)) {
            Notification notification(subscription.getUser(), alert, now);
            notification.deliver();
            sent.push_back(notification);
        }
    }
    notifications_.insert(notifications_.end(), sent.begin(), sent.end());
    return sent;
}

int SubscriptionManager::countFor(const User& user) const {
    return static_cast<int>(std::count_if(subscriptions_.begin(), subscriptions_.end(),
                                          [&user](const Subscription& subscription) {
                                              return subscription.getUser().getId() == user.getId();
                                          }));
}

// ----- ReportGenerator -----

ReportGenerator::ReportGenerator(const std::string& title, const std::string& author,
                                 const Date& generatedAt, int decimals)
    : title_(title), author_(author), generatedAt_(generatedAt), decimals_(decimals) {
    if (decimals < 0) {
        throw InvalidUserDataException("Число знаков после запятой не может быть отрицательным");
    }
}

std::string ReportGenerator::dailyReport(const City& city, const DailyForecast& forecast) const {
    std::ostringstream text;
    text << std::fixed << std::setprecision(decimals_) << title_ << " (" << author_ << ", " << generatedAt_
         << ")\n" << city.getName() << ", " << forecast.getDate() << ": от " << forecast.minTemperature()
         << " до " << forecast.maxTemperature() << " °C, осадки до " << forecast.peakRainChance() << "%";
    return text.str();
}

std::string ReportGenerator::accuracyReport(const ForecastAccuracy& accuracy) const {
    std::ostringstream text;
    text << std::fixed << std::setprecision(decimals_) << title_ << ": средняя ошибка "
         << accuracy.meanError() << " °C, максимальная " << accuracy.getMaxError() << " °C, сравнений "
         << accuracy.getForecastCount();
    return text.str();
}

// ----- FavoriteLocations -----

FavoriteLocations::FavoriteLocations(const User& owner, int maxCount) : owner_(owner), maxCount_(maxCount) {
    requirePositive(maxCount, "Лимит избранных городов должен быть положительным");
}

void FavoriteLocations::add(const City& city) {
    if (contains(city.getName())) {
        throw InvalidUserDataException("Город " + city.getName() + " уже в избранном");
    }
    if (getCount() >= maxCount_) {
        throw CapacityExceededException("Список избранных городов заполнен");
    }
    cities_.push_back(city);
}

void FavoriteLocations::remove(const std::string& cityName) {
    auto found = std::find_if(cities_.begin(), cities_.end(),
                              [&cityName](const City& city) { return city.getName() == cityName; });
    if (found == cities_.end()) {
        throw InvalidUserDataException("Города " + cityName + " нет в избранном");
    }
    cities_.erase(found);
}

bool FavoriteLocations::contains(const std::string& cityName) const {
    return std::any_of(cities_.begin(), cities_.end(),
                       [&cityName](const City& city) { return city.getName() == cityName; });
}

// ----- ObservationArchive -----

ObservationArchive::ObservationArchive(const std::string& name, int capacity)
    : name_(name), capacity_(capacity) {
    if (capacity <= 0) {
        throw InvalidMeasurementException("Ёмкость архива наблюдений должна быть положительной");
    }
}

void ObservationArchive::store(const WeatherSnapshot& snapshot) {
    if (getCount() >= capacity_) {
        throw CapacityExceededException("Архив наблюдений " + name_ + " заполнен");
    }
    snapshots_.push_back(snapshot);
}

double ObservationArchive::averageTemperature() const {
    if (snapshots_.empty()) {
        throw ForecastNotAvailableException("В архиве наблюдений нет данных");
    }
    double total = std::accumulate(snapshots_.begin(), snapshots_.end(), 0.0,
                                   [](double sum, const WeatherSnapshot& snapshot) {
                                       return sum + snapshot.getTemperature().toCelsius();
                                   });
    return total / static_cast<double>(snapshots_.size());
}

int ObservationArchive::countWarmerThan(const Temperature& limit) const {
    return static_cast<int>(std::count_if(snapshots_.begin(), snapshots_.end(),
                                          [&limit](const WeatherSnapshot& snapshot) {
                                              return snapshot.getTemperature().toCelsius() > limit.toCelsius();
                                          }));
}