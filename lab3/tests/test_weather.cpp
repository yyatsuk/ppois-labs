#include <catch2/catch_test_macros.hpp>
#include <catch2/catch_approx.hpp>
#include <cmath>
#include <string>
#include <vector>

#include "alerts.h"
#include "exceptions.h"
#include "forecasts.h"
#include "measurements.h"
#include "service.h"
#include "stations.h"

namespace {
const Date kNow = "2026-11-15 12:00";
const Date kToday = "2026-11-15";

Temperature celsius(double v) { return Temperature(v, TemperatureUnit::Celsius, kNow); }
Wind wind(double speed = 3.0, double gust = 5.0) { return Wind(speed, 90.0, gust, kNow); }
Pressure pressure(double hpa = 1010.0) { return Pressure(hpa, kNow, 0.0); }
Humidity humidity(double percent = 50.0) { return Humidity(percent, kNow); }
Precipitation rain(double mm = 0.0) {
    return Precipitation(mm > 0 ? PrecipitationType::Rain : PrecipitationType::None, mm, 1.0, kNow);
}
CloudCover clouds(double percent = 40.0) { return CloudCover(percent, 900.0, kNow); }
WeatherSnapshot snapshot(double t = 20.0, double windMs = 4.0, double rainMm = 0.0) {
    return WeatherSnapshot(celsius(t), pressure(), humidity(), wind(windMs, windMs + 1), rain(rainMm), clouds());
}

Coordinates minsk() { return Coordinates(53.9, 27.5667, 220.0); }
Coordinates moscow() { return Coordinates(55.7558, 37.6173, 150.0); }
Region region() { return Region("Минская область", "Беларусь", minsk(), 150.0); }
City city() { return City("Минск", 2000000, minsk(), region()); }
User user(int id = 1) { return User("u" + std::to_string(id), "Анна", "anna@mail.by", city(), kToday); }

WeatherStation station(const std::string& id, const Coordinates& at, bool calibrated = true) {
    TemperatureSensor t("T" + id, "m", "2025-01-01", TemperatureUnit::Celsius);
    PressureSensor p("P" + id, "m", "2025-01-01", 220.0);
    HumiditySensor h("H" + id, "m", "2025-01-01", 1.0);
    WindSensor w("W" + id, "m", "2025-01-01", 10.0);
    if (calibrated) {
        t.calibrate(kToday); p.calibrate(kToday); h.calibrate(kToday); w.calibrate(kToday);
    }
    return WeatherStation(id, "Станция " + id, at, t, p, h, w);
}

HourlyForecast hour(const std::string& stamp, double t, double windMs, double chance) {
    return HourlyForecast(stamp, Temperature(t, TemperatureUnit::Celsius, stamp), Wind(windMs, 90.0, windMs, stamp), chance);
}
ForecastModel model(double bias = 1.0, int horizon = 72) { return ForecastModel("GFS", 25.0, horizon, bias); }
Severity danger() { return Severity(3, "опасность"); }
Severity info() { return Severity(1, "информация"); }
Alert alert(const Severity& s = danger()) { return Alert("A1", "Сильный ветер", s, region(), "2026-11-15 23:59"); }
}

// ============ 1. Coordinates + Temperature ===================================
TEST_CASE("Coordinates и Temperature", "[measurements]") {
    REQUIRE(minsk().distanceTo(moscow()) == Catch::Approx(675.0).margin(15.0));
    REQUIRE(minsk().distanceTo(minsk()) == Catch::Approx(0.0).margin(1e-9));
    REQUIRE(minsk().isNorthern());
    REQUIRE_FALSE(Coordinates(-33.9, 151.2, 0.0).isNorthern());
    REQUIRE_THROWS_AS(Coordinates(91.0, 0.0, 0.0), InvalidCoordinatesException);
    REQUIRE_THROWS_AS(Coordinates(0.0, 181.0, 0.0), InvalidCoordinatesException);

    REQUIRE(celsius(25.0).toFahrenheit() == Catch::Approx(77.0));
    REQUIRE(Temperature(300.0, TemperatureUnit::Kelvin, kNow).toCelsius() == Catch::Approx(26.85));
    REQUIRE(Temperature(77.0, TemperatureUnit::Fahrenheit, kNow).toCelsius() == Catch::Approx(25.0));
    REQUIRE_THROWS_AS(Temperature(-1.0, TemperatureUnit::Kelvin, kNow), InvalidMeasurementException);
    REQUIRE(celsius(20.0).calculateFeelsLike(wind(5.0, 8.0), humidity(50.0)) == Catch::Approx(16.35).margin(0.1));
}

