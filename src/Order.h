#ifndef CANTEENFLOW_ORDER_H
#define CANTEENFLOW_ORDER_H

#include <string>

enum class OrderStatus {
    PENDING,
    PREPARING,
    COMPLETED,
    CANCELLED
};

const char* statusToString(OrderStatus status);
bool parseOrderStatus(const std::string& value, OrderStatus& status);
bool isValidStatusTransition(OrderStatus current, OrderStatus next);

class Order {
public:
    Order(int token, const std::string& items, const std::string& date,
          const std::string& time, OrderStatus status);

    int getToken() const;
    const std::string& getItems() const;
    const std::string& getDate() const;
    const std::string& getTime() const;
    OrderStatus getStatus() const;
    Order* getNext() const;

private:
    friend class OrderList;

    int token_;
    std::string items_;
    std::string date_;
    std::string time_;
    OrderStatus status_;
    Order* next_;
};

#endif
