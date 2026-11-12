#include <catch2/catch_test_macros.hpp>
#include <catch2/catch_approx.hpp> 
#include <string>
#include <vector>

#include "backstage_finance.h"
#include "exceptions.h"
#include "people.h"
#include "repertoire.h"
#include "sales.h"
#include "venue.h"

namespace {
Employee emp(int id = 1, const std::string& pos = "Актёр", const std::string& b = "1990-01-01") {
    return Employee("E" + std::to_string(id), b, "+" + std::to_string(id), id, pos, "2015-01-01");
}
Play play(const std::string& t = "P") {
    return Play(t, 120, Genre("Драма", "d", 12), Playwright("A", "C"));
}
Production prod(const std::string& p = "2026-11-01") {
    Director d(emp(1, "Режиссёр", "1970-01-01"), "s", 2, 5);
    Production x(play(), d, p); x.approve(); return x;
}
Hall hall(int cap = 10) {
    Hall h("Зал", cap, Stage(20, 10, 5000));
    SeatCategory c("Партер", 1.0, "d");
    Row r(1, "Партер"); r.addSeat(Seat(1, c)); r.addSeat(Seat(2, c));
    h.addRow(r); return h;
}
Spectator spec(int id = 1, double b = 1000.0) {
    return Spectator("S" + std::to_string(id), "2000-01-01", "+" + std::to_string(id), id, b);
}
}

// ============ 1. Person + Employee + Contract + Spectator ====================
TEST_CASE("Люди: Person, Employee, Contract, Spectator", "[people]") {
    Person p("T", "2000-06-15", "+1");
    REQUIRE(p.getAge("2026-06-14") == 25);
    REQUIRE(p.isAdult("2026-06-15"));
    REQUIRE_FALSE(p.isAdult("2017-01-01"));

    Employee e = emp(1, "Актёр", "1980-01-15");
    REQUIRE(e.calculateSeniority("2026-01-01") == 11);
    e.promote("X");
    REQUIRE_THROWS_AS(e.promote(""), TheatreException);

    Contract c(e, "2025-01-01", "2025-12-31");
    REQUIRE(c.isActive("2025-06-01"));
    REQUIRE_THROWS_AS(c.renew("2026-12-31", "2026-06-01"), ContractExpiredException);
    c.renew("2026-12-31", "2025-06-01");
    c.terminate("2025-07-01");
    REQUIRE(c.getEndDate() == "2025-07-01");
    REQUIRE_THROWS_AS(Contract(e, "2025-12-31", "2025-01-01"), TheatreException);

    Spectator s = spec(1, 100);
    s.topUpBalance(50); s.pay(30); s.addLoyaltyPoints(10);
    REQUIRE(s.getBalance() == Catch::Approx(120.0));
    REQUIRE_THROWS_AS(s.pay(200), InsufficientFundsException);
    REQUIRE_THROWS_AS(s.topUpBalance(-1), TheatreException);
}

// ============ 2. Salary + Payroll ============================================
TEST_CASE("Salary и Payroll", "[people][salary]") {
    Salary s(emp(), 1000, 0.13);
    REQUIRE(s.calculateNet() == Catch::Approx(870.0));
    s.addBonus(200);
    REQUIRE(s.calculateNet() == Catch::Approx(1044.0));
    REQUIRE_THROWS_AS(s.addBonus(-1), TheatreException);

    Budget b(2026, 5000);
    Payroll p("2026-11");
    p.addSalary(s); 
    p.payAll(b);
    REQUIRE(p.isPaid());
    REQUIRE(b.getRemaining() == Catch::Approx(3956.0));
    REQUIRE_THROWS_AS(p.payAll(b), TheatreException);
    
    Payroll p2("2026-12"); 
    p2.addSalary(s);
    
    // Исправление: создаем объект бюджета в переменной
    Budget poorBudget(2026, 10);
    REQUIRE_THROWS_AS(p2.payAll(poorBudget), InsufficientFundsException);
}

