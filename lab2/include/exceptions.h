#pragma once
#include <stdexcept>
#include <string>

class TheatreException : public std::runtime_error {
public:
    explicit TheatreException(const std::string& message)
        : std::runtime_error(message) {}
};

class SeatAlreadyBookedException : public TheatreException {
public:
    explicit SeatAlreadyBookedException(const std::string& message)
        : TheatreException(message) {}
};

class HallOverCapacityException : public TheatreException {
public:
    explicit HallOverCapacityException(const std::string& message)
        : TheatreException(message) {}
};

class InvalidTicketException : public TheatreException {
public:
    explicit InvalidTicketException(const std::string& message)
        : TheatreException(message) {}
};

class InsufficientFundsException : public TheatreException {
public:
    explicit InsufficientFundsException(const std::string& message)
        : TheatreException(message) {}
};

class PerformanceNotFoundException : public TheatreException {
public:
    explicit PerformanceNotFoundException(const std::string& message)
        : TheatreException(message) {}
};

class ActorNotAvailableException : public TheatreException {
public:
    explicit ActorNotAvailableException(const std::string& message)
        : TheatreException(message) {}
};

class RehearsalConflictException : public TheatreException {
public:
    explicit RehearsalConflictException(const std::string& message)
        : TheatreException(message) {}
};

class InvalidPromoCodeException : public TheatreException {
public:
    explicit InvalidPromoCodeException(const std::string& message)
        : TheatreException(message) {}
};

class ContractExpiredException : public TheatreException {
public:
    explicit ContractExpiredException(const std::string& message)
        : TheatreException(message) {}
};

class SubscriptionExpiredException : public TheatreException {
public:
    explicit SubscriptionExpiredException(const std::string& message)
        : TheatreException(message) {}
};

class RefundNotAllowedException : public TheatreException {
public:
    explicit RefundNotAllowedException(const std::string& message)
        : TheatreException(message) {}
};

class InvalidCastException : public TheatreException {
public:
    explicit InvalidCastException(const std::string& message)
        : TheatreException(message) {}
};