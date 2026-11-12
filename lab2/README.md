# Лабораторная работа №2. Крупный ООП-проект: «Театр»

Объектная модель театра: репертуар и постановки, залы и расписание, продажа билетов,
закулисье, финансы и кадры, программа лояльности.

Формат строки: `Класс | число полей | число методов | связанные классы`.

| Класс | Поля | Методы | Ассоциации (связанные классы) |
|---|---|---|---|
| Actor | 3 | 2 | Role |
| Administrator | 2 | 2 | Contract, Refund |
| Booking | 5 | 4 | Payment, Spectator, Ticket |
| Budget | 3 | 4 | Expense |
| Cashier | 3 | 2 | Payment, Receipt |
| Cast | 3 | 4 | Actor, Production, Role |
| Contract | 3 | 3 | Employee |
| Costume | 4 | 2 | Role |
| CostumeDepartment | 3 | 2 | Costume, Role |
| Decoration | 3 | 2 | Production |
| Director | 4 | 2 | Cast, Production |
| Discount | 3 | 2 | — |
| Employee | 3 | 2 | — |
| Expense | 4 | 2 | — |
| FinancialReport | 3 | 2 | Budget |
| Genre | 3 | 1 | — |
| Hall | 4 | 3 | Row, Stage |
| LightingSystem | 3 | 2 | — |
| LoyaltyProgram | 3 | 4 | Discount, Spectator, Ticket |
| Payment | 4 | 2 | Spectator |
| Payroll | 3 | 3 | Budget, Salary |
| Performance | 4 | 3 | Hall, Production |
| Person | 3 | 2 | — |
| Play | 4 | 2 | Genre, Playwright |
| Playwright | 3 | 1 | Play |
| Production | 4 | 3 | Director, Play |
| PromoCode | 4 | 2 | Discount |
| Prop | 3 | 2 | — |
| Receipt | 3 | 2 | Payment |
| Refund | 3 | 2 | Ticket |
| Rehearsal | 4 | 2 | Production, Stage |
| Review | 4 | 2 | Performance, Spectator |
| Role | 3 | 2 | — |
| Row | 3 | 2 | Seat |
| Salary | 4 | 2 | Employee |
| Schedule | 2 | 3 | Performance, Season |
| Script | 3 | 2 | Play |
| Season | 4 | 2 | Production |
| Seat | 3 | 3 | SeatCategory |
| SeatCategory | 3 | 1 | — |
| SoundSystem | 4 | 2 | Actor |
| Spectator | 3 | 3 | — |
| Sponsor | 3 | 2 | Budget |
| Stage | 3 | 2 | Decoration |
| Subscription | 4 | 3 | Spectator |
| Theatre | 5 | 3 | Budget, Cashier, Hall, Performance, Production, Season, TicketOffice |
| Ticket | 6 | 3 | Performance, Seat, Spectator |
| TicketOffice | 3 | 3 | Cashier, Performance, Schedule, Seat, Spectator, Ticket |
| Usher | 3 | 2 | Seat, Ticket |
| Warehouse | 3 | 3 | Prop |

### Исключения (13)

- TheatreException
- SeatAlreadyBookedException
- HallOverCapacityException
- InvalidTicketException
- InsufficientFundsException
- PerformanceNotFoundException
- ActorNotAvailableException
- RehearsalConflictException
- InvalidPromoCodeException
- ContractExpiredException
- SubscriptionExpiredException
- RefundNotAllowedException
- InvalidCastException

### Итоговая статистика

| Показатель | Значение |
|---|---|
| Классы | 50 |
| Поля | 170 |
| Поведения | 118 |
| Ассоциации | 72 |
| Исключения | 13 |