#ifndef CANTEENFLOW_CANTEEN_MANAGER_H
#define CANTEENFLOW_CANTEEN_MANAGER_H

#include "OrderList.h"
#include "OrderQueue.h"

#include <iostream>
#include <string>

class CanteenManager {
public:
    explicit CanteenManager(const std::string& filePath = "data/orders.txt");

    CanteenManager(const CanteenManager&) = delete;
    CanteenManager& operator=(const CanteenManager&) = delete;

    bool loadFromFile(std::ostream& messages = std::cout);
    bool saveToFile(std::ostream& messages = std::cout) const;
    int placeOrder(const std::string& items);
    bool processNextOrder();
    bool completeOrder(int token);
    bool cancelOrder(int token);
    const Order* findOrder(int token) const;
    void displayAllOrders() const;
    void displayPendingOrdersAndQueue() const;
    void displayOrderStatus(int token) const;
    void generateTodaysReport() const;

private:
    static void getCurrentDateTime(std::string& date, std::string& time);
    bool rebuildPendingQueue(std::ostream& messages);

    OrderList orders_;
    OrderQueue pendingQueue_;
    long long nextToken_;
    std::string filePath_;
};

#endif
