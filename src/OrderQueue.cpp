#include "OrderQueue.h"

#include <iostream>
#include <new>

class OrderQueue::Node {
public:
    explicit Node(int orderToken) : token(orderToken), next(nullptr) {}

    int token;
    Node* next;
};

OrderQueue::OrderQueue() : front_(nullptr), rear_(nullptr), size_(0) {}

OrderQueue::~OrderQueue() { clear(); }

bool OrderQueue::enqueue(int token) {
    Node* node = new Node(token);
    if (rear_ == nullptr) {
        front_ = rear_ = node;
    } else {
        rear_->next = node;
        rear_ = node;
    }
    ++size_;
    return true;
}

bool OrderQueue::dequeue(int& token) {
    if (isEmpty()) {
        return false;
    }
    Node* removed = front_;
    token = removed->token;
    front_ = front_->next;
    if (front_ == nullptr) {
        rear_ = nullptr;
    }
    delete removed;
    --size_;
    return true;
}

bool OrderQueue::front(int& token) const {
    if (isEmpty()) {
        return false;
    }
    token = front_->token;
    return true;
}

bool OrderQueue::remove(int token) {
    Node* previous = nullptr;
    Node* current = front_;
    while (current != nullptr && current->token != token) {
        previous = current;
        current = current->next;
    }
    if (current == nullptr) {
        return false;
    }

    if (previous == nullptr) {
        front_ = current->next;
    } else {
        previous->next = current->next;
    }
    if (current == rear_) {
        rear_ = previous;
    }
    delete current;
    --size_;
    return true;
}

bool OrderQueue::isEmpty() const { return front_ == nullptr; }

void OrderQueue::display(std::ostream& output) const {
    if (isEmpty()) {
        output << "Pending queue is empty.\n";
        return;
    }
    output << "Pending queue (front to rear): ";
    for (Node* current = front_; current != nullptr; current = current->next) {
        output << current->token;
        if (current->next != nullptr) {
            output << " -> ";
        }
    }
    output << '\n';
}

int OrderQueue::getSize() const { return size_; }

void OrderQueue::clear() {
    while (front_ != nullptr) {
        Node* removed = front_;
        front_ = front_->next;
        delete removed;
    }
    rear_ = nullptr;
    size_ = 0;
}
