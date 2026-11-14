#pragma once
#include <stdexcept>
#include <string>

/// Базовое исключение предметной области прогноза погоды.
class WeatherException : public std::runtime_error {
public:
    explicit WeatherException(const std::string& message)
        : std::runtime_error(message) {}
};

/// Значение измерения вне физически возможного диапазона.
class InvalidMeasurementException : public WeatherException {
public:
    explicit InvalidMeasurementException(const std::string& message)
        : WeatherException(message) {}
};

/// Широта или долгота вне допустимых пределов.
class InvalidCoordinatesException : public WeatherException {
public:
    explicit InvalidCoordinatesException(const std::string& message)
        : WeatherException(message) {}
};

/// Показание датчика вне его рабочего диапазона или датчик отключён.
class SensorMalfunctionException : public WeatherException {
public:
    explicit SensorMalfunctionException(const std::string& message)
        : WeatherException(message) {}
};

/// Калибровка датчика просрочена.
class SensorNotCalibratedException : public WeatherException {
public:
    explicit SensorNotCalibratedException(const std::string& message)
        : WeatherException(message) {}
};

/// Метеостанция не найдена.
class StationNotFoundException : public WeatherException {
public:
    explicit StationNotFoundException(const std::string& message)
        : WeatherException(message) {}
};

/// Прогноз на запрошенную дату недоступен.
class ForecastNotAvailableException : public WeatherException {
public:
    explicit ForecastNotAvailableException(const std::string& message)
        : WeatherException(message) {}
};

/// Запрошенный горизонт прогноза не поддерживается моделью.
class UnsupportedHorizonException : public WeatherException {
public:
    explicit UnsupportedHorizonException(const std::string& message)
        : WeatherException(message) {}
};

/// Предупреждение задано неверно: некорректные даты или срок.
class InvalidAlertException : public WeatherException {
public:
    explicit InvalidAlertException(const std::string& message)
        : WeatherException(message) {}
};

/// Уведомление не удалось отправить.
class NotificationFailedException : public WeatherException {
public:
    explicit NotificationFailedException(const std::string& message)
        : WeatherException(message) {}
};

/// Данные пользователя некорректны (например, неверный адрес почты).
class InvalidUserDataException : public WeatherException {
public:
    explicit InvalidUserDataException(const std::string& message)
        : WeatherException(message) {}
};

/// Превышен допустимый размер: лимит подписок, избранного или вместимость архива.
class CapacityExceededException : public WeatherException {
public:
    explicit CapacityExceededException(const std::string& message)
        : WeatherException(message) {}
};

/// Подходящий источник данных недоступен.
class DataSourceUnavailableException : public WeatherException {
public:
    explicit DataSourceUnavailableException(const std::string& message)
        : WeatherException(message) {}
};