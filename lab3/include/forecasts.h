#pragma once

#include <string>
#include <vector>

#include "stations.h"

/// Уверенность в прогнозе.
class Confidence {
public:
    /// Бросает InvalidMeasurementException, если оценка вне [0, 100],
    /// и UnsupportedHorizonException, если горизонт не положителен.
    Confidence(double scorePercent, int horizonHours, const std::string& modelName);

    /// Уровень словами: низкая, средняя или высокая.
    std::string level() const;
    bool isReliable() const;

    double getScorePercent() const { return scorePercent_; }
    int getHorizonHours() const { return horizonHours_; }
    const std::string& getModelName() const { return modelName_; }

private:
    double scorePercent_;
    int horizonHours_;
    std::string modelName_;
};

/// Прогностическая модель.
class ForecastModel {
public:
    /// Бросает InvalidMeasurementException, если разрешение или горизонт не положительны.
    ForecastModel(const std::string& name, double resolutionKm, int maxHorizonHours, double bias);

    /// Убирает систематическую ошибку модели из температуры.
    Temperature correct(const Temperature& raw) const;
    bool supportsHorizon(int hours) const;

    const std::string& getName() const { return name_; }
    double getResolutionKm() const { return resolutionKm_; }
    int getMaxHorizonHours() const { return maxHorizonHours_; }
    double getBias() const { return bias_; }

private:
    std::string name_;
    double resolutionKm_;
    int maxHorizonHours_;
    double bias_;
};

/// Прогноз на один час.
class HourlyForecast {
public:
    /// Бросает InvalidMeasurementException, если вероятность осадков вне [0, 100].
    HourlyForecast(const Date& hour, const Temperature& temperature, const Wind& wind,
                   double precipitationChance);

    /// Сухо ли: вероятность осадков ниже 30 процентов.
    bool isDry() const;
    /// Штормовой ли час: сильный ветер или осадки почти наверняка.
    bool isStormy() const;

    const Date& getHour() const { return hour_; }
    const Temperature& getTemperature() const { return temperature_; }
    const Wind& getWind() const { return wind_; }
    double getPrecipitationChance() const { return precipitationChance_; }

private:
    Date hour_;
    Temperature temperature_;
    Wind wind_;
    double precipitationChance_;
};

/// Один расчёт модели от начального состояния атмосферы.
class ModelRun {
public:
    ModelRun(const ForecastModel& model, const WeatherSnapshot& initial, const Date& runAt);

    void addStep(const HourlyForecast& step);
    /// Прогноз на заданное число часов вперёд. Бросает UnsupportedHorizonException,
    /// если модель не считает так далеко, и ForecastNotAvailableException, если шаг ещё не рассчитан.
    const HourlyForecast& forecastAt(int hoursAhead) const;

    const ForecastModel& getModel() const { return model_; }
    const WeatherSnapshot& getInitial() const { return initial_; }
    const Date& getRunAt() const { return runAt_; }
    int getStepCount() const { return static_cast<int>(steps_.size()); }

private:
    ForecastModel model_;
    WeatherSnapshot initial_;
    Date runAt_;
    std::vector<HourlyForecast> steps_;
};

/// Прогноз на сутки из почасовых прогнозов.
class DailyForecast {
public:
    DailyForecast(const Date& date, const ForecastModel& model, const Date& issuedAt);

    /// Бросает ForecastNotAvailableException, если час относится к другому дню.
    void addHour(const HourlyForecast& hour);
    /// Максимальная температура дня в градусах Цельсия с поправкой модели.
    double maxTemperature() const;
    double minTemperature() const;
    /// Наибольшая вероятность осадков за день.
    double peakRainChance() const;

    const Date& getDate() const { return date_; }
    const ForecastModel& getModel() const { return model_; }
    const Date& getIssuedAt() const { return issuedAt_; }
    int getHourCount() const { return static_cast<int>(hours_.size()); }

private:
    Date date_;
    std::vector<HourlyForecast> hours_;
    ForecastModel model_;
    Date issuedAt_;
};

/// Прогноз на неделю.
class WeeklyForecast {
public:
    WeeklyForecast(const City& city, const Confidence& confidence, const Date& startDate);

    /// Бросает CapacityExceededException, если в прогнозе уже семь дней.
    void addDay(const DailyForecast& day);
    /// Самый тёплый день. Бросает ForecastNotAvailableException, если дней нет.
    const DailyForecast& warmestDay() const;
    double averageMaxTemperature() const;

    const City& getCity() const { return city_; }
    const Confidence& getConfidence() const { return confidence_; }
    const Date& getStartDate() const { return startDate_; }

private:
    City city_;
    std::vector<DailyForecast> days_;
    Confidence confidence_;
    Date startDate_;
};

/// Статистика точности прогнозов модели.
class ForecastAccuracy {
public:
    explicit ForecastAccuracy(const std::string& modelName);

    /// Учитывает расхождение прогноза и фактической температуры.
    void record(const Temperature& predicted, const Temperature& actual);
    /// Средняя абсолютная ошибка. Бросает ForecastNotAvailableException, если сравнений ещё не было.
    double meanError() const;

    double getMaxError() const { return maxError_; }
    int getForecastCount() const { return forecastCount_; }

private:
    int forecastCount_;
    double totalError_;
    double maxError_;
    std::string modelName_;
};

/// Бюллетень погоды для города.
class ForecastBulletin {
public:
    ForecastBulletin(const City& city, const DailyForecast& forecast, const Confidence& confidence,
                     const Date& issuedAt);

    /// Текст бюллетеня.
    std::string render() const;
    /// Выпущен ли бюллетень в тот же день, что и момент now.
    bool isActual(const Date& now) const;

    const City& getCity() const { return city_; }

private:
    City city_;
    DailyForecast forecast_;
    Confidence confidence_;
    Date issuedAt_;
};

/// Климатические нормы города.
class Climatology {
public:
    /// Бросает InvalidMeasurementException, если средних по месяцам не двенадцать.
    Climatology(const City& city, const std::vector<double>& monthlyMeans, int yearsOfData);

    /// Норма температуры месяца (1–12) в градусах Цельсия.
    double normalFor(int month) const;
    /// Отклонение наблюдаемой температуры от нормы.
    double anomaly(const Temperature& observed, int month) const;

    const City& getCity() const { return city_; }

private:
    City city_;
    std::vector<double> monthlyMeans_;
    int yearsOfData_;
};

/// Анализ тенденций по последним наблюдениям.
class TrendAnalyzer {
public:
    /// Бросает InvalidMeasurementException, если окно меньше двух наблюдений.
    explicit TrendAnalyzer(int windowSize);

    /// Добавляет наблюдение, храня только последние windowSize штук.
    void addObservation(const Pressure& pressure, const Temperature& temperature);
    /// Ожидаемая тенденция погоды. Бросает ForecastNotAvailableException, если наблюдений меньше двух.
    std::string expectedWeather() const;

private:
    std::vector<double> pressureHistory_;
    std::vector<double> temperatureHistory_;
    int windowSize_;
};