// ============ 2. Humidity + Wind + Pressure ==================================
TEST_CASE("Humidity, Wind, Pressure", "[measurements]") {
    REQUIRE(humidity(50.0).dewPoint(celsius(20.0)) == Catch::Approx(9.27).margin(0.05));
    REQUIRE(humidity(50.0).comfortLevel() == "комфортно");
    REQUIRE(humidity(10.0).comfortLevel() == "сухо");
    REQUIRE(humidity(80.0).comfortLevel() == "влажно");
    REQUIRE_THROWS_AS(humidity(0.0).dewPoint(celsius(20.0)), InvalidMeasurementException);
    REQUIRE_THROWS_AS(humidity(101.0), InvalidMeasurementException);

    REQUIRE(wind(10.0, 10.0).beaufortScale() == 5);
    REQUIRE(wind(0.0, 0.0).beaufortScale() == 0);
    REQUIRE(wind(40.0, 40.0).beaufortScale() == 12);
    REQUIRE(wind(15.0, 15.0).isStrong());
    REQUIRE_FALSE(wind(5.0, 5.0).isStrong());
    REQUIRE(wind().compassDirection() == "В");
    REQUIRE(Wind(1.0, 359.0, 1.0, kNow).compassDirection() == "С");
    REQUIRE(Wind(1.0, 225.0, 1.0, kNow).compassDirection() == "ЮЗ");
    REQUIRE_THROWS_AS(Wind(-1.0, 0.0, 0.0, kNow), InvalidMeasurementException);
    REQUIRE_THROWS_AS(Wind(5.0, 0.0, 3.0, kNow), InvalidMeasurementException);
    REQUIRE_THROWS_AS(Wind(5.0, 360.0, 6.0, kNow), InvalidMeasurementException);

    REQUIRE(pressure(1013.25).toMillimetersOfMercury() == Catch::Approx(760.0).margin(0.1));
    REQUIRE(pressure(1013.25).seaLevelEquivalent() == Catch::Approx(1013.25));
    REQUIRE(Pressure(950.0, kNow, 500.0).seaLevelEquivalent() == Catch::Approx(1008.0).margin(3.0));
    REQUIRE(pressure(990.0).isStormy());
    REQUIRE_FALSE(pressure(1013.0).isStormy());
    REQUIRE_THROWS_AS(pressure(100.0), InvalidMeasurementException);
}

// ============ 3. Precipitation + CloudCover + Visibility + UvIndex ===========
TEST_CASE("Precipitation, CloudCover, Visibility, UvIndex", "[measurements]") {
    Precipitation heavy(PrecipitationType::Rain, 12.0, 2.0, kNow);
    REQUIRE(heavy.intensity() == Catch::Approx(6.0));
    REQUIRE(heavy.isHeavy());
    REQUIRE_FALSE(rain(1.0).isHeavy());
    REQUIRE_FALSE(rain(0.0).isHeavy());
    REQUIRE_THROWS_AS(Precipitation(PrecipitationType::Rain, -1.0, 1.0, kNow), InvalidMeasurementException);
    REQUIRE_THROWS_AS(Precipitation(PrecipitationType::Rain, 1.0, 0.0, kNow), InvalidMeasurementException);

    REQUIRE(clouds(5.0).describe() == "ясно");
    REQUIRE(clouds(30.0).describe() == "малооблачно");
    REQUIRE(clouds(70.0).describe() == "облачно");
    REQUIRE(clouds(95.0).describe() == "пасмурно");
    REQUIRE(clouds(95.0).isOvercast());
    REQUIRE_FALSE(clouds(30.0).isOvercast());
    REQUIRE_THROWS_AS(clouds(120.0), InvalidMeasurementException);
    REQUIRE_THROWS_AS(CloudCover(50.0, -1.0, kNow), InvalidMeasurementException);

    Visibility fog(400.0, "туман", kNow);
    REQUIRE(fog.isFoggy());
    REQUIRE(fog.toKilometers() == Catch::Approx(0.4));
    REQUIRE_FALSE(Visibility(5000.0, "дымка", kNow).isFoggy());
    REQUIRE_THROWS_AS(Visibility(-1.0, "x", kNow), InvalidMeasurementException);

    REQUIRE(UvIndex(1.0, kNow).riskLevel() == "низкий");
    REQUIRE(UvIndex(5.0, kNow).riskLevel() == "умеренный");
    REQUIRE(UvIndex(7.0, kNow).riskLevel() == "высокий");
    REQUIRE(UvIndex(9.0, kNow).riskLevel() == "очень высокий");
    REQUIRE(UvIndex(12.0, kNow).riskLevel() == "экстремальный");
    REQUIRE_FALSE(UvIndex(12.0, kNow).protectionAdvice().empty());
    REQUIRE_THROWS_AS(UvIndex(17.0, kNow), InvalidMeasurementException);
}

