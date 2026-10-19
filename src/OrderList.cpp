#include "OrderList.h"

#include <iostream>

OrderList::OrderList() : head_(nullptr), tail_(nullptr), size_(0) {}

OrderList::~OrderList() { clear(); }

bool OrderList::insert(Order* order) {
    if (order == nullptr || find(order->getToken()) != nullptr) {
        return false;
    }

    order->next_ = nullptr;
    if (tail_ == nullptr) {
        head_ = order;
    } else {
        tail_->next_ = order;
    }
    tail_ = order;
    ++size_;
    return true;
}

Order* OrderList::find(int token) const {
    for (Order* current = head_; current != nullptr; current = current->next_) {
        if (current->token_ == token) {
            return current;
        }
    }
    return nullptr;
}

bool OrderList::updateStatus(int token, OrderStatus newStatus) {
    Order* order = find(token);
    if (order == nullptr || !isValidStatusTransition(order->status_, newStatus)) {
        return false;
    }
    order->status_ = newStatus;
    return true;
}

void OrderList::displayAll(std::ostream& output) const {
    if (head_ == nullptr) {
        output << "No orders have been placed yet.\n";
        return;
    }
    for (Order* current = head_; current != nullptr; current = current->next_) {
        displayOrder(*current, output);
    }
}

void OrderList::displayByStatus(OrderStatus status, std::ostream& output) const {
    bool found = false;
    for (Order* current = head_; current != nullptr; current = current->next_) {
        if (current->status_ == status) {
            displayOrder(*current, output);
            found = true;
        }
    }
    if (!found) {
        output << "No " << statusToString(status) << " orders.\n";
    }
}

Order* OrderList::getHead() const { return head_; }

int OrderList::getSize() const { return size_; }

void OrderList::displayOrder(const Order& order, std::ostream& output) {
    output << "Token: " << order.getToken()
           << " | Items: " << order.getItems()
           << " | Date: " << order.getDate()
           << " | Time: " << order.getTime()
           << " | Status: " << statusToString(order.getStatus()) << '\n';
}

void OrderList::clear() {
    while (head_ != nullptr) {
        Order* removed = head_;
        head_ = head_->next_;
        delete removed;
    }
    tail_ = nullptr;
    size_ = 0;
}
