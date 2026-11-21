#include "forecasts.h"

#include <algorithm>
#include <cmath>
#include <iomanip>
#include <numeric>
#include <sstream>

#include "exceptions.h"

namespace {

const double kMinPercent = 0.0;
const double kMaxPercent = 100.0;
const double kLowConfidence = 40.0;
const double kReliableConfidence = 70.0;
const double kDryChance = 30.0;
const double kStormChance = 70.0;
const int kDaysInWeek = 7;
const int kMonthsInYear = 12;
const int kDateLength = 10;
const int kMinObservations = 2;
const double kTrendThresholdHpa = 1.0;
const int kReportDecimals = 1;

void requirePercent(double value, const std::string& message) {
    if (value < kMinPercent || value > kMaxPercent) {
        throw InvalidMeasurementException(message);
    }
}

std::vector<double> correctedTemperatures(const ForecastModel& model,
                                          const std::vector<HourlyForecast>& hours) {
    if (hours.empty()) {
        throw ForecastNotAvailableException("В прогнозе на день нет ни одного часа");
    }
    std::vector<double> values;
    for (const HourlyForecast& hour : hours) {
        values.push_back(model.correct(hour.getTemperature()).toCelsius());
    }
    return values;
}

}  // namespace

// ----- Confidence -----

Confidence::Confidence(double scorePercent, int horizonHours, const std::string& modelName)
    : scorePercent_(scorePercent), horizonHours_(horizonHours), modelName_(modelName) {
    requirePercent(scorePercent, "Уверенность должна быть от 0 до 100 процентов");
    if (horizonHours <= 0) {
        throw UnsupportedHorizonException("Горизонт прогноза должен быть положительным");
    }
}

std::string Confidence::level() const {
    if (scorePercent_ < kLowConfidence) {
        return "низкая";
    }
    return scorePercent_ < kReliableConfidence ? "средняя" : "высокая";
}

bool Confidence::isReliable() const {
    return scorePercent_ >= kReliableConfidence;
}

// ----- ForecastModel -----

ForecastModel::ForecastModel(const std::string& name, double resolutionKm, int maxHorizonHours, double bias)
    : name_(name), resolutionKm_(resolutionKm), maxHorizonHours_(maxHorizonHours), bias_(bias) {
    if (resolutionKm <= 0 || maxHorizonHours <= 0) {
        throw InvalidMeasurementException("Разрешение и горизонт модели должны быть положительными");
    }
}

Temperature ForecastModel::correct(const Temperature& raw) const {
    return Temperature(raw.toCelsius() - bias_, TemperatureUnit::Celsius, raw.getMeasuredAt());
}

bool ForecastModel::supportsHorizon(int hours) const {
    return hours > 0 && hours <= maxHorizonHours_;
}

// ----- HourlyForecast -----

HourlyForecast::HourlyForecast(const Date& hour, const Temperature& temperature, const Wind& wind,
                               double precipitationChance)
    : hour_(hour), temperature_(temperature), wind_(wind), precipitationChance_(precipitationChance) {
    requirePercent(precipitationChance, "Вероятность осадков должна быть от 0 до 100 процентов");
}

bool HourlyForecast::isDry() const {
    return precipitationChance_ < kDryChance;
}

bool HourlyForecast::isStormy() const {
    return wind_.isStrong() || precipitationChance_ >= kStormChance;
}

// ----- ModelRun -----

ModelRun::ModelRun(const ForecastModel& model, const WeatherSnapshot& initial, const Date& runAt)
    : model_(model), initial_(initial), runAt_(runAt) {}

void ModelRun::addStep(const HourlyForecast& step) {
    steps_.push_back(step);
}

const HourlyForecast& ModelRun::forecastAt(int hoursAhead) const {
    if (!model_.supportsHorizon(hoursAhead)) {
        throw UnsupportedHorizonException("Модель " + model_.getName() + " не считает на такой срок");
    }
    if (hoursAhead > getStepCount()) {
        throw ForecastNotAvailableException("Шаг расчёта ещё не готов");
    }
    return steps_[hoursAhead - 1];
}

// ----- DailyForecast -----

DailyForecast::DailyForecast(const Date& date, const ForecastModel& model, const Date& issuedAt)
    : date_(date), model_(model), issuedAt_(issuedAt) {}

void DailyForecast::addHour(const HourlyForecast& hour) {
    if (hour.getHour().compare(0, date_.size(), date_) != 0) {
        throw ForecastNotAvailableException("Час " + hour.getHour() + " не относится к дню " + date_);
    }
    hours_.push_back(hour);
}

double DailyForecast::maxTemperature() const {
    std::vector<double> values = correctedTemperatures(model_, hours_);
    return *std::max_element(values.begin(), values.end());
}

double DailyForecast::minTemperature() const {
    std::vector<double> values = correctedTemperatures(model_, hours_);
    return *std::min_element(values.begin(), values.end());
}

double DailyForecast::peakRainChance() const {
    correctedTemperatures(model_, hours_);
    auto peak = std::max_element(hours_.begin(), hours_.end(), [](const HourlyForecast& a, const HourlyForecast& b) {
        return a.getPrecipitationChance() < b.getPrecipitationChance();
    });
    return peak->getPrecipitationChance();
}