// ============ 4. WeatherSnapshot =============================================
TEST_CASE("WeatherSnapshot", "[measurements]") {
    REQUIRE(snapshot(20.0, 4.0, 5.0).isRainy());
    REQUIRE_FALSE(snapshot().isRainy());
    REQUIRE(snapshot().comfortIndex() == Catch::Approx(100.0 - 3.0 * (21.0 - snapshot().getTemperature().calculateFeelsLike(
                                                                              snapshot().getWind(), snapshot().getHumidity()))));
    REQUIRE(snapshot(-30.0, 20.0).comfortIndex() == Catch::Approx(0.0).margin(1e-9));
    REQUIRE(snapshot().summary().find("°C") != std::string::npos);
    REQUIRE(snapshot().getCloudCover().getPercent() == Catch::Approx(40.0));
}

// ============ 5. Sensor + четыре датчика =====================================
TEST_CASE("Sensor и датчики", "[stations][sensors]") {
    Sensor s("S1", "X", "2025-01-01");
    REQUIRE_FALSE(s.isReady());
    REQUIRE_THROWS_AS(s.ensureReady(), SensorNotCalibratedException);
    s.calibrate("2026-02-01");
    REQUIRE(s.isReady());
    s.ensureReady();
    s.markFaulty();
    REQUIRE_FALSE(s.isReady());
    REQUIRE_THROWS_AS(s.ensureReady(), SensorMalfunctionException);

    TemperatureSensor t("T1", "m", "2025-01-01", TemperatureUnit::Celsius);
    REQUIRE_THROWS_AS(t.read(20.0, kNow), SensorNotCalibratedException);
    t.adjustOffset(20.0, 19.0, kToday);
    REQUIRE(t.read(19.0, kNow).toCelsius() == Catch::Approx(20.0));

    PressureSensor p("P1", "m", "2025-01-01", 200.0);
    p.adjustOffset(1013.0, 1010.0, kToday);
    REQUIRE(p.read(1010.0, kNow).getHectopascals() == Catch::Approx(1013.0));

    HumiditySensor h("H1", "m", "2025-01-01", 1.1);
    h.adjustOffset(55.0, 50.0, kToday);
    REQUIRE(h.read(50.0, kNow).getPercent() == Catch::Approx(55.0));
    REQUIRE(h.read(99.0, kNow).getPercent() == Catch::Approx(100.0));
    REQUIRE_THROWS_AS(HumiditySensor("H", "m", "d", 0.0), InvalidMeasurementException);

    WindSensor w("W1", "m", "2025-01-01", 20.0);
    w.adjustOffset(10.0, 10.0, kToday);
    REQUIRE(w.read(10.0, 90.0, 12.0, kNow).getSpeedMs() == Catch::Approx(10.0 * std::pow(0.5, 0.143)));
    REQUIRE(w.read(10.0, 90.0, 5.0, kNow).getGustMs() >= w.read(10.0, 90.0, 5.0, kNow).getSpeedMs());
    REQUIRE_THROWS_AS(WindSensor("W", "m", "d", -5.0), InvalidMeasurementException);
}

// ============ 6. Calibration + Region + City =================================
TEST_CASE("Calibration, Region, City", "[stations]") {
    Calibration c("T1", "2026-03-10", 0.5, "Иванов");
    REQUIRE(c.apply(10.0) == Catch::Approx(10.5));
    REQUIRE_FALSE(c.isExpired("2027-03-09"));
    REQUIRE(c.isExpired("2027-03-10"));
    REQUIRE(c.isExpired("2028-01-01"));
    REQUIRE_THROWS_AS(Calibration("T", "2026", 0.0, "x"), InvalidMeasurementException);

    REQUIRE(region().contains(Coordinates(54.0, 28.0, 0.0)));
    REQUIRE_FALSE(region().contains(moscow()));
    REQUIRE(region().areaKm2() == Catch::Approx(3.14159265358979 * 150.0 * 150.0));
    REQUIRE_THROWS_AS(Region("r", "c", minsk(), 0.0), InvalidCoordinatesException);

    REQUIRE(city().isInRegion());
    REQUIRE(city().isMegacity());
    REQUIRE_FALSE(City("Борисов", 140000, Coordinates(54.2, 28.5, 0.0), region()).isMegacity());
    REQUIRE_FALSE(City("Москва", 13000000, moscow(), region()).isInRegion());
}

