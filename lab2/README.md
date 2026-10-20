# Лабораторная работа №2. Крупный ООП-проект: «Театр»

Объектная модель театра: репертуар и постановки, залы и расписание, продажа билетов,
закулисье, финансы и кадры, программа лояльности.

Формат строки: `Класс | число полей | число методов | связанные классы`.

| Класс | Поля | Методы | Ассоциации (связанные классы) |
|---|---|---|---|
| Actor | 3 | 2 | Role |
| Administrator | 2 | 2 | Refund, Contract |
| Booking | 4 | 3 | Ticket, Spectator, Payment |
| Budget | 3 | 3 | Expense |
| Cashier | 3 | 2 | Payment, Receipt |
| Cast | 3 | 3 | Production, Role, Actor |
| Contract | 3 | 3 | Employee |
| Costume | 4 | 2 | Role |
| CostumeDepartment | 3 | 2 | Costume, Role |
| Decoration | 3 | 2 | Production |
| Director | 3 | 2 | Production, Cast |
| Discount | 3 | 2 | — |
| Employee | 3 | 2 | — |
| Expense | 4 | 2 | — |
| FinancialReport | 3 | 2 | Budget |
| Genre | 3 | 1 | — |
| Hall | 4 | 3 | Row, Stage |
| LightingSystem | 3 | 2 | — |
| LoyaltyProgram | 3 | 3 | Spectator, Ticket, Discount |
| Payment | 4 | 2 | Spectator |
| Payroll | 3 | 3 | Salary, Budget |
| Performance | 4 | 3 | Production, Hall |
| Person | 3 | 2 | — |
| Play | 4 | 2 | Genre, Playwright |
| Playwright | 3 | 2 | Play |
| Production | 4 | 3 | Play, Director |
| Prop | 3 | 2 | — |
| PromoCode | 4 | 2 | Discount |
| Receipt | 3 | 2 | Payment |
| Refund | 3 | 2 | Ticket |
| Rehearsal | 4 | 2 | Production, Stage |
| Review | 4 | 2 | Spectator, Performance |
| Role | 3 | 2 | — |
| Row | 3 | 2 | Seat |
| Salary | 4 | 2 | Employee |
| Schedule | 2 | 3 | Performance, Season |
| Script | 3 | 2 | Play |
| Seat | 3 | 3 | SeatCategory |
| SeatCategory | 3 | 1 | — |
| Season | 4 | 2 | Production |
| SoundSystem | 3 | 2 | Actor |
| Spectator | 3 | 3 | — |
| Sponsor | 3 | 2 | Budget |
| Stage | 3 | 2 | Decoration |
| Subscription | 4 | 3 | Spectator |
| Theatre | 5 | 3 | Hall, Schedule, TicketOffice, Season, Performance, Budget |
| Ticket | 5 | 3 | Performance, Seat, Spectator |
| TicketOffice | 3 | 3 | Schedule, Cashier, Performance, Seat, Spectator, Ticket |
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
| Поля | 166 |
| Поведения | 115 |
| Ассоциации | 71 |
| Исключения | 13 |