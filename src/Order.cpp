#include "Order.h"

const char* statusToString(OrderStatus status) {
    switch (status) {
    case OrderStatus::PENDING:
        return "PENDING";
    case OrderStatus::PREPARING:
        return "PREPARING";
    case OrderStatus::COMPLETED:
        return "COMPLETED";
    case OrderStatus::CANCELLED:
        return "CANCELLED";
    }
    return "UNKNOWN";
}

bool parseOrderStatus(const std::string& value, OrderStatus& status) {
    if (value == "PENDING") {
        status = OrderStatus::PENDING;
    } else if (value == "PREPARING") {
        status = OrderStatus::PREPARING;
    } else if (value == "COMPLETED") {
        status = OrderStatus::COMPLETED;
    } else if (value == "CANCELLED") {
        status = OrderStatus::CANCELLED;
    } else {
        return false;
    }
    return true;
}

bool isValidStatusTransition(OrderStatus current, OrderStatus next) {
    return (current == OrderStatus::PENDING &&
            (next == OrderStatus::PREPARING || next == OrderStatus::CANCELLED)) ||
           (current == OrderStatus::PREPARING && next == OrderStatus::COMPLETED);
}

Order::Order(int token, const std::string& items, const std::string& date,
             const std::string& time, OrderStatus status)
    : token_(token), items_(items), date_(date), time_(time), status_(status),
      next_(nullptr) {}

int Order::getToken() const { return token_; }
const std::string& Order::getItems() const { return items_; }
const std::string& Order::getDate() const { return date_; }
const std::string& Order::getTime() const { return time_; }
OrderStatus Order::getStatus() const { return status_; }
Order* Order::getNext() const { return next_; }
