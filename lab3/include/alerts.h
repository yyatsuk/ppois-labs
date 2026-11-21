#pragma once

#include <memory>
#include <string>
#include <vector>

#include "stations.h"

/// Степень опасности.
class Severity {
public:
    /// Бросает InvalidAlertException, если уровень вне [1, 4].
    Severity(int level, const std::string& label);

    /// Опасный ли уровень (3 и выше).
    bool isDangerous() const;
    /// Через сколько минут нужно разослать уведомление: чем опаснее, тем быстрее.
    int notificationDelayMinutes() const;

    int getLevel() const { return level_; }
    const std::string& getLabel() const { return label_; }

private:
    int level_;
    std::string label_;
};

/// Правило, по которому погода признаётся опасной.
class AlertRule {
public:
    /// Бросает InvalidAlertException, если пороги не положительны.
    AlertRule(const std::string& name, double windThresholdMs, double rainThresholdMmH,
              const Severity& severity);

    /// Превышен ли хотя бы один порог.
    bool matches(const WeatherSnapshot& snapshot) const;
    /// Текст с фактическими значениями, на которых сработало правило.
    std::string describe(const WeatherSnapshot& snapshot) const;

    const std::string& getName() const { return name_; }
    const Severity& getSeverity() const { return severity_; }

private:
    std::string name_;
    double windThresholdMs_;
    double rainThresholdMmH_;
    Severity severity_;
};

/// Предупреждение об опасном явлении.
class Alert {
public:
    /// Бросает InvalidAlertException, если идентификатор или текст пусты или дата короче ГГГГ-ММ-ДД.
    Alert(const std::string& id, const std::string& message, const Severity& severity,
          const Region& region, const Date& validUntil);
    virtual ~Alert() = default;

    /// Действует ли предупреждение в момент now.
    bool isActive(const Date& now) const;
    /// Заголовок: степень опасности, текст и регион.
    virtual std::string headline() const;

    const std::string& getId() const { return id_; }
    const std::string& getMessage() const { return message_; }
    const Severity& getSeverity() const { return severity_; }
    const Region& getRegion() const { return region_; }
    const Date& getValidUntil() const { return validUntil_; }

private:
    std::string id_;
    std::string message_;
    Severity severity_;
    Region region_;
    Date validUntil_;
};

/// Штормовое предупреждение.
class StormWarning : public Alert {
public:
    StormWarning(const std::string& id, const Severity& severity, const Region& region,
                 const Date& validUntil, const City& center, double maxGustMs);

    std::string headline() const override;
    /// Находится ли город в зоне шторма (50 км от центра).
    bool threatens(const City& other) const;

private:
    City center_;
    double maxGustMs_;
};

/// Предупреждение о заморозках.
class FrostWarning : public Alert {
public:
    FrostWarning(const std::string& id, const Severity& severity, const Region& region,
                 const Date& validUntil, double thresholdC, int nights);

    std::string headline() const override;
    /// Подтверждает ли наблюдение заморозок.
    bool confirmedBy(const Temperature& observed) const;

private:
    double thresholdC_;
    int nights_;
};

/// Предупреждение о жаре.
class HeatWarning : public Alert {
public:
    HeatWarning(const std::string& id, const Severity& severity, const Region& region,
                const Date& validUntil, double thresholdC, int days);

    std::string headline() const override;
    /// Подтверждает ли наблюдение жару.
    bool confirmedBy(const Temperature& observed) const;

private:
    double thresholdC_;
    int days_;
};

/// Предупреждение о наводнении.
class FloodWarning : public Alert {
public:
    FloodWarning(const std::string& id, const Severity& severity, const Region& region,
                 const Date& validUntil, double rainfallLimitMm, double riverLevelCm);

    std::string headline() const override;
    /// Подтверждают ли осадки угрозу наводнения.
    bool confirmedBy(const Precipitation& observed) const;

private:
    double rainfallLimitMm_;
    double riverLevelCm_;
};

using AlertPtr = std::shared_ptr<Alert>;

/// Центр предупреждений: проверяет погоду по правилам и хранит выпущенные предупреждения.
class AlertCenter {
public:
    explicit AlertCenter(const std::string& name);

    void addRule(const AlertRule& rule);
    /// Выпускает предупреждение за каждое сработавшее правило. Действует до конца текущих суток.
    std::vector<AlertPtr> evaluate(const WeatherSnapshot& snapshot, const Region& region, const Date& now);
    /// Публикует готовое предупреждение. Бросает InvalidAlertException для пустого указателя.
    void publish(const AlertPtr& alert);
    std::vector<AlertPtr> activeAlerts(const Date& now) const;

    const std::string& getName() const { return name_; }
    int getRuleCount() const { return static_cast<int>(rules_.size()); }

private:
    std::string name_;
    std::vector<AlertRule> rules_;
    std::vector<AlertPtr> alerts_;
    int nextId_;
};

/// Свершившееся погодное явление.
class WeatherEvent {
public:
    /// Бросает InvalidAlertException, если ущерб отрицателен.
    WeatherEvent(const std::string& name, const Date& occurredAt, const Severity& severity,
                 double damage);

    /// Крупное ли явление: опасная степень или большой ущерб.
    bool isMajor() const;
    /// Масштаб ущерба словами.
    std::string classify() const;

    const std::string& getName() const { return name_; }
    const Date& getOccurredAt() const { return occurredAt_; }
    const Severity& getSeverity() const { return severity_; }
    double getDamage() const { return damage_; }

private:
    std::string name_;
    Date occurredAt_;
    Severity severity_;
    double damage_;
};

/// Архив погодных явлений региона.
class EventArchive {
public:
    /// Бросает InvalidAlertException, если ёмкость не положительна.
    EventArchive(const Region& region, int capacity);

    /// Бросает CapacityExceededException, если архив заполнен.
    void record(const WeatherEvent& event);
    std::vector<WeatherEvent> majorEvents() const;
    double totalDamage() const;

    const Region& getRegion() const { return region_; }
    int getEventCount() const { return static_cast<int>(events_.size()); }

private:
    Region region_;
    std::vector<WeatherEvent> events_;
    int capacity_;
};