// ============ 7. WeatherStation + StationNetwork =============================
TEST_CASE("WeatherStation и StationNetwork", "[stations]") {
    WeatherStation a = station("A", minsk());
    REQUIRE(a.isOperational());
    RawReadings raw{20.0, 1010.0, 60.0, 4.0, 180.0, 7.0};
    WeatherSnapshot s = a.measure(raw, rain(), clouds(), kNow);
    REQUIRE(s.getTemperature().toCelsius() == Catch::Approx(20.0));
    REQUIRE(s.getHumidity().getPercent() == Catch::Approx(60.0));
    REQUIRE_FALSE(station("B", minsk(), false).isOperational());
    REQUIRE_THROWS_AS(station("B", minsk(), false).measure(raw, rain(), clouds(), kNow), SensorNotCalibratedException);
    a.deactivate();
    REQUIRE_FALSE(a.isOperational());
    REQUIRE_THROWS_AS(a.measure(raw, rain(), clouds(), kNow), SensorMalfunctionException);

    StationNetwork net("Сеть", region(), 2);
    REQUIRE_THROWS_AS(net.nearestStation(minsk()), StationNotFoundException);
    net.addStation(station("A", minsk()));
    net.addStation(station("B", Coordinates(54.2, 28.5, 0.0)));
    REQUIRE(net.getStationCount() == 2);
    REQUIRE_THROWS_AS(net.addStation(station("C", minsk())), CapacityExceededException);
    REQUIRE(net.findStation("B").getName() == "Станция B");
    REQUIRE_THROWS_AS(net.findStation("Z"), StationNotFoundException);
    REQUIRE(net.nearestStation(Coordinates(54.3, 28.6, 0.0)).getId() == "B");
    REQUIRE_THROWS_AS(StationNetwork("M", region(), 5).addStation(station("M", moscow())), InvalidCoordinatesException);
    REQUIRE_THROWS_AS(StationNetwork("x", region(), 0), InvalidMeasurementException);
}

// ============ 8. Confidence + ForecastModel + HourlyForecast + ModelRun ======
TEST_CASE("Confidence, ForecastModel, HourlyForecast, ModelRun", "[forecast]") {
    REQUIRE(Confidence(80.0, 24, "M").level() == "высокая");
    REQUIRE(Confidence(80.0, 24, "M").isReliable());
    REQUIRE(Confidence(50.0, 24, "M").level() == "средняя");
    REQUIRE(Confidence(10.0, 24, "M").level() == "низкая");
    REQUIRE_FALSE(Confidence(10.0, 24, "M").isReliable());
    REQUIRE_THROWS_AS(Confidence(101.0, 24, "M"), InvalidMeasurementException);
    REQUIRE_THROWS_AS(Confidence(50.0, 0, "M"), UnsupportedHorizonException);

    REQUIRE(model().correct(celsius(21.0)).toCelsius() == Catch::Approx(20.0));
    REQUIRE(model().supportsHorizon(72));
    REQUIRE_FALSE(model().supportsHorizon(73));
    REQUIRE_FALSE(model().supportsHorizon(0));
    REQUIRE_THROWS_AS(ForecastModel("x", 0.0, 10, 0.0), InvalidMeasurementException);

    REQUIRE(hour("2026-11-15 12:00", 5.0, 3.0, 10.0).isDry());
    REQUIRE_FALSE(hour("2026-11-15 12:00", 5.0, 3.0, 10.0).isStormy());
    REQUIRE(hour("2026-11-15 13:00", 5.0, 20.0, 10.0).isStormy());
    REQUIRE(hour("2026-11-15 14:00", 5.0, 3.0, 90.0).isStormy());
    REQUIRE_FALSE(hour("2026-11-15 14:00", 5.0, 3.0, 50.0).isDry());
    REQUIRE_THROWS_AS(hour("d", 5.0, 3.0, 120.0), InvalidMeasurementException);

    ModelRun run(model(), snapshot(), "2026-11-15 00:00");
    REQUIRE_THROWS_AS(run.forecastAt(1), ForecastNotAvailableException);
    run.addStep(hour("2026-11-15 12:00", 5.0, 3.0, 10.0));
    REQUIRE(run.forecastAt(1).getHour() == "2026-11-15 12:00");
    REQUIRE(run.getStepCount() == 1);
    REQUIRE_THROWS_AS(run.forecastAt(100), UnsupportedHorizonException);
}

