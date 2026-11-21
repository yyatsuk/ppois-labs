#pragma once

#include <string>

/// Дата или момент времени строкой: "ГГГГ-ММ-ДД" или "ГГГГ-ММ-ДД ЧЧ:ММ".
using Date = std::string;

/// Географические координаты точки.
class Coordinates {
public:
    /// Бросает InvalidCoordinatesException, если широта вне [-90, 90] или долгота вне [-180, 180].
    Coordinates(double latitude, double longitude, double altitude);

    /// Расстояние до другой точки по поверхности Земли в километрах (формула гаверсинусов).
    double distanceTo(const Coordinates& other) const;
    /// Находится ли точка в Северном полушарии.
    bool isNorthern() const;

    double getLatitude() const { return latitude_; }
    double getLongitude() const { return longitude_; }
    double getAltitude() const { return altitude_; }

private:
    double latitude_;
    double longitude_;
    double altitude_;
};

/// Ветер.
class Wind {
public:
    /// Бросает InvalidMeasurementException, если скорость или порыв отрицательны,
    /// порыв слабее скорости или направление вне [0, 360).
    Wind(double speedMs, double directionDegrees, double gustMs, const Date& measuredAt);

    /// Сила ветра по шкале Бофорта (0..12).
    int beaufortScale() const;
    /// Направление ветра словами: С, СВ, В, ЮВ, Ю, ЮЗ, З, СЗ.
    std::string compassDirection() const;
    /// Сильный ли ветер (7 баллов по Бофорту и выше).
    bool isStrong() const;

    double getSpeedMs() const { return speedMs_; }
    double getDirectionDegrees() const { return directionDegrees_; }
    double getGustMs() const { return gustMs_; }
    const Date& getMeasuredAt() const { return measuredAt_; }

private:
    double speedMs_;
    double directionDegrees_;
    double gustMs_;
    Date measuredAt_;
};

class Humidity;

/// Единицы измерения температуры.
enum class TemperatureUnit { Celsius, Fahrenheit, Kelvin };

/// Температура воздуха.
class Temperature {
public:
    /// Бросает InvalidMeasurementException, если значение ниже абсолютного нуля.
    Temperature(double value, TemperatureUnit unit, const Date& measuredAt);

    double toCelsius() const;
    double toFahrenheit() const;
    /// Температура по ощущениям в градусах Цельсия (формула Стедмана с учётом ветра и влажности).
    double calculateFeelsLike(const Wind& wind, const Humidity& humidity) const;

    double getValue() const { return value_; }
    TemperatureUnit getUnit() const { return unit_; }
    const Date& getMeasuredAt() const { return measuredAt_; }

private:
    double value_;
    TemperatureUnit unit_;
    Date measuredAt_;
};

/// Относительная влажность воздуха.
class Humidity {
public:
    /// Бросает InvalidMeasurementException, если влажность вне [0, 100] процентов.
    Humidity(double percent, const Date& measuredAt);

    /// Точка росы в градусах Цельсия (формула Магнуса). Бросает InvalidMeasurementException при нулевой влажности.
    double dewPoint(const Temperature& temperature) const;
    /// Оценка комфортности: сухо, комфортно или влажно.
    std::string comfortLevel() const;

    double getPercent() const { return percent_; }
    const Date& getMeasuredAt() const { return measuredAt_; }

private:
    double percent_;
    Date measuredAt_;
};

/// Атмосферное давление.
class Pressure {
public:
    /// Бросает InvalidMeasurementException, если давление вне [300, 1100] гПа.
    Pressure(double hectopascals, const Date& measuredAt, double stationAltitude);

    double toMillimetersOfMercury() const;
    /// Давление, приведённое к уровню моря, в гектопаскалях.
    double seaLevelEquivalent() const;
    /// Низкое ли давление (признак ненастья).
    bool isStormy() const;

