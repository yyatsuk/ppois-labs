#pragma once

#include <string>
#include <vector>

#include "measurements.h"

/// Датчик: общая часть всех приборов станции.
class Sensor {
public:
    Sensor(const std::string& id, const std::string& model, const Date& installedAt);
    virtual ~Sensor() = default;

    /// Фиксирует дату последней калибровки.
    void calibrate(const Date& date);
    /// Помечает датчик неисправным.
    void markFaulty();
    /// Исправен ли и откалиброван ли датчик.
    bool isReady() const;
    /// Бросает SensorMalfunctionException для неисправного и SensorNotCalibratedException для некалиброванного.
    void ensureReady() const;

    const std::string& getId() const { return id_; }
    const std::string& getModel() const { return model_; }
    const Date& getInstalledAt() const { return installedAt_; }
    const Date& getCalibratedAt() const { return calibratedAt_; }

private:
    std::string id_;
    std::string model_;
    Date installedAt_;
    Date calibratedAt_;
    bool faulty_;
};

/// Термометр.
class TemperatureSensor : public Sensor {
public:
    TemperatureSensor(const std::string& id, const std::string& model, const Date& installedAt,
                      TemperatureUnit unit);

    /// Превращает сырое показание в измерение с учётом поправки.
    Temperature read(double rawValue, const Date& at) const;
    /// Подбирает поправку по эталонному значению и калибрует датчик.
    void adjustOffset(double referenceValue, double rawValue, const Date& today);

private:
    TemperatureUnit unit_;
    double offset_;
};

/// Барометр.
class PressureSensor : public Sensor {
public:
    PressureSensor(const std::string& id, const std::string& model, const Date& installedAt,
                   double stationAltitude);

    Pressure read(double rawHectopascals, const Date& at) const;
    void adjustOffset(double referenceValue, double rawValue, const Date& today);

private:
    double stationAltitude_;
    double offset_;
};

/// Гигрометр.
class HumiditySensor : public Sensor {
public:
    /// Бросает InvalidMeasurementException, если чувствительность не положительна.
    HumiditySensor(const std::string& id, const std::string& model, const Date& installedAt,
                   double sensitivity);

    /// Показание ограничивается диапазоном 0–100 процентов.
    Humidity read(double rawPercent, const Date& at) const;
    void adjustOffset(double referenceValue, double rawValue, const Date& today);

private:
    double sensitivity_;
    double offset_;
};

/// Анемометр.
class WindSensor : public Sensor {
public:
    /// Бросает InvalidMeasurementException, если высота установки не положительна.
    WindSensor(const std::string& id, const std::string& model, const Date& installedAt, double heightM);

    /// Скорость приводится к стандартной высоте 10 м.
    Wind read(double rawSpeedMs, double directionDegrees, double rawGustMs, const Date& at) const;
    void adjustOffset(double referenceValue, double rawValue, const Date& today);

private:
    double heightM_;
    double offset_;
};

/// Запись о калибровке датчика.
class Calibration {
public:
    /// Бросает InvalidMeasurementException, если дата короче формата ГГГГ-ММ-ДД.
    Calibration(const std::string& sensorId, const Date& performedAt, double offset,
                const std::string& technician);

    /// Применяет поправку к сырому показанию.
    double apply(double rawValue) const;
    /// Истёк ли годовой срок действия калибровки.
    bool isExpired(const Date& today) const;

    const std::string& getSensorId() const { return sensorId_; }
    const Date& getPerformedAt() const { return performedAt_; }
    double getOffset() const { return offset_; }
    const std::string& getTechnician() const { return technician_; }

private:
    std::string sensorId_;
    Date performedAt_;
    double offset_;
    std::string technician_;
};

/// Регион: круглая область вокруг центра.
class Region {
public:
    /// Бросает InvalidCoordinatesException, если радиус не положителен.
    Region(const std::string& name, const std::string& country, const Coordinates& center, double radiusKm);

    /// Попадает ли точка в регион.
    bool contains(const Coordinates& point) const;
    double areaKm2() const;

    const std::string& getName() const { return name_; }
    const std::string& getCountry() const { return country_; }
    const Coordinates& getCenter() const { return center_; }

private:
    std::string name_;
    std::string country_;
    Coordinates center_;
    double radiusKm_;
};

/// Город.
class City {
public:
    City(const std::string& name, int population, const Coordinates& location, const Region& region);

    /// Лежит ли город в своём регионе.
    bool isInRegion() const;
    /// Город-миллионник ли.
    bool isMegacity() const;

    const std::string& getName() const { return name_; }
    int getPopulation() const { return population_; }
    const Coordinates& getLocation() const { return location_; }
    const Region& getRegion() const { return region_; }

private:
    std::string name_;
    int population_;
    Coordinates location_;
    Region region_;
};

/// Сырые показания приборов станции.
struct RawReadings {
    double temperature;
    double pressure;
    double humidity;
    double windSpeed;
    double windDirection;
    double gust;
};

/// Метеостанция с набором датчиков.
class WeatherStation {
public:
    WeatherStation(const std::string& id, const std::string& name, const Coordinates& location,
                   const TemperatureSensor& temperatureSensor, const PressureSensor& pressureSensor,
                   const HumiditySensor& humiditySensor, const WindSensor& windSensor);

    /// Собирает срез погоды. Бросает SensorMalfunctionException, если станция отключена,
    /// и исключения датчиков, если прибор неисправен или не откалиброван.
    WeatherSnapshot measure(const RawReadings& raw, const Precipitation& precipitation,
                            const CloudCover& cloudCover, const Date& at) const;
    /// Работает ли станция: включена и все датчики готовы.
    bool isOperational() const;
    void deactivate();

    const std::string& getId() const { return id_; }
    const std::string& getName() const { return name_; }
    const Coordinates& getLocation() const { return location_; }

private:
    std::string id_;
    std::string name_;
    Coordinates location_;
    TemperatureSensor temperatureSensor_;
    PressureSensor pressureSensor_;
    HumiditySensor humiditySensor_;
    WindSensor windSensor_;
    bool active_;
};

/// Сеть станций региона.
class StationNetwork {
public:
    /// Бросает InvalidMeasurementException, если ёмкость не положительна.
    StationNetwork(const std::string& name, const Region& region, int capacity);

    /// Бросает CapacityExceededException при переполнении и InvalidCoordinatesException,
    /// если станция вне региона сети.
    void addStation(const WeatherStation& station);
    /// Бросает StationNotFoundException, если станции с таким идентификатором нет.
    const WeatherStation& findStation(const std::string& id) const;
    /// Ближайшая к точке станция. Бросает StationNotFoundException, если сеть пуста.
    const WeatherStation& nearestStation(const Coordinates& point) const;

    const std::string& getName() const { return name_; }
    int getStationCount() const { return static_cast<int>(stations_.size()); }

private:
    std::string name_;
    Region region_;
    std::vector<WeatherStation> stations_;
    int capacity_;
};