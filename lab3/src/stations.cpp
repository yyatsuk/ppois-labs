#include "stations.h"

#include <algorithm>
#include <cmath>

#include "exceptions.h"

namespace {

const double kPi = 3.14159265358979323846;
const double kMinPercent = 0.0;
const double kMaxPercent = 100.0;
const double kStandardHeightM = 10.0;
const double kShearExponent = 0.143;
const int kDateLength = 10;
const int kYearLength = 4;
const int kMonthDayStart = 5;
const int kMonthDayLength = 5;
const int kMegacityPopulation = 1000000;

void requirePositive(double value, const std::string& message) {
    if (value <= 0) {
        throw InvalidMeasurementException(message);
    }
}

int yearOf(const Date& date) {
    return std::stoi(date.substr(0, kYearLength));
}

}  // namespace

// ----- Sensor -----

Sensor::Sensor(const std::string& id, const std::string& model, const Date& installedAt)
    : id_(id), model_(model), installedAt_(installedAt), faulty_(false) {}

void Sensor::calibrate(const Date& date) {
    calibratedAt_ = date;
}

void Sensor::markFaulty() {
    faulty_ = true;
}

bool Sensor::isReady() const {
    return !faulty_ && !calibratedAt_.empty();
}

void Sensor::ensureReady() const {
    if (faulty_) {
        throw SensorMalfunctionException("Датчик " + id_ + " неисправен");
    }
    if (calibratedAt_.empty()) {
        throw SensorNotCalibratedException("Датчик " + id_ + " не откалиброван");
    }
}

// ----- TemperatureSensor -----

TemperatureSensor::TemperatureSensor(const std::string& id, const std::string& model,
                                     const Date& installedAt, TemperatureUnit unit)
    : Sensor(id, model, installedAt), unit_(unit), offset_(0.0) {}

Temperature TemperatureSensor::read(double rawValue, const Date& at) const {
    ensureReady();
    return Temperature(rawValue + offset_, unit_, at);
}

void TemperatureSensor::adjustOffset(double referenceValue, double rawValue, const Date& today) {
    offset_ = referenceValue - rawValue;
    calibrate(today);
}

// ----- PressureSensor -----

PressureSensor::PressureSensor(const std::string& id, const std::string& model, const Date& installedAt,
                               double stationAltitude)
    : Sensor(id, model, installedAt), stationAltitude_(stationAltitude), offset_(0.0) {}

Pressure PressureSensor::read(double rawHectopascals, const Date& at) const {
    ensureReady();
    return Pressure(rawHectopascals + offset_, at, stationAltitude_);
}

void PressureSensor::adjustOffset(double referenceValue, double rawValue, const Date& today) {
    offset_ = referenceValue - rawValue;
    calibrate(today);
}

// ----- HumiditySensor -----

HumiditySensor::HumiditySensor(const std::string& id, const std::string& model, const Date& installedAt,
                               double sensitivity)
    : Sensor(id, model, installedAt), sensitivity_(sensitivity), offset_(0.0) {
    requirePositive(sensitivity, "Чувствительность гигрометра должна быть положительной");
}

Humidity HumiditySensor::read(double rawPercent, const Date& at) const {
    ensureReady();
    double percent = std::clamp(rawPercent * sensitivity_ + offset_, kMinPercent, kMaxPercent);
    return Humidity(percent, at);
}

void HumiditySensor::adjustOffset(double referenceValue, double rawValue, const Date& today) {
    offset_ = referenceValue - rawValue * sensitivity_;
    calibrate(today);
}

// ----- WindSensor -----

WindSensor::WindSensor(const std::string& id, const std::string& model, const Date& installedAt,
                       double heightM)
    : Sensor(id, model, installedAt), heightM_(heightM), offset_(0.0) {
    requirePositive(heightM, "Высота установки анемометра должна быть положительной");
}

Wind WindSensor::read(double rawSpeedMs, double directionDegrees, double rawGustMs, const Date& at) const {
    ensureReady();
    double factor = std::pow(kStandardHeightM / heightM_, kShearExponent);
    double speed = std::max(0.0, rawSpeedMs + offset_) * factor;
    double gust = std::max(speed, (rawGustMs + offset_) * factor);
    return Wind(speed, directionDegrees, gust, at);
}

void WindSensor::adjustOffset(double referenceValue, double rawValue, const Date& today) {
    offset_ = referenceValue - rawValue;
    calibrate(today);
}

// ----- Calibration -----

