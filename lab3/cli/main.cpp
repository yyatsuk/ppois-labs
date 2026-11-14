#include <iostream>
#include <optional>
#include <string>
#include <vector>

#ifdef _WIN32
#include <windows.h>
#endif

#include "exceptions.h"
#include "service.h"

namespace {

const char* const kToday = "2026-11-15";
const char* const kNow = "2026-11-15 12:00";
const char* const kTomorrow = "2026-11-16";
const char* const kSeverityLabels[] = {"информация", "предупреждение", "опасность", "чрезвычайная"};
const int kSeverityCount = 4;
const double kRegionRadiusKm = 150.0;
const double kCloudBaseM = 1000.0;
const double kWarmLimitC = 20.0;
const int kMaxSubscriptions = 3;
const int kMaxFavorites = 5;
const int kArchiveCapacity = 50;
const int kStationCapacity = 5;
const int kReportDecimals = 1;

/// Признак закрытого ввода: меню завершается без сообщения об ошибке.
struct InputClosed {};

std::string readLine(const std::string& prompt) {
    std::cout << prompt << ": ";
    std::string line;
    if (!std::getline(std::cin, line)) {
        throw InputClosed();
    }
    return line;
}

double readDouble(const std::string& prompt) {
    std::string line = readLine(prompt);
    std::size_t parsed = 0;
    double value = 0.0;
    try {
        value = std::stod(line, &parsed);
    } catch (const std::logic_error&) {
        throw WeatherException("Нужно ввести число");
    }
    if (parsed != line.size()) {
        throw WeatherException("Нужно ввести число");
    }
    return value;
}

int readInt(const std::string& prompt) {
    return static_cast<int>(readDouble(prompt));
}

enum class MenuItem { Exit, Observe, Forecast, Alerts, Subscribe, AddFavorite, Archive };

/// Консольное меню сервиса прогноза погоды. С предметной областью работает только через её классы.
class WeatherMenu {
public:
    WeatherMenu();

    void run();

private:
    bool runOnce();
    void printMenu() const;
    void handle(MenuItem item);
    void observe();
    void showForecast() const;
    void showAlerts();
    void subscribe();
    void addFavorite();
    void showArchive() const;
    const WeatherSnapshot& requireSnapshot() const;

