#ifndef CANTEENFLOW_ORDER_LIST_H
#define CANTEENFLOW_ORDER_LIST_H

#include "Order.h"

#include <iosfwd>

class OrderList {
public:
    OrderList();

    OrderList(const OrderList&) = delete;
    OrderList& operator=(const OrderList&) = delete;

    ~OrderList();

    bool insert(Order* order);
    Order* find(int token) const;
    bool updateStatus(int token, OrderStatus newStatus);
    void displayAll(std::ostream& output) const;
    void displayByStatus(OrderStatus status, std::ostream& output) const;
    Order* getHead() const;
    int getSize() const;

private:
    static void displayOrder(const Order& order, std::ostream& output);
    void clear();

    Order* head_;
    Order* tail_;
    int size_;
};

#endif