    double getHectopascals() const { return hectopascals_; }
    const Date& getMeasuredAt() const { return measuredAt_; }
    double getStationAltitude() const { return stationAltitude_; }

private:
    double hectopascals_;
    Date measuredAt_;
    double stationAltitude_;
};

/// Вид осадков.
enum class PrecipitationType { None, Rain, Snow, Hail };

/// Осадки.
class Precipitation {
public:
    /// Бросает InvalidMeasurementException, если количество отрицательно, а длительность не положительна.
    /// Для отсутствия осадков используйте PrecipitationType::None, количество 0 и длительность 1 час.
    Precipitation(PrecipitationType type, double amountMm, double durationHours, const Date& startedAt);

    /// Интенсивность в миллиметрах в час.
    double intensity() const;
    /// Сильные ли осадки (интенсивность от 4 мм/ч).
    bool isHeavy() const;

    PrecipitationType getType() const { return type_; }
    double getAmountMm() const { return amountMm_; }
    double getDurationHours() const { return durationHours_; }
    const Date& getStartedAt() const { return startedAt_; }

private:
    PrecipitationType type_;
    double amountMm_;
    double durationHours_;
    Date startedAt_;
};

/// Облачность.
class CloudCover {
public:
    /// Бросает InvalidMeasurementException, если покрытие вне [0, 100] процентов или высота отрицательна.
    CloudCover(double percent, double baseHeightM, const Date& measuredAt);

    /// Описание словами: ясно, малооблачно, облачно или пасмурно.
    std::string describe() const;
    /// Пасмурно ли (покрытие от 90 процентов).
    bool isOvercast() const;

    double getPercent() const { return percent_; }
    double getBaseHeightM() const { return baseHeightM_; }

private:
    double percent_;
    double baseHeightM_;
    Date measuredAt_;
};

/// Видимость.
class Visibility {
public:
    /// Бросает InvalidMeasurementException, если видимость отрицательна.
    Visibility(double meters, const std::string& cause, const Date& measuredAt);

    double toKilometers() const;
    /// Туман ли (видимость меньше километра).
    bool isFoggy() const;

    double getMeters() const { return meters_; }
    const std::string& getCause() const { return cause_; }

private:
    double meters_;
    std::string cause_;
    Date measuredAt_;
};

/// Ультрафиолетовый индекс.
class UvIndex {
public:
    /// Бросает InvalidMeasurementException, если индекс вне [0, 16].
    UvIndex(double value, const Date& measuredAt);

    /// Уровень риска: низкий, умеренный, высокий, очень высокий или экстремальный.
    std::string riskLevel() const;
    /// Совет по защите от солнца для текущего уровня риска.
    std::string protectionAdvice() const;

    double getValue() const { return value_; }

private:
    double value_;
    Date measuredAt_;
};

/// Срез погоды: все основные величины, измеренные в одной точке в один момент.
class WeatherSnapshot {
public:
    WeatherSnapshot(const Temperature& temperature, const Pressure& pressure,
                    const Humidity& humidity, const Wind& wind, const Precipitation& precipitation,
                    const CloudCover& cloudCover);

    /// Идёт ли дождь.
    bool isRainy() const;
    /// Индекс комфорта от 0 до 100: чем ближе ощущаемая температура и влажность к идеальным, тем выше.
    double comfortIndex() const;
    /// Краткое текстовое описание погоды.
    std::string summary() const;

    const Temperature& getTemperature() const { return temperature_; }
    const Pressure& getPressure() const { return pressure_; }
    const Humidity& getHumidity() const { return humidity_; }
    const Wind& getWind() const { return wind_; }
    const Precipitation& getPrecipitation() const { return precipitation_; }
    const CloudCover& getCloudCover() const { return cloudCover_; }

private:
    Temperature temperature_;
    Pressure pressure_;
    Humidity humidity_;
    Wind wind_;
    Precipitation precipitation_;
    CloudCover cloudCover_;
};