// ============ 9. DailyForecast + WeeklyForecast ==============================
TEST_CASE("DailyForecast и WeeklyForecast", "[forecast]") {
    DailyForecast day("2026-11-15", model(), "2026-11-15 06:00");
    REQUIRE_THROWS_AS(day.maxTemperature(), ForecastNotAvailableException);
    REQUIRE_THROWS_AS(day.peakRainChance(), ForecastNotAvailableException);
    day.addHour(hour("2026-11-15 06:00", 2.0, 3.0, 20.0));
    day.addHour(hour("2026-11-15 15:00", 8.0, 3.0, 65.0));
    REQUIRE_THROWS_AS(day.addHour(hour("2026-11-16 06:00", 2.0, 3.0, 20.0)), ForecastNotAvailableException);
    REQUIRE(day.maxTemperature() == Catch::Approx(7.0));
    REQUIRE(day.minTemperature() == Catch::Approx(1.0));
    REQUIRE(day.peakRainChance() == Catch::Approx(65.0));

    WeeklyForecast week(city(), Confidence(80.0, 168, "GFS"), "2026-11-15");
    REQUIRE_THROWS_AS(week.warmestDay(), ForecastNotAvailableException);
    REQUIRE_THROWS_AS(week.averageMaxTemperature(), ForecastNotAvailableException);
    week.addDay(day);
    DailyForecast warm("2026-11-16", model(), "2026-11-15 06:00");
    warm.addHour(hour("2026-11-16 12:00", 14.0, 2.0, 5.0));
    week.addDay(warm);
    REQUIRE(week.warmestDay().getDate() == "2026-11-16");
    REQUIRE(week.averageMaxTemperature() == Catch::Approx(10.0));
    for (int i = 2; i < 7; ++i) {
        week.addDay(day);
    }
    REQUIRE_THROWS_AS(week.addDay(day), CapacityExceededException);
}

// ============ 10. Accuracy + Bulletin + Climatology + TrendAnalyzer ==========
TEST_CASE("ForecastAccuracy, ForecastBulletin, Climatology, TrendAnalyzer", "[forecast]") {
    ForecastAccuracy acc("GFS");
    REQUIRE_THROWS_AS(acc.meanError(), ForecastNotAvailableException);
    acc.record(celsius(10.0), celsius(12.0));
    acc.record(celsius(10.0), celsius(9.0));
    REQUIRE(acc.meanError() == Catch::Approx(1.5));
    REQUIRE(acc.getMaxError() == Catch::Approx(2.0));

    DailyForecast day("2026-11-15", model(), "2026-11-15 06:00");
    day.addHour(hour("2026-11-15 06:00", 2.0, 3.0, 20.0));
    ForecastBulletin bulletin(city(), day, Confidence(80.0, 24, "GFS"), "2026-11-15 06:00");
    REQUIRE(bulletin.render().find("Минск") != std::string::npos);
    REQUIRE(bulletin.isActual("2026-11-15 23:00"));
    REQUIRE_FALSE(bulletin.isActual("2026-11-16 01:00"));

    Climatology climate(city(), {-6, -5, -1, 6, 13, 16, 18, 17, 12, 6, 1, -3}, 30);
    REQUIRE(climate.normalFor(7) == Catch::Approx(18.0));
    REQUIRE(climate.anomaly(celsius(20.0), 7) == Catch::Approx(2.0));
    REQUIRE_THROWS_AS(climate.normalFor(13), InvalidMeasurementException);
    REQUIRE_THROWS_AS(Climatology(city(), {1.0, 2.0}, 5), InvalidMeasurementException);

    TrendAnalyzer trend(3);
    REQUIRE_THROWS_AS(TrendAnalyzer(1), InvalidMeasurementException);
    REQUIRE_THROWS_AS(trend.expectedWeather(), ForecastNotAvailableException);
    trend.addObservation(pressure(1015.0), celsius(10.0));
    trend.addObservation(pressure(1010.0), celsius(8.0));
    REQUIRE(trend.expectedWeather() == "ухудшение с похолоданием");
    trend.addObservation(pressure(1005.0), celsius(12.0));
    REQUIRE(trend.expectedWeather() == "ухудшение, возможны осадки");
    trend.addObservation(pressure(1012.0), celsius(12.0));
    trend.addObservation(pressure(1020.0), celsius(12.0));
    REQUIRE(trend.expectedWeather() == "улучшение погоды");
    trend.addObservation(pressure(1020.0), celsius(12.0));
    trend.addObservation(pressure(1020.0), celsius(12.0));
    REQUIRE(trend.expectedWeather() == "без перемен");
}