// ============ 3. Actor + Role + CostumeDepartment ============================
TEST_CASE("Actor, Role, CostumeDepartment", "[people][actor][costume]") {
    Actor a(emp(5), "S", 3);
    a.promoteRank();
    REQUIRE(a.getRank() == 4);
    Role r("H", 20, 50);
    REQUIRE(a.canPlay(r, "2026-01-01"));
    a.setAvailable(false);
    REQUIRE_FALSE(a.canPlay(r, "2026-01-01"));

    CostumeDepartment d("H", 500);
    Costume c("C", "M", 100, r);
    d.addCostume(c);
    REQUIRE(d.issueCostume(r).getName() == "C");
    REQUIRE_THROWS_AS(d.issueCostume(r), TheatreException);
    REQUIRE_THROWS_AS(CostumeDepartment("H", 50).addCostume(c), InsufficientFundsException);
}

// ============ 4. Director + Cast =============================================
TEST_CASE("Director и Cast", "[people][director][cast]") {
    Director d(emp(1, "Р", "1970-01-01"), "s", 3, 2);
    Production p = prod();
    d.assignProduction(p); d.assignProduction(p);
    REQUIRE_THROWS_AS(d.assignProduction(p), TheatreException);

    Role r("H", 20, 50);
    Cast c(p, {r});
    REQUIRE_THROWS_AS(c.approve(), InvalidCastException);
    
    Actor a(emp(2), "S", 3);
    c.assignActor(r, a, "2026-01-01");
    c.approve();
    REQUIRE(c.isApproved());

    Cast c2(p, {Role("Old", 90, 100)});
    REQUIRE_THROWS_AS(c2.assignActor(Role("Old", 90, 100), a, "2026-01-01"), ActorNotAvailableException);
}

// ============ 5. Genre + Playwright + Play + Script + Season + Production ====
TEST_CASE("Репертуар: Genre, Playwright, Play, Script, Season, Production", "[repertoire]") {
    Genre g("Драма", "d", 12);
    Playwright w("W", "C");
    Play p = play("T");
    w.addPlay(p);
    REQUIRE_THROWS_AS(w.addPlay(p), TheatreException);

    Script s(p, 1, 50); s.revise(60);
    REQUIRE(s.getVersion() == 2);

    Season season("S", "2026-09-01", "2027-06-30");
    season.addProduction(prod());
    
    Production pr = prod();
    pr.cancel();
    REQUIRE_THROWS_AS(pr.approve(), TheatreException);
}

// ============ 6. Costume + Decoration ========================================
TEST_CASE("Costume и Decoration", "[repertoire][costume][decoration]") {
    Role r("R", 20, 50);
    Costume c("C", "M", 50, r); c.repair();
    REQUIRE(c.isUsable());
    REQUIRE_THROWS_AS(Costume("X", "M", 101, r), TheatreException);

    Decoration d("D", 100, prod());
    REQUIRE(d.calculateAssemblyTime() == Catch::Approx(50.0));
}

// ============ 7. Venue: Seat, SeatCategory, Stage, Row, Hall =================
TEST_CASE("Venue: Seat, SeatCategory, Stage, Row, Hall", "[venue]") {
    SeatCategory cat("P", 1.5, "d");
    Seat s(1, cat); s.book();
    REQUIRE_THROWS_AS(s.book(), SeatAlreadyBookedException);
    
    Stage st(20, 10, 5000);
    Row r(1, "P");
    r.addSeat(Seat(1, cat));
    
    // Проверка HallOverCapacityException
    Hall h("H", 1, st); // Вместимость 1
    h.addRow(r);        // Занимает 1 место
    REQUIRE_THROWS_AS(h.addRow(r), HallOverCapacityException);
}

