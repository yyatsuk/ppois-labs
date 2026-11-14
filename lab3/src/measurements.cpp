#include "measurements.h"
#include "exceptions.h"

#include <algorithm>
#include <cmath>
#include <iomanip>
#include <limits>
#include <sstream>


namespace {

const double kPi = 3.14159265358979323846;
const double kDegreesInHalfTurn = 180.0;
const double kEarthDiameterKm = 12742.0;
const double kHalf = 0.5;
const double kMaxLatitude = 90.0;
const double kMaxLongitude = 180.0;
const double kPercentDivisor = 100.0;
const double kMaxPercent = 100.0;

const double kKelvinOffset = 273.15;
const double kFahrenheitOffset = 32.0;
const double kFahrenheitRatio = 1.8;
const double kVaporBase = 6.105;
const double kVaporA = 17.27;
const double kVaporB = 237.7;
const double kVaporWeight = 0.33;
const double kWindWeight = 0.70;
const double kApparentShift = 4.0;

const double kMagnusA = 17.62;
const double kMagnusB = 243.12;
const double kDryLimit = 30.0;
const double kComfortLimit = 60.0;

const double kMinPressure = 300.0;
const double kMaxPressure = 1100.0;
const double kHpaToMmHg = 0.750062;
const double kLapseRate = 0.0065;
const double kStandardTemperatureK = 288.15;
const double kBarometricExponent = -5.255;
const double kStormyPressure = 1000.0;

const double kMaxDirection = 360.0;
const double kDirectionStep = 45.0;
const int kCompassPoints = 8;
const int kStrongBeaufort = 7;
const int kMaxBeaufort = 12;
const double kBeaufortLimits[] = {0.3, 1.6, 3.4, 5.5, 8.0, 10.8, 13.9, 17.2, 20.8, 24.5, 28.5, 32.7};
const char* const kCompassNames[] = {"С", "СВ", "В", "ЮВ", "Ю", "ЮЗ", "З", "СЗ"};

const double kHeavyIntensity = 4.0;
const double kClearLimit = 10.0;
const double kPartlyCloudyLimit = 50.0;
const double kCloudyLimit = 90.0;
const double kFogLimitMeters = 1000.0;
const double kMetersInKilometer = 1000.0;
const double kMaxUvIndex = 16.0;

const double kIdealTemperature = 21.0;
const double kIdealHumidity = 50.0;
const double kTemperaturePenalty = 3.0;
const double kHumidityPenalty = 0.4;
const double kMaxComfort = 100.0;
const int kSummaryDecimals = 1;

struct UvBand {
    double upperLimit;
    const char* risk;
    const char* advice;
};

const UvBand kUvBands[] = {
    {3.0, "низкий", "защита не требуется"},
    {6.0, "умеренный", "используйте солнцезащитный крем"},
    {8.0, "высокий", "носите головной убор и очки, избегайте солнца в полдень"},
    {11.0, "очень высокий", "ограничьте пребывание на солнце, ищите тень"},
    {std::numeric_limits<double>::infinity(), "экстремальный", "избегайте пребывания на открытом солнце"},
};

double toRadians(double degrees) {
    return degrees * kPi / kDegreesInHalfTurn;
}

void requireRange(double value, double minimum, double maximum, const std::string& message) {
    if (value < minimum || value > maximum) {
        throw InvalidMeasurementException(message);
    }
}

double convertToCelsius(double value, TemperatureUnit unit) {
    switch (unit) {
        case TemperatureUnit::Fahrenheit:
            return (value - kFahrenheitOffset) / kFahrenheitRatio;
        case TemperatureUnit::Kelvin:
            return value - kKelvinOffset;
        default:
            return value;
    }
}

const UvBand& findUvBand(double value) {
    for (const UvBand& band : kUvBands) {
        if (value < band.upperLimit) {
            return band;
        }
    }
    return kUvBands[sizeof(kUvBands) / sizeof(kUvBands[0]) - 1];
}

}  // namespace

// ----- Coordinates -----

Coordinates::Coordinates(double latitude, double longitude, double altitude)
    : latitude_(latitude), longitude_(longitude), altitude_(altitude) {
    if (std::fabs(latitude) > kMaxLatitude) {
        throw InvalidCoordinatesException("Широта должна быть от -90 до 90 градусов");
    }
    if (std::fabs(longitude) > kMaxLongitude) {
        throw InvalidCoordinatesException("Долгота должна быть от -180 до 180 градусов");
    }
}