// ============ 11. Severity + AlertRule + Alert + предупреждения ==============
TEST_CASE("Severity, AlertRule, Alert и виды предупреждений", "[alerts]") {
    REQUIRE_FALSE(info().isDangerous());
    REQUIRE(danger().isDangerous());
    REQUIRE(info().notificationDelayMinutes() == 60);
    REQUIRE(Severity(4, "x").notificationDelayMinutes() == 0);
    REQUIRE_THROWS_AS(Severity(0, "x"), InvalidAlertException);
    REQUIRE_THROWS_AS(Severity(5, "x"), InvalidAlertException);

    AlertRule rule("Шторм", 20.0, 10.0, danger());
    REQUIRE(rule.matches(snapshot(20.0, 25.0)));
    REQUIRE(rule.matches(snapshot(20.0, 5.0, 12.0)));
    REQUIRE_FALSE(rule.matches(snapshot(20.0, 5.0, 2.0)));
    REQUIRE(rule.describe(snapshot(20.0, 25.0)).find("25.0") != std::string::npos);
    REQUIRE_THROWS_AS(AlertRule("x", 0.0, 1.0, danger()), InvalidAlertException);
    REQUIRE_THROWS_AS(AlertRule("x", 1.0, -1.0, danger()), InvalidAlertException);

    REQUIRE(alert().isActive("2026-11-15 12:00"));
    REQUIRE_FALSE(alert().isActive("2026-11-16 00:00"));
    REQUIRE(alert().headline() == "[опасность] Сильный ветер — Минская область");
    REQUIRE_THROWS_AS(Alert("", "m", danger(), region(), "2026-11-15"), InvalidAlertException);
    REQUIRE_THROWS_AS(Alert("id", "", danger(), region(), "2026-11-15"), InvalidAlertException);
    REQUIRE_THROWS_AS(Alert("id", "m", danger(), region(), "2026"), InvalidAlertException);

    StormWarning storm("S1", danger(), region(), "2026-11-15 23:59", city(), 30.0);
    REQUIRE(storm.threatens(City("Заславль", 10000, Coordinates(54.0, 27.3, 0.0), region())));
    REQUIRE_FALSE(storm.threatens(City("Москва", 13000000, moscow(), region())));
    REQUIRE(storm.headline().find("30.0") != std::string::npos);
    FrostWarning frost("F1", info(), region(), "2026-11-15 23:59", -5.0, 2);
    REQUIRE(frost.confirmedBy(celsius(-7.0)));
    REQUIRE_FALSE(frost.confirmedBy(celsius(0.0)));
    REQUIRE(frost.headline().find("ночей: 2") != std::string::npos);
    HeatWarning heat("H1", danger(), region(), "2026-11-15 23:59", 30.0, 3);
    REQUIRE(heat.confirmedBy(celsius(32.0)));
    REQUIRE_FALSE(heat.confirmedBy(celsius(20.0)));
    REQUIRE(heat.headline().find("дней: 3") != std::string::npos);
    FloodWarning flood("W1", danger(), region(), "2026-11-15 23:59", 50.0, 320.0);
    REQUIRE(flood.confirmedBy(Precipitation(PrecipitationType::Rain, 60.0, 6.0, kNow)));
    REQUIRE_FALSE(flood.confirmedBy(Precipitation(PrecipitationType::Rain, 10.0, 6.0, kNow)));
    const Alert& base = flood;
    REQUIRE(base.headline().find("320.0") != std::string::npos);
}