// ============ 8. Performance + Schedule + Rehearsal + Prop + Warehouse =======
TEST_CASE("Performance, Schedule, Rehearsal, Prop, Warehouse", "[venue]") {
    Production p = prod(); Hall h = hall();
    Performance pf(p, h, "2026-11-14 19:00");
    pf.start();
    REQUIRE_THROWS_AS(pf.cancel(), TheatreException);

    Schedule sch(Season("S", "2026-09-01", "2027-06-30"));
    sch.addPerformance(pf);
    REQUIRE_THROWS_AS(sch.findByDate("2026-12-01"), PerformanceNotFoundException);
    REQUIRE_THROWS_AS(sch.addPerformance(Performance(p, h, "2026-11-14 20:00")), TheatreException);

    Rehearsal r(p, h.getStage(), "2026-11-10 10:00", 120);
    REQUIRE(r.conflictsWith(Rehearsal(p, h.getStage(), "2026-11-10 11:00", 120)));
    REQUIRE_THROWS_AS(r.reschedule("2026-11-10 11:00", {Rehearsal(p, h.getStage(), "2026-11-10 10:00", 120)}), RehearsalConflictException);

    Prop pr("S", 10, 100);
    pr.use(); pr.writeOff(3);
    Warehouse w("L", 20); w.storeProp(pr);
    REQUIRE(w.takeProp("S").getName() == "S");
    REQUIRE_THROWS_AS(w.takeProp("S"), TheatreException);
}

// ============ 9. Discount + PromoCode + Payment + Receipt + Ticket ===========
TEST_CASE("Sales: Discount, PromoCode, Payment, Receipt, Ticket", "[sales]") {
    Discount d("P", 20, "2026-12-31");
    PromoCode pc("C", d, 1);
    pc.redeem("2026-06-01");
    REQUIRE_THROWS_AS(pc.redeem("2026-06-01"), InvalidPromoCodeException);

    Payment pay(1, 100, spec());
    pay.process();
    Receipt r(1, pay);
    REQUIRE(r.getTaxAmount() == Catch::Approx(100.0 * 0.2 / 1.2));

    Production p = prod(); Hall h = hall();
    SeatCategory c("A", 1, "d"); Spectator sp = spec();
    Performance pf(p, h, "2026-11-14 19:00");
    Ticket t(1, pf, Seat(1, c), sp, 100);
    t.markUsed();
    REQUIRE_THROWS_AS(t.validate(), InvalidTicketException);
}

// ============ 10. Booking + Refund + Subscription ============================
TEST_CASE("Booking, Refund, Subscription", "[sales]") {
    Production p = prod(); Hall h = hall();
    SeatCategory c("A", 1, "d"); Spectator sp = spec();
    Performance pf(p, h, "2026-11-14 19:00");
    Ticket t1(1, pf, Seat(1, c), sp, 100);

    Booking b(1, {t1}, sp, "2026-11-10");
    Payment ok(2, 100, sp);
    b.confirm(ok, "2026-11-09");
    REQUIRE_THROWS_AS(b.confirm(ok, "2026-11-09"), TheatreException);

    Refund full(t1, 10); full.approve();
    REQUIRE_THROWS_AS(Refund(t1, 0).approve(), RefundNotAllowedException);

    Spectator s = spec();
    Subscription sub(s, "2026-01-01", "2026-12-31", 1);
    sub.useVisit("2026-06-15");
    REQUIRE_THROWS_AS(sub.useVisit("2026-06-15"), SubscriptionExpiredException);
}

