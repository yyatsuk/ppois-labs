# Лабораторная работа №3. Крупный ООП-проект: «Прогноз погоды»

Объектная модель сервиса прогноза погоды: измерения и метеовеличины, метеостанции и датчики,
построение прогнозов, предупреждения об опасных явлениях, пользователи и подписки.

Формат строки: `Класс | число полей | число методов | связанные классы`.

Наследование (в ассоциации не входит): TemperatureSensor, PressureSensor, HumiditySensor и WindSensor
наследуются от Sensor; StormWarning, FrostWarning, HeatWarning и FloodWarning наследуются от Alert.

| Класс | Поля | Методы | Ассоциации (связанные классы) |
|---|---|---|---|
| Alert | 5 | 2 | Region, Severity |
| AlertCenter | 4 | 3 | Alert, AlertRule, Region, WeatherSnapshot |
| AlertRule | 4 | 2 | Severity, WeatherSnapshot |
| Calibration | 4 | 2 | — |
| City | 4 | 2 | Coordinates, Region |
| Climatology | 3 | 2 | City, Temperature |
| CloudCover | 3 | 2 | — |
| Confidence | 3 | 2 | — |
| Coordinates | 3 | 2 | — |
| DailyForecast | 4 | 4 | ForecastModel, HourlyForecast |
| DataProvider | 3 | 2 | Coordinates, DataSource |
| DataSource | 3 | 2 | Coordinates, Region |
| EventArchive | 3 | 3 | Region, WeatherEvent |
| FavoriteLocations | 3 | 3 | City, User |
| FloodWarning | 2 | 2 | Precipitation |
| ForecastAccuracy | 4 | 2 | Temperature |
| ForecastBulletin | 4 | 2 | City, Confidence, DailyForecast |
| ForecastModel | 4 | 2 | Temperature |
| ForecastService | 4 | 3 | Alert, AlertCenter, Coordinates, DailyForecast, ForecastModel, StationNetwork, WeatherSnapshot, WeatherStation |
| FrostWarning | 2 | 2 | Temperature |
| HeatWarning | 2 | 2 | Temperature |
| HourlyForecast | 4 | 2 | Temperature, Wind |
| Humidity | 2 | 2 | Temperature |
| HumiditySensor | 2 | 2 | Humidity |
| ModelRun | 4 | 2 | ForecastModel, HourlyForecast, WeatherSnapshot |
| Notification | 4 | 2 | Alert, User |
| ObservationArchive | 3 | 3 | Temperature, WeatherSnapshot |
| Precipitation | 4 | 2 | — |
| Pressure | 3 | 3 | — |
| PressureSensor | 2 | 2 | Pressure |
| Region | 4 | 2 | Coordinates |
| ReportGenerator | 4 | 2 | City, DailyForecast, ForecastAccuracy |
| Sensor | 5 | 3 | — |
| Severity | 2 | 2 | — |
| StationNetwork | 4 | 3 | Coordinates, Region, WeatherStation |
| StormWarning | 2 | 2 | City |
| Subscription | 4 | 3 | Alert, Region, Severity, User |
| SubscriptionManager | 3 | 2 | Alert, Subscription, User |
| Temperature | 3 | 3 | Humidity, Wind |
| TemperatureSensor | 2 | 2 | Temperature |
| TrendAnalyzer | 3 | 2 | Pressure, Temperature |
| User | 5 | 2 | City |
| UvIndex | 2 | 2 | — |
| Visibility | 3 | 2 | — |
| WeatherEvent | 4 | 2 | Severity |
| WeatherSnapshot | 6 | 3 | CloudCover, Humidity, Precipitation, Pressure, Temperature, Wind |
| WeatherStation | 8 | 3 | Coordinates, HumiditySensor, PressureSensor, TemperatureSensor, WeatherSnapshot, WindSensor |
| WeeklyForecast | 4 | 3 | City, Confidence, DailyForecast |
| Wind | 4 | 3 | — |
| WindSensor | 2 | 2 | Wind |

### Исключения (13)

- WeatherException
- InvalidMeasurementException
- InvalidCoordinatesException
- SensorMalfunctionException
- SensorNotCalibratedException
- StationNotFoundException
- ForecastNotAvailableException
- UnsupportedHorizonException
- InvalidAlertException
- NotificationFailedException
- InvalidUserDataException
- CapacityExceededException
- DataSourceUnavailableException

### Итоговая статистика

| Показатель | Значение |
|---|---|
| Классы | 50 |
| Поля | 173 |
| Поведения | 116 |
| Ассоциации | 88 |
| Исключения | 13 |