// ============ 12. AlertCenter + WeatherEvent + EventArchive ==================
TEST_CASE("AlertCenter, WeatherEvent, EventArchive", "[alerts]") {
    AlertCenter center("Центр");
    REQUIRE(center.evaluate(snapshot(20.0, 25.0), region(), kNow).empty());
    center.addRule(AlertRule("Ветер", 20.0, 10.0, danger()));
    center.addRule(AlertRule("Ливень", 40.0, 8.0, Severity(4, "чрезвычайная")));
    REQUIRE(center.getRuleCount() == 2);
    std::vector<AlertPtr> issued = center.evaluate(snapshot(20.0, 25.0, 12.0), region(), kNow);
    REQUIRE(issued.size() == 2);
    REQUIRE(issued[1]->getId() == "A2");
    REQUIRE(issued[0]->getValidUntil() == "2026-11-15 23:59");
    center.publish(std::make_shared<StormWarning>("S1", danger(), region(), "2026-11-15 23:59", city(), 30.0));
    REQUIRE_THROWS_AS(center.publish(nullptr), InvalidAlertException);
    REQUIRE(center.activeAlerts(kNow).size() == 3);
    REQUIRE(center.activeAlerts("2026-11-16 11:00").empty());

    WeatherEvent flood("Наводнение", "2026-05-01", danger(), 2500000.0);
    REQUIRE(flood.isMajor());
    REQUIRE(flood.classify() == "катастрофическое");
    WeatherEvent gale("Порывы", "2026-06-01", info(), 150000.0);
    REQUIRE_FALSE(gale.isMajor());
    REQUIRE(gale.classify() == "значительное");
    WeatherEvent drizzle("Дождь", "2026-07-01", info(), 10.0);
    REQUIRE(drizzle.classify() == "незначительное");
    REQUIRE_THROWS_AS(WeatherEvent("x", "d", info(), -1.0), InvalidAlertException);

    EventArchive archive(region(), 3);
    archive.record(flood); archive.record(gale); archive.record(drizzle);
    REQUIRE_THROWS_AS(archive.record(gale), CapacityExceededException);
    REQUIRE(archive.getEventCount() == 3);
    REQUIRE(archive.majorEvents().size() == 1);
    REQUIRE(archive.totalDamage() == Catch::Approx(2650010.0));
    REQUIRE_THROWS_AS(EventArchive(region(), 0), InvalidAlertException);
}

// ============ 13. User + Subscription + Notification + источники =============
TEST_CASE("User, Subscription, Notification, DataSource, DataProvider", "[service]") {
    REQUIRE(user().greeting() == "Здравствуйте, Анна! Ваш город: Минск");
    REQUIRE(user().emailDomain() == "mail.by");
    REQUIRE_THROWS_AS(User("", "Анна", "a@b", city(), kToday), InvalidUserDataException);
    REQUIRE_THROWS_AS(User("u", "", "a@b", city(), kToday), InvalidUserDataException);
    REQUIRE_THROWS_AS(User("u", "Анна", "no-at", city(), kToday), InvalidUserDataException);

    Subscription sub(user(), region(), danger());
    REQUIRE(sub.wants(alert()));
    REQUIRE_FALSE(Subscription(user(), region(), Severity(4, "x")).wants(alert()));
    Region brest("Брестская область", "Беларусь", Coordinates(52.1, 23.7, 0.0), 100.0);
    REQUIRE_FALSE(Subscription(user(), brest, info()).wants(alert()));
    sub.pause();
    REQUIRE_FALSE(sub.wants(alert()));
    sub.resume();
    REQUIRE(sub.wants(alert()));

    Notification note(user(), alert(), kNow);
    REQUIRE(note.format().find("Сильный ветер") != std::string::npos);
    note.deliver();
    REQUIRE_THROWS_AS(note.deliver(), NotificationFailedException);

    DataSource source("Метео-1", region(), true);
    REQUIRE(source.covers(minsk()));
    REQUIRE_FALSE(source.covers(moscow()));
    source.setOnline(false);
    REQUIRE_THROWS_AS(source.ensureOnline(), DataSourceUnavailableException);
    source.setOnline(true);
    source.ensureOnline();

    DataProvider provider("Провайдер");
    REQUIRE_THROWS_AS(provider.sourceFor(minsk()), DataSourceUnavailableException);
    provider.addSource(source);
    REQUIRE(provider.sourceFor(minsk()).getName() == "Метео-1");
    REQUIRE(provider.getRequestsServed() == 1);
    provider.addSource(DataSource("Оффлайн", brest, false));
    REQUIRE_THROWS_AS(provider.sourceFor(Coordinates(52.1, 23.7, 0.0)), DataSourceUnavailableException);
}

