#pragma once

#include <string>
#include <vector>

#include "alerts.h"
#include "forecasts.h"

/// Пользователь сервиса.
class User {
public:
    /// Бросает InvalidUserDataException, если идентификатор или имя пусты, а адрес почты без символа «@».
    User(const std::string& id, const std::string& name, const std::string& email, const City& city,
         const Date& registeredAt);

    std::string greeting() const;
    /// Домен почтового адреса.
    std::string emailDomain() const;

    const std::string& getId() const { return id_; }
    const std::string& getName() const { return name_; }
    const std::string& getEmail() const { return email_; }
    const City& getCity() const { return city_; }

private:
    std::string id_;
    std::string name_;
    std::string email_;
    City city_;
    Date registeredAt_;
};

/// Подписка пользователя на предупреждения региона.
class Subscription {
public:
    Subscription(const User& user, const Region& region, const Severity& minSeverity);

    /// Нужно ли отправить предупреждение: подписка активна, регион совпадает, степень не ниже порога.
    bool wants(const Alert& alert) const;
    void pause();
    void resume();

    const User& getUser() const { return user_; }
    const Region& getRegion() const { return region_; }
    const Severity& getMinSeverity() const { return minSeverity_; }

private:
    User user_;
    Region region_;
    Severity minSeverity_;
    bool active_;
};

/// Уведомление пользователю о предупреждении.
class Notification {
public:
    Notification(const User& recipient, const Alert& alert, const Date& createdAt);

    /// Отмечает уведомление доставленным. Бросает NotificationFailedException при повторной доставке.
    void deliver();
    /// Текст для показа получателю.
    std::string format() const;

    const User& getRecipient() const { return recipient_; }
    const std::string& getText() const { return text_; }
    const Date& getCreatedAt() const { return createdAt_; }

private:
    User recipient_;
    std::string text_;
    Date createdAt_;
    bool delivered_;
};

/// Источник метеоданных.
class DataSource {
public:
    DataSource(const std::string& name, const Region& coverage, bool online);

    /// Лежит ли точка в зоне покрытия источника.
    bool covers(const Coordinates& point) const;
    /// Бросает DataSourceUnavailableException, если источник отключён.
    void ensureOnline() const;
    void setOnline(bool online) { online_ = online; }

    const std::string& getName() const { return name_; }

private:
    std::string name_;
    Region coverage_;
    bool online_;
};

/// Поставщик данных: выбирает источник для точки.
class DataProvider {
public:
    explicit DataProvider(const std::string& name);

    void addSource(const DataSource& source);
    /// Источник для точки. Бросает DataSourceUnavailableException, если точку никто не покрывает
    /// или подходящий источник отключён.
    const DataSource& sourceFor(const Coordinates& point);

    const std::string& getName() const { return name_; }
    int getRequestsServed() const { return requestsServed_; }

private:
    std::string name_;
    std::vector<DataSource> sources_;
    int requestsServed_;
};

/// Сервис прогноза: наблюдения, суточные прогнозы и проверка опасных явлений.
class ForecastService {
public:
    ForecastService(const std::string& name, const StationNetwork& network, const ForecastModel& model,
                    const AlertCenter& alertCenter);

    /// Срез погоды по ближайшей к точке станции.
    WeatherSnapshot observe(const Coordinates& point, const RawReadings& raw,
                            const Precipitation& precipitation, const CloudCover& cloudCover,
                            const Date& now) const;
    /// Прогноз на сутки от текущего среза. Бросает UnsupportedHorizonException,
    /// если модель не считает на 24 часа.
    DailyForecast forecastDay(const WeatherSnapshot& start, const Date& date) const;
    /// Выпускает предупреждения по правилам центра.
    std::vector<AlertPtr> checkAlerts(const WeatherSnapshot& snapshot, const Region& region,
                                      const Date& now);

    const std::string& getName() const { return name_; }

private:
    std::string name_;
    StationNetwork network_;
    ForecastModel model_;
    AlertCenter alertCenter_;
};

/// Менеджер подписок: рассылает предупреждения подписчикам.
class SubscriptionManager {
public:
    /// Бросает InvalidUserDataException, если лимит подписок не положителен.
    explicit SubscriptionManager(int maxPerUser);

    /// Бросает CapacityExceededException, если у пользователя уже максимум подписок.
    void subscribe(const Subscription& subscription);
    /// Рассылает предупреждение всем подходящим подписчикам.
    std::vector<Notification> dispatch(const Alert& alert, const Date& now);
    int countFor(const User& user) const;

    int getNotificationCount() const { return static_cast<int>(notifications_.size()); }

private:
    std::vector<Subscription> subscriptions_;
    std::vector<Notification> notifications_;
    int maxPerUser_;
};

/// Генератор текстовых отчётов.
class ReportGenerator {
public:
    /// Бросает InvalidUserDataException, если число знаков отрицательно.
    ReportGenerator(const std::string& title, const std::string& author, const Date& generatedAt,
                    int decimals);

    std::string dailyReport(const City& city, const DailyForecast& forecast) const;
    /// Отчёт о точности модели. Бросает ForecastNotAvailableException, если сравнений ещё не было.
    std::string accuracyReport(const ForecastAccuracy& accuracy) const;

    const std::string& getTitle() const { return title_; }

private:
    std::string title_;
    std::string author_;
    Date generatedAt_;
    int decimals_;
};

/// Избранные города пользователя.
class FavoriteLocations {
public:
    /// Бросает InvalidUserDataException, если лимит не положителен.
    FavoriteLocations(const User& owner, int maxCount);

    /// Бросает CapacityExceededException при переполнении и InvalidUserDataException для повтора.
    void add(const City& city);
    /// Бросает InvalidUserDataException, если города нет в избранном.
    void remove(const std::string& cityName);
    bool contains(const std::string& cityName) const;

    const User& getOwner() const { return owner_; }
    int getCount() const { return static_cast<int>(cities_.size()); }

private:
    User owner_;
    std::vector<City> cities_;
    int maxCount_;
};

/// Архив наблюдений.
class ObservationArchive {
public:
    /// Бросает InvalidMeasurementException, если ёмкость не положительна.
    ObservationArchive(const std::string& name, int capacity);

    /// Бросает CapacityExceededException, если архив заполнен.
    void store(const WeatherSnapshot& snapshot);
    /// Средняя температура. Бросает ForecastNotAvailableException, если архив пуст.
    double averageTemperature() const;
    /// Сколько срезов теплее заданной температуры.
    int countWarmerThan(const Temperature& limit) const;

    const std::string& getName() const { return name_; }
    int getCount() const { return static_cast<int>(snapshots_.size()); }

private:
    std::string name_;
    std::vector<WeatherSnapshot> snapshots_;
    int capacity_;
};