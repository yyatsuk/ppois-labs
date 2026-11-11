#pragma once

#include <map>
#include <string>
#include <vector>

#include "people.h"

class Play;

/// Жанр пьесы с возрастным ограничением.
class Genre {
public:
    Genre(const std::string& name, const std::string& description, int minAge);

    /// Разрешён ли жанр зрителю данного возраста.
    bool isAllowedFor(int age) const;

    const std::string& getName() const;

private:
    std::string name_;
    std::string description_;
    int minAge_;
};

/// Драматург.
class Playwright {
public:
    Playwright(const std::string& name, const std::string& country);

    /// Добавляет пьесу в список автора. Бросает TheatreException при повторе.
    void addPlay(const Play& play);
    std::size_t getPlaysCount() const;

    const std::string& getName() const;

private:
    std::string name_;
    std::string country_;
    std::vector<std::string> plays_;
};

/// Пьеса.
class Play {
public:
    Play(const std::string& title, int durationMinutes, const Genre& genre,
         const Playwright& author);

    /// Подходит ли пьеса зрителю данного возраста (по жанру).
    bool isSuitableFor(int age) const;
    /// Длительность в часах.
    double getDurationHours() const;

    const std::string& getTitle() const;
    int getDurationMinutes() const;

private:
    std::string title_;
    int durationMinutes_;
    Genre genre_;
    Playwright author_;
};

/// Сценарий пьесы.
class Script {
public:
    Script(const Play& play, int version, int pageCount);

    /// Выпускает новую редакцию с новым числом страниц.
    void revise(int newPageCount);
    /// Примерное время чтения вслух в минутах.
    int estimateReadingTime() const;

    int getVersion() const;
    const Play& getPlay() const;

private:
    Play play_;
    int version_;
    int pageCount_;
};

/// Роль в пьесе с допустимым возрастным диапазоном исполнителя.
class Role {
public:
    Role(const std::string& name, int minAge, int maxAge);

    /// Подходит ли исполнитель данного возраста.
    bool isSuitableForAge(int age) const;
    /// Проверяет диапазон. Бросает TheatreException, если он некорректен.
    void validateAgeRange() const;

    const std::string& getName() const;

private:
    std::string name_;
    int minAge_;
    int maxAge_;
};

/// Состояние постановки.
enum class ProductionStatus { Draft, Approved, Cancelled };

/// Постановка пьесы режиссёром.
class Production {
public:
    Production(const Play& play, const Director& director, const Date& premiereDate);

    /// Утверждает постановку. Отменённую утвердить нельзя.
    void approve();
    void cancel();
    /// Состоялась ли премьера на дату today.
    bool isPremiered(const Date& today) const;

    const Play& getPlay() const;
    const Director& getDirector() const;
    const Date& getPremiereDate() const;
    ProductionStatus getStatus() const;

private:
    Play play_;
    Director director_;
    Date premiereDate_;
    ProductionStatus status_;
};

/// Актёрский состав постановки: какая роль кому отдана.
class Cast {
public:
    Cast(const Production& production, const std::vector<Role>& roles);

    /// Назначает актёра на роль. Бросает ActorNotAvailableException, если он не подходит.
    void assignActor(const Role& role, const Actor& actor, const Date& today);
    void removeActor(const Role& role);
    /// Все ли роли заняты.
    bool isComplete() const;
    /// Утверждает состав. Бросает InvalidCastException, если он неполный.
    void approve();

    bool isApproved() const;
    const Production& getProduction() const;

private:
    Production production_;
    std::map<std::string, std::string> assignments_;
    bool approved_;
};

/// Театральный сезон.
class Season {
public:
    Season(const std::string& name, const Date& startDate, const Date& endDate);

    /// Добавляет постановку, если её премьера попадает в сезон.
    void addProduction(const Production& production);
    bool isActive(const Date& today) const;

    std::size_t getProductionsCount() const;

private:
    std::string name_;
    Date startDate_;
    Date endDate_;
    std::vector<Production> productions_;
};

/// Костюм для роли.
class Costume {
public:
    Costume(const std::string& name, const std::string& size, int condition, const Role& role);

    /// Полностью восстанавливает состояние костюма.
    void repair();
    /// Пригоден ли костюм для выхода на сцену.
    bool isUsable() const;

    const std::string& getName() const;
    const Role& getRole() const;

private:
    std::string name_;
    std::string size_;
    int condition_;
    Role role_;
};

/// Декорация постановки.
class Decoration {
public:
    Decoration(const std::string& name, double weightKg, const Production& production);

    /// Описание сборки декорации с указанием времени.
    std::string assemble() const;
    /// Время сборки в минутах (зависит от веса).
    double calculateAssemblyTime() const;

    double getWeightKg() const;

private:
    std::string name_;
    double weightKg_;
    Production production_;
};