// ============ 14. Сервис, подписки, отчёты, избранное, архив =================
TEST_CASE("ForecastService, SubscriptionManager, ReportGenerator, FavoriteLocations, ObservationArchive", "[service][integration]") {
    StationNetwork net("Сеть", region(), 3);
    net.addStation(station("A", minsk()));
    AlertCenter center("Центр");
    center.addRule(AlertRule("Ветер", 3.0, 10.0, danger()));
    ForecastService service("Сервис", net, model(0.0), center);
    RawReadings raw{20.0, 1010.0, 60.0, 4.0, 180.0, 7.0};
    WeatherSnapshot now = service.observe(minsk(), raw, rain(), clouds(50.0), kNow);
    REQUIRE(now.getTemperature().toCelsius() == Catch::Approx(20.0));
    DailyForecast day = service.forecastDay(now, "2026-11-16");
    REQUIRE(day.getHourCount() == 24);
    REQUIRE(day.maxTemperature() == Catch::Approx(25.0));
    REQUIRE(day.minTemperature() == Catch::Approx(15.0));
    REQUIRE(day.peakRainChance() == Catch::Approx(30.0));
    REQUIRE_THROWS_AS(ForecastService("Короткий", net, model(0.0, 12), center).forecastDay(now, "2026-11-16"),
                      UnsupportedHorizonException);
    REQUIRE(service.checkAlerts(now, region(), kNow).size() == 1);

    SubscriptionManager manager(2);
    REQUIRE_THROWS_AS(SubscriptionManager(0), InvalidUserDataException);
    manager.subscribe(Subscription(user(), region(), danger()));
    manager.subscribe(Subscription(user(), region(), info()));
    REQUIRE_THROWS_AS(manager.subscribe(Subscription(user(), region(), info())), CapacityExceededException);
    REQUIRE(manager.countFor(user()) == 2);
    manager.subscribe(Subscription(user(2), region(), Severity(4, "x")));
    std::vector<Notification> sent = manager.dispatch(alert(), kNow);
    REQUIRE(sent.size() == 2);
    REQUIRE(manager.getNotificationCount() == 2);

    ReportGenerator report("Сводка", "Сервис", kToday, 1);
    REQUIRE(report.dailyReport(city(), day).find("25.0") != std::string::npos);
    ForecastAccuracy accuracy("GFS");
    REQUIRE_THROWS_AS(report.accuracyReport(accuracy), ForecastNotAvailableException);
    accuracy.record(celsius(10.0), celsius(12.0));
    REQUIRE(report.accuracyReport(accuracy).find("2.0") != std::string::npos);
    REQUIRE_THROWS_AS(ReportGenerator("t", "a", kToday, -1), InvalidUserDataException);

    FavoriteLocations favorites(user(), 2);
    favorites.add(city());
    REQUIRE(favorites.contains("Минск"));
    REQUIRE_FALSE(favorites.contains("Брест"));
    REQUIRE_THROWS_AS(favorites.add(city()), InvalidUserDataException);
    favorites.add(City("Брест", 340000, Coordinates(52.1, 23.7, 0.0), region()));
    REQUIRE_THROWS_AS(favorites.add(City("Гомель", 500000, Coordinates(52.4, 31.0, 0.0), region())), CapacityExceededException);
    favorites.remove("Брест");
    REQUIRE_FALSE(favorites.contains("Брест"));
    REQUIRE_THROWS_AS(favorites.remove("Брест"), InvalidUserDataException);
    REQUIRE_THROWS_AS(FavoriteLocations(user(), 0), InvalidUserDataException);

    ObservationArchive archive("Архив", 3);
    REQUIRE_THROWS_AS(archive.averageTemperature(), ForecastNotAvailableException);
    archive.store(snapshot(10.0)); archive.store(snapshot(20.0)); archive.store(snapshot(30.0));
    REQUIRE_THROWS_AS(archive.store(snapshot(5.0)), CapacityExceededException);
    REQUIRE(archive.averageTemperature() == Catch::Approx(20.0));
    REQUIRE(archive.countWarmerThan(celsius(15.0)) == 2);
    REQUIRE_THROWS_AS(ObservationArchive("x", 0), InvalidMeasurementException);
}