double Coordinates::distanceTo(const Coordinates& other) const {
    double firstLatitude = toRadians(latitude_);
    double secondLatitude = toRadians(other.latitude_);
    double halfLatitudeDelta = (secondLatitude - firstLatitude) * kHalf;
    double halfLongitudeDelta = toRadians(other.longitude_ - longitude_) * kHalf;
    double haversine = std::pow(std::sin(halfLatitudeDelta), 2) +
                       std::cos(firstLatitude) * std::cos(secondLatitude) *
                           std::pow(std::sin(halfLongitudeDelta), 2);
    return kEarthDiameterKm * std::asin(std::sqrt(haversine));
}

bool Coordinates::isNorthern() const {
    return latitude_ >= 0;
}

// ----- Wind -----

Wind::Wind(double speedMs, double directionDegrees, double gustMs, const Date& measuredAt)
    : speedMs_(speedMs), directionDegrees_(directionDegrees), gustMs_(gustMs), measuredAt_(measuredAt) {
    if (speedMs < 0 || gustMs < speedMs) {
        throw InvalidMeasurementException("Скорость ветра отрицательна или порыв слабее ветра");
    }
    if (directionDegrees < 0 || directionDegrees >= kMaxDirection) {
        throw InvalidMeasurementException("Направление ветра должно быть от 0 до 360 градусов");
    }
}

int Wind::beaufortScale() const {
    for (int scale = 0; scale < kMaxBeaufort; ++scale) {
        if (speedMs_ < kBeaufortLimits[scale]) {
            return scale;
        }
    }
    return kMaxBeaufort;
}

std::string Wind::compassDirection() const {
    int index = static_cast<int>(std::lround(directionDegrees_ / kDirectionStep)) % kCompassPoints;
    return kCompassNames[index];
}

bool Wind::isStrong() const {
    return beaufortScale() >= kStrongBeaufort;
}

// ----- Temperature -----

Temperature::Temperature(double value, TemperatureUnit unit, const Date& measuredAt)
    : value_(value), unit_(unit), measuredAt_(measuredAt) {
    if (convertToCelsius(value, unit) < -kKelvinOffset) {
        throw InvalidMeasurementException("Температура ниже абсолютного нуля");
    }
}

double Temperature::toCelsius() const {
    return convertToCelsius(value_, unit_);
}

double Temperature::toFahrenheit() const {
    return toCelsius() * kFahrenheitRatio + kFahrenheitOffset;
}

double Temperature::calculateFeelsLike(const Wind& wind, const Humidity& humidity) const {
    double celsius = toCelsius();
    double relativeHumidity = humidity.getPercent() / kPercentDivisor;
    double vaporPressure =
        relativeHumidity * kVaporBase * std::exp(kVaporA * celsius / (kVaporB + celsius));
    return celsius + kVaporWeight * vaporPressure - kWindWeight * wind.getSpeedMs() - kApparentShift;
}

// ----- Humidity -----

Humidity::Humidity(double percent, const Date& measuredAt)
    : percent_(percent), measuredAt_(measuredAt) {
    requireRange(percent, 0.0, kMaxPercent, "Влажность должна быть от 0 до 100 процентов");
}

double Humidity::dewPoint(const Temperature& temperature) const {
    if (percent_ <= 0) {
        throw InvalidMeasurementException("Точку росы нельзя вычислить при нулевой влажности");
    }
    double celsius = temperature.toCelsius();
    double gamma = std::log(percent_ / kPercentDivisor) + kMagnusA * celsius / (kMagnusB + celsius);
    return kMagnusB * gamma / (kMagnusA - gamma);
}

std::string Humidity::comfortLevel() const {
    if (percent_ < kDryLimit) {
        return "сухо";
    }
    return percent_ <= kComfortLimit ? "комфортно" : "влажно";
}

// ----- Pressure -----

Pressure::Pressure(double hectopascals, const Date& measuredAt, double stationAltitude)
    : hectopascals_(hectopascals), measuredAt_(measuredAt), stationAltitude_(stationAltitude) {
    requireRange(hectopascals, kMinPressure, kMaxPressure, "Давление должно быть от 300 до 1100 гПа");
}