    Coordinates minskPoint_;
    Region region_;
    std::vector<City> cities_;
    ForecastService service_;
    User user_;
    SubscriptionManager subscriptions_;
    FavoriteLocations favorites_;
    ObservationArchive archive_;
    ReportGenerator report_;
    std::optional<WeatherSnapshot> lastSnapshot_;
};

WeatherStation makeStation(const Coordinates& location) {
    TemperatureSensor temperature("T-1", "DS18B20", "2025-01-01", TemperatureUnit::Celsius);
    PressureSensor pressure("P-1", "BMP280", "2025-01-01", location.getAltitude());
    HumiditySensor humidity("H-1", "DHT22", "2025-01-01", 1.0);
    WindSensor wind("W-1", "Davis", "2025-01-01", 10.0);
    for (Sensor* sensor : std::vector<Sensor*>{&temperature, &pressure, &humidity, &wind}) {
        sensor->calibrate("2026-01-01");
    }
    return WeatherStation("MINSK-1", "Минск, центр", location, temperature, pressure, humidity, wind);
}

ForecastService makeService(const Region& region, const Coordinates& location) {
    StationNetwork network("Минская сеть", region, kStationCapacity);
    network.addStation(makeStation(location));
    AlertCenter center("Центр предупреждений");
    center.addRule(AlertRule("Сильный ветер", 15.0, 10.0, Severity(3, kSeverityLabels[2])));
    center.addRule(AlertRule("Ураган", 25.0, 30.0, Severity(4, kSeverityLabels[3])));
    return ForecastService("Погода", network, ForecastModel("GFS", 25.0, 72, 0.5), center);
}

WeatherMenu::WeatherMenu()
    : minskPoint_(53.9, 27.5667, 220.0),
      region_("Минская область", "Беларусь", minskPoint_, kRegionRadiusKm),
      service_(makeService(region_, minskPoint_)),
      user_("u1", "Анна", "anna@mail.by", City("Минск", 2000000, minskPoint_, region_), kToday),
      subscriptions_(kMaxSubscriptions),
      favorites_(user_, kMaxFavorites),
      archive_("Наблюдения", kArchiveCapacity),
      report_("Прогноз", "Сервис погоды", kToday, kReportDecimals) {
    cities_.push_back(user_.getCity());
    cities_.push_back(City("Борисов", 140000, Coordinates(54.23, 28.5, 200.0), region_));
    cities_.push_back(City("Молодечно", 90000, Coordinates(54.31, 26.84, 190.0), region_));
}

void WeatherMenu::printMenu() const {
    std::cout << "\n=== Прогноз погоды: " << user_.getCity().getName() << " ===\n"
              << "1. Ввести показания станции\n"
              << "2. Прогноз на завтра\n"
              << "3. Проверить предупреждения\n"
              << "4. Подписаться на предупреждения\n"
              << "5. Добавить город в избранное\n"
              << "6. Архив наблюдений\n"
              << "0. Выход\n";
}

const WeatherSnapshot& WeatherMenu::requireSnapshot() const {
    if (!lastSnapshot_) {
        throw ForecastNotAvailableException("Сначала введите показания станции (пункт 1)");
    }
    return *lastSnapshot_;
}

void WeatherMenu::observe() {
    RawReadings raw{};
    raw.temperature = readDouble("Температура, °C");
    raw.pressure = readDouble("Давление, гПа");
    raw.humidity = readDouble("Влажность, %");
    raw.windSpeed = readDouble("Скорость ветра, м/с");
    raw.windDirection = readDouble("Направление ветра, градусы");
    raw.gust = readDouble("Порывы, м/с");
    double rain = readDouble("Дождь, мм за час");
    double cloud = readDouble("Облачность, %");
    PrecipitationType type = rain > 0 ? PrecipitationType::Rain : PrecipitationType::None;
    WeatherSnapshot snapshot = service_.observe(minskPoint_, raw, Precipitation(type, rain, 1.0, kNow),
                                                CloudCover(cloud, kCloudBaseM, kNow), kNow);
    archive_.store(snapshot);
    lastSnapshot_ = snapshot;
    std::cout << snapshot.summary() << "\nИндекс комфорта: " << snapshot.comfortIndex() << "\n";
}

void WeatherMenu::showForecast() const {
    DailyForecast day = service_.forecastDay(requireSnapshot(), kTomorrow);
    std::cout << report_.dailyReport(user_.getCity(), day) << "\n";
}

void WeatherMenu::showAlerts() {
    std::vector<AlertPtr> alerts = service_.checkAlerts(requireSnapshot(), region_, kNow);
    if (alerts.empty()) {
        std::cout << "Опасных явлений нет\n";
    }
    for (const AlertPtr& alert : alerts) {
        std::cout << alert->headline() << "\n";
        for (const Notification& note : subscriptions_.dispatch(*alert, kNow)) {
            std::cout << "  " << note.format() << "\n";
        }
    }
}

void WeatherMenu::subscribe() {
    int level = readInt("Минимальная степень опасности (1-4)");
    if (level < 1 || level > kSeverityCount) {
        throw InvalidAlertException("Степень опасности должна быть от 1 до 4");
    }
    Severity severity(level, kSeverityLabels[level - 1]);
    subscriptions_.subscribe(Subscription(user_, region_, severity));
    std::cout << "Подписка оформлена, подписок: " << subscriptions_.countFor(user_) << "\n";
}

void WeatherMenu::addFavorite() {
    for (const City& city : cities_) {
        std::cout << " - " << city.getName() << "\n";
    }
    std::string name = readLine("Название города");
    for (const City& city : cities_) {
        if (city.getName() == name) {
            favorites_.add(city);
            std::cout << "Добавлено, в избранном: " << favorites_.getCount() << "\n";
            return;
        }
    }
    throw InvalidUserDataException("Неизвестный город: " + name);
}

void WeatherMenu::showArchive() const {
    double average = archive_.averageTemperature();
    std::cout << "Наблюдений: " << archive_.getCount() << ", средняя температура: "
              << average << " °C, теплее "
              << kWarmLimitC << " °C: "
              << archive_.countWarmerThan(Temperature(kWarmLimitC, TemperatureUnit::Celsius, kNow)) << "\n";
}

void WeatherMenu::handle(MenuItem item) {
    switch (item) {
        case MenuItem::Observe: observe(); break;
        case MenuItem::Forecast: showForecast(); break;
        case MenuItem::Alerts: showAlerts(); break;
        case MenuItem::Subscribe: subscribe(); break;
        case MenuItem::AddFavorite: addFavorite(); break;
        case MenuItem::Archive: showArchive(); break;
        default: break;
    }
}

void WeatherMenu::run() {
    std::cout << user_.greeting() << "\n";
    try {
        while (true) {
            if (!runOnce()) {
                return;
            }
        }
    } catch (const InputClosed&) {
        std::cout << "\nВвод закончен\n";
    }
}

bool WeatherMenu::runOnce() {
    printMenu();
    try {
        int choice = readInt("Выберите пункт");
        if (choice == static_cast<int>(MenuItem::Exit)) {
            return false;
        }
        if (choice < 0 || choice > static_cast<int>(MenuItem::Archive)) {
            std::cout << "Нет такого пункта\n";
        } else {
            handle(static_cast<MenuItem>(choice));
        }
    } catch (const WeatherException& error) {
        std::cout << "Ошибка: " << error.what() << "\n";
    }
    return true;
}

}  // namespace

int main() {
#ifdef _WIN32
    SetConsoleOutputCP(CP_UTF8);
    SetConsoleCP(CP_UTF8);
#endif
    WeatherMenu menu;
    menu.run();
    return 0;
}