// ============ 11. Loyalty + TicketOffice =====================================
TEST_CASE("LoyaltyProgram и TicketOffice", "[sales]") {
    LoyaltyProgram lp(10, 20);
    Spectator s = spec(1, 500);
    lp.enroll(s);
    REQUIRE_THROWS_AS(lp.enroll(s), TheatreException);

    Hall h = hall(); Production p = prod();
    Schedule sch(Season("S", "2026-09-01", "2027-06-30"));
    Performance pf(p, h, "2026-11-14 19:00"); sch.addPerformance(pf);
    TicketOffice office(sch, Cashier(emp(2, "K", "1980-01-01"), 2, 1));

    Seat& s1 = pf.getHall().getRows()[0].getSeats()[0];
    Ticket t = office.sellTicket(pf, s1, s, 100);
    lp.awardPoints(s, t); lp.awardPoints(s, t);
    REQUIRE(lp.getDiscountFor(s).getPercent() == Catch::Approx(10.0));

    // Исправление: создаем переменную для зрителя
    Spectator stranger = spec(2); 
    REQUIRE_THROWS_AS(lp.awardPoints(stranger, t), TheatreException);

    // Исправление: создаем переменную для бедного зрителя
    Spectator poor = spec(3, 10);
    Seat& s2 = pf.getHall().getRows()[0].getSeats()[1];
    REQUIRE_THROWS_AS(office.sellTicket(pf, s2, poor, 100), InsufficientFundsException);

    REQUIRE(office.returnTicket(t.getTicketId(), s1, s, "2026-11-01") == Catch::Approx(100.0));
    REQUIRE_THROWS_AS(office.returnTicket(99, s1, s, "2026-11-01"), InvalidTicketException);
}

// ============ 12. Backstage + Finance ========================================
TEST_CASE("Lighting, Sound, Budget, Sponsor, Report, Review, Usher, Admin", "[backstage]") {
    LightingSystem ls(10);
    REQUIRE_THROWS_AS(ls.setScene("X"), TheatreException);

    SoundSystem ss(2);
    ss.setMuted(true);
    REQUIRE_THROWS_AS(ss.setVolume(50), TheatreException);

    Budget b(2026, 10000);
    b.addExpense(Expense("R", 12000, "Cat", "2026-06-01"));
    REQUIRE(b.isExceeded());

    Sponsor sp("S", 5000, "2026-01-01");
    Budget b2(2026, 1000); sp.donate(b2, "2026-06-01");
    REQUIRE_THROWS_AS(sp.donate(b2, "2027-06-01"), TheatreException);

    FinancialReport r("2026-01-01", "2026-12-31", b2);
    REQUIRE_THROWS_AS(FinancialReport("2026-12-31", "2026-01-01", b2), TheatreException);

    Production p = prod(); Hall h = hall();
    Performance pf(p, h, "2026-11-14 19:00");
    REQUIRE_THROWS_AS(Review(spec(), pf, 0, "x"), TheatreException);

    Usher u(emp(1, "Б", "1980-01-01"), "P", "09:00", "18:00");
    Ticket tk(1, pf, Seat(5, SeatCategory("A", 1, "d")), spec(), 100);
    REQUIRE(u.checkTicket(tk));
    
    Administrator low(emp(), 1, "B");
    Refund ref(Ticket(2, pf, Seat(1, SeatCategory("A",1,"d")), spec(), 100), 10);
    REQUIRE_THROWS_AS(low.approveRefund(ref), TheatreException);
}

// ============ 13. Theatre + Интеграция =======================================
TEST_CASE("Theatre: сквозной сценарий", "[theatre][integration]") {
    Hall h = hall();
    Theatre th("T", h, Season("S", "2026-09-01", "2027-06-30"), Cashier(emp(2, "K", "1980-01-01"), 2, 1), Budget(2026, 50000));
    Production p = prod();
    th.planSeason(p);
    Performance pf(p, h, "2026-11-14 19:00");
    th.registerPerformance(pf);

    Spectator s = spec(1, 1000);
    Seat& seat = pf.getHall().getRows()[0].getSeats()[0];
    Ticket t = th.getTicketOffice().sellTicket(pf, seat, s, 100);
    
    th.getBudget().addExpense(Expense("E", 500, "C", "2026-11-01"));
    FinancialReport rep("2026-09-01", "2026-12-31", th.getBudget());
    REQUIRE(rep.calculateProfit(th.getTotalRevenue()) == Catch::Approx(-400.0));

    th.getTicketOffice().returnTicket(t.getTicketId(), seat, s, "2026-11-01");
    REQUIRE(th.getTotalRevenue() == Catch::Approx(0.0));
}