Calibration::Calibration(const std::string& sensorId, const Date& performedAt, double offset,
                         const std::string& technician)
    : sensorId_(sensorId), performedAt_(performedAt), offset_(offset), technician_(technician) {
    if (static_cast<int>(performedAt.size()) < kDateLength) {
        throw InvalidMeasurementException("Дата калибровки должна быть в формате ГГГГ-ММ-ДД");
    }
}

double Calibration::apply(double rawValue) const {
    return rawValue + offset_;
}

bool Calibration::isExpired(const Date& today) const {
    int years = yearOf(today) - yearOf(performedAt_);
    bool anniversaryPassed = today.substr(kMonthDayStart, kMonthDayLength) >=
                             performedAt_.substr(kMonthDayStart, kMonthDayLength);
    return years > 1 || (years == 1 && anniversaryPassed);
}

// ----- Region -----

Region::Region(const std::string& name, const std::string& country, const Coordinates& center,
               double radiusKm)
    : name_(name), country_(country), center_(center), radiusKm_(radiusKm) {
    if (radiusKm <= 0) {
        throw InvalidCoordinatesException("Радиус региона должен быть положительным");
    }
}

bool Region::contains(const Coordinates& point) const {
    return center_.distanceTo(point) <= radiusKm_;
}

double Region::areaKm2() const {
    return kPi * radiusKm_ * radiusKm_;
}

// ----- City -----

City::City(const std::string& name, int population, const Coordinates& location, const Region& region)
    : name_(name), population_(population), location_(location), region_(region) {}

bool City::isInRegion() const {
    return region_.contains(location_);
}

bool City::isMegacity() const {
    return population_ >= kMegacityPopulation;
}

// ----- WeatherStation -----

WeatherStation::WeatherStation(const std::string& id, const std::string& name, const Coordinates& location,
                               const TemperatureSensor& temperatureSensor,
                               const PressureSensor& pressureSensor, const HumiditySensor& humiditySensor,
                               const WindSensor& windSensor)
    : id_(id),
      name_(name),
      location_(location),
      temperatureSensor_(temperatureSensor),
      pressureSensor_(pressureSensor),
      humiditySensor_(humiditySensor),
      windSensor_(windSensor),
      active_(true) {}

WeatherSnapshot WeatherStation::measure(const RawReadings& raw, const Precipitation& precipitation,
                                        const CloudCover& cloudCover, const Date& at) const {
    if (!active_) {
        throw SensorMalfunctionException("Станция " + id_ + " отключена");
    }
    return WeatherSnapshot(temperatureSensor_.read(raw.temperature, at),
                           pressureSensor_.read(raw.pressure, at), humiditySensor_.read(raw.humidity, at),
                           windSensor_.read(raw.windSpeed, raw.windDirection, raw.gust, at),
                           precipitation, cloudCover);
}

bool WeatherStation::isOperational() const {
    return active_ && temperatureSensor_.isReady() && pressureSensor_.isReady() &&
           humiditySensor_.isReady() && windSensor_.isReady();
}

void WeatherStation::deactivate() {
    active_ = false;
}

// ----- StationNetwork -----

StationNetwork::StationNetwork(const std::string& name, const Region& region, int capacity)
    : name_(name), region_(region), capacity_(capacity) {
    requirePositive(capacity, "Ёмкость сети должна быть положительной");
}

void StationNetwork::addStation(const WeatherStation& station) {
    if (getStationCount() >= capacity_) {
        throw CapacityExceededException("Сеть " + name_ + " заполнена");
    }
    if (!region_.contains(station.getLocation())) {
        throw InvalidCoordinatesException("Станция " + station.getId() + " вне региона сети");
    }
    stations_.push_back(station);
}

const WeatherStation& StationNetwork::findStation(const std::string& id) const {
    auto found = std::find_if(stations_.begin(), stations_.end(),
                              [&id](const WeatherStation& station) { return station.getId() == id; });
    if (found == stations_.end()) {
        throw StationNotFoundException("Станция " + id + " не найдена");
    }
    return *found;
}

const WeatherStation& StationNetwork::nearestStation(const Coordinates& point) const {
    if (stations_.empty()) {
        throw StationNotFoundException("В сети " + name_ + " нет станций");
    }
    return *std::min_element(stations_.begin(), stations_.end(),
                             [&point](const WeatherStation& left, const WeatherStation& right) {
                                 return left.getLocation().distanceTo(point) <
                                        right.getLocation().distanceTo(point);
                             });
}