// ----- WeeklyForecast -----

WeeklyForecast::WeeklyForecast(const City& city, const Confidence& confidence, const Date& startDate)
    : city_(city), confidence_(confidence), startDate_(startDate) {}

void WeeklyForecast::addDay(const DailyForecast& day) {
    if (static_cast<int>(days_.size()) >= kDaysInWeek) {
        throw CapacityExceededException("В недельном прогнозе уже семь дней");
    }
    days_.push_back(day);
}

const DailyForecast& WeeklyForecast::warmestDay() const {
    if (days_.empty()) {
        throw ForecastNotAvailableException("В недельном прогнозе нет дней");
    }
    return *std::max_element(days_.begin(), days_.end(), [](const DailyForecast& a, const DailyForecast& b) {
        return a.maxTemperature() < b.maxTemperature();
    });
}

double WeeklyForecast::averageMaxTemperature() const {
    if (days_.empty()) {
        throw ForecastNotAvailableException("В недельном прогнозе нет дней");
    }
    double total = std::accumulate(days_.begin(), days_.end(), 0.0, [](double sum, const DailyForecast& day) {
        return sum + day.maxTemperature();
    });
    return total / static_cast<double>(days_.size());
}

// ----- ForecastAccuracy -----

ForecastAccuracy::ForecastAccuracy(const std::string& modelName)
    : forecastCount_(0), totalError_(0.0), maxError_(0.0), modelName_(modelName) {}

void ForecastAccuracy::record(const Temperature& predicted, const Temperature& actual) {
    double error = std::fabs(predicted.toCelsius() - actual.toCelsius());
    totalError_ += error;
    maxError_ = std::max(maxError_, error);
    ++forecastCount_;
}

double ForecastAccuracy::meanError() const {
    if (forecastCount_ == 0) {
        throw ForecastNotAvailableException("Для модели " + modelName_ + " нет сравнений с фактом");
    }
    return totalError_ / forecastCount_;
}

// ----- ForecastBulletin -----

ForecastBulletin::ForecastBulletin(const City& city, const DailyForecast& forecast,
                                   const Confidence& confidence, const Date& issuedAt)
    : city_(city), forecast_(forecast), confidence_(confidence), issuedAt_(issuedAt) {}

std::string ForecastBulletin::render() const {
    std::ostringstream text;
    text << std::fixed << std::setprecision(kReportDecimals) << city_.getName() << ", "
         << forecast_.getDate() << ": от " << forecast_.minTemperature() << " до "
         << forecast_.maxTemperature() << " °C, вероятность осадков до " << forecast_.peakRainChance()
         << "%, уверенность " << confidence_.level();
    return text.str();
}

bool ForecastBulletin::isActual(const Date& now) const {
    return now.substr(0, kDateLength) == issuedAt_.substr(0, kDateLength);
}

// ----- Climatology -----

Climatology::Climatology(const City& city, const std::vector<double>& monthlyMeans, int yearsOfData)
    : city_(city), monthlyMeans_(monthlyMeans), yearsOfData_(yearsOfData) {
    if (static_cast<int>(monthlyMeans.size()) != kMonthsInYear) {
        throw InvalidMeasurementException("Нужны средние температуры за двенадцать месяцев");
    }
}

double Climatology::normalFor(int month) const {
    if (month < 1 || month > kMonthsInYear) {
        throw InvalidMeasurementException("Месяц должен быть от 1 до 12");
    }
    return monthlyMeans_[month - 1];
}

double Climatology::anomaly(const Temperature& observed, int month) const {
    return observed.toCelsius() - normalFor(month);
}

// ----- TrendAnalyzer -----

TrendAnalyzer::TrendAnalyzer(int windowSize) : windowSize_(windowSize) {
    if (windowSize < kMinObservations) {
        throw InvalidMeasurementException("Окно анализа должно содержать не меньше двух наблюдений");
    }
}

void TrendAnalyzer::addObservation(const Pressure& pressure, const Temperature& temperature) {
    pressureHistory_.push_back(pressure.getHectopascals());
    temperatureHistory_.push_back(temperature.toCelsius());
    if (static_cast<int>(pressureHistory_.size()) > windowSize_) {
        pressureHistory_.erase(pressureHistory_.begin());
        temperatureHistory_.erase(temperatureHistory_.begin());
    }
}

std::string TrendAnalyzer::expectedWeather() const {
    if (static_cast<int>(pressureHistory_.size()) < kMinObservations) {
        throw ForecastNotAvailableException("Для оценки тенденции нужно не меньше двух наблюдений");
    }
    double pressureChange = pressureHistory_.back() - pressureHistory_.front();
    double temperatureChange = temperatureHistory_.back() - temperatureHistory_.front();
    if (pressureChange < -kTrendThresholdHpa) {
        return temperatureChange < 0 ? "ухудшение с похолоданием" : "ухудшение, возможны осадки";
    }
    return pressureChange > kTrendThresholdHpa ? "улучшение погоды" : "без перемен";
}