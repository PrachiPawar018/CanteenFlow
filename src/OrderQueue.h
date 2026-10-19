#ifndef CANTEENFLOW_ORDER_QUEUE_H
#define CANTEENFLOW_ORDER_QUEUE_H

#include <iosfwd>

class OrderQueue {
private:
    class Node;

public:
    OrderQueue();

    OrderQueue(const OrderQueue&) = delete;
    OrderQueue& operator=(const OrderQueue&) = delete;

    ~OrderQueue();

    bool enqueue(int token);
    bool dequeue(int& token);
    bool front(int& token) const;
    bool remove(int token);
    bool isEmpty() const;
    void display(std::ostream& output) const;
    int getSize() const;
    void clear();

private:
    Node* front_;
    Node* rear_;
    int size_;
};

#endif