double Pressure::toMillimetersOfMercury() const {
    return hectopascals_ * kHpaToMmHg;
}

double Pressure::seaLevelEquivalent() const {
    double heightFactor = 1.0 - kLapseRate * stationAltitude_ / kStandardTemperatureK;
    return hectopascals_ * std::pow(heightFactor, kBarometricExponent);
}

bool Pressure::isStormy() const {
    return hectopascals_ < kStormyPressure;
}

// ----- Precipitation -----

Precipitation::Precipitation(PrecipitationType type, double amountMm, double durationHours,
                             const Date& startedAt)
    : type_(type), amountMm_(amountMm), durationHours_(durationHours), startedAt_(startedAt) {
    if (amountMm < 0 || durationHours <= 0) {
        throw InvalidMeasurementException("Количество осадков отрицательно или длительность не положительна");
    }
}

double Precipitation::intensity() const {
    return amountMm_ / durationHours_;
}

bool Precipitation::isHeavy() const {
    return type_ != PrecipitationType::None && intensity() >= kHeavyIntensity;
}

// ----- CloudCover -----

CloudCover::CloudCover(double percent, double baseHeightM, const Date& measuredAt)
    : percent_(percent), baseHeightM_(baseHeightM), measuredAt_(measuredAt) {
    requireRange(percent, 0.0, kMaxPercent, "Облачность должна быть от 0 до 100 процентов");
    if (baseHeightM < 0) {
        throw InvalidMeasurementException("Высота облаков не может быть отрицательной");
    }
}

std::string CloudCover::describe() const {
    if (percent_ < kClearLimit) {
        return "ясно";
    }
    if (percent_ < kPartlyCloudyLimit) {
        return "малооблачно";
    }
    return percent_ < kCloudyLimit ? "облачно" : "пасмурно";
}

bool CloudCover::isOvercast() const {
    return percent_ >= kCloudyLimit;
}

// ----- Visibility -----

Visibility::Visibility(double meters, const std::string& cause, const Date& measuredAt)
    : meters_(meters), cause_(cause), measuredAt_(measuredAt) {
    if (meters < 0) {
        throw InvalidMeasurementException("Видимость не может быть отрицательной");
    }
}

double Visibility::toKilometers() const {
    return meters_ / kMetersInKilometer;
}

bool Visibility::isFoggy() const {
    return meters_ < kFogLimitMeters;
}

// ----- UvIndex -----

UvIndex::UvIndex(double value, const Date& measuredAt) : value_(value), measuredAt_(measuredAt) {
    requireRange(value, 0.0, kMaxUvIndex, "УФ-индекс должен быть от 0 до 16");
}

std::string UvIndex::riskLevel() const {
    return findUvBand(value_).risk;
}

std::string UvIndex::protectionAdvice() const {
    return findUvBand(value_).advice;
}

// ----- WeatherSnapshot -----

WeatherSnapshot::WeatherSnapshot(const Temperature& temperature, const Pressure& pressure,
                                 const Humidity& humidity, const Wind& wind,
                                 const Precipitation& precipitation, const CloudCover& cloudCover)
    : temperature_(temperature),
      pressure_(pressure),
      humidity_(humidity),
      wind_(wind),
      precipitation_(precipitation),
      cloudCover_(cloudCover) {}

bool WeatherSnapshot::isRainy() const {
    return precipitation_.getType() == PrecipitationType::Rain;
}

double WeatherSnapshot::comfortIndex() const {
    double feelsLike = temperature_.calculateFeelsLike(wind_, humidity_);
    double temperatureLoss = std::fabs(feelsLike - kIdealTemperature) * kTemperaturePenalty;
    double humidityLoss = std::fabs(humidity_.getPercent() - kIdealHumidity) * kHumidityPenalty;
    return std::clamp(kMaxComfort - temperatureLoss - humidityLoss, 0.0, kMaxComfort);
}

std::string WeatherSnapshot::summary() const {
    std::ostringstream text;
    text << std::fixed << std::setprecision(kSummaryDecimals) << "Температура "
         << temperature_.toCelsius() << " °C, ветер " << wind_.getSpeedMs() << " м/с ("
         << wind_.compassDirection() << "), давление " << pressure_.getHectopascals()
         << " гПа, влажность " << humidity_.getPercent() << "%, " << cloudCover_.describe();
    return text.str();
}