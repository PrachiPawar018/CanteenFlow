#include <ctime>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <limits>
#include <new>
#include <sstream>
#include <string>

enum class OrderStatus {
    PENDING,
    PREPARING,
    COMPLETED,
    CANCELLED
};

const char* statusToString(OrderStatus status) {
    switch (status) {
    case OrderStatus::PENDING: return "PENDING";
    case OrderStatus::PREPARING: return "PREPARING";
    case OrderStatus::COMPLETED: return "COMPLETED";
    case OrderStatus::CANCELLED: return "CANCELLED";
    }
    return "UNKNOWN";
}

bool parseOrderStatus(const std::string& text, OrderStatus& status) {
    if (text == "PENDING") {
        status = OrderStatus::PENDING;
    } else if (text == "PREPARING") {
        status = OrderStatus::PREPARING;
    } else if (text == "COMPLETED") {
        status = OrderStatus::COMPLETED;
    } else if (text == "CANCELLED") {
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

class Order {
public:
    Order(int token, const std::string& items, const std::string& date,
          const std::string& time, OrderStatus status)
        : token_(token), items_(items), date_(date), time_(time), status_(status),
          next_(nullptr) {}

    int token() const { return token_; }
    const std::string& items() const { return items_; }
    const std::string& date() const { return date_; }
    const std::string& time() const { return time_; }
    OrderStatus status() const { return status_; }
    Order* next() const { return next_; }

private:
    friend class OrderList;

    int token_;
    std::string items_;
    std::string date_;
    std::string time_;
    OrderStatus status_;
    Order* next_;
};

class OrderList {
public:
    OrderList() : head_(nullptr), tail_(nullptr), size_(0) {}
    OrderList(const OrderList&) = delete;
    OrderList& operator=(const OrderList&) = delete;

    ~OrderList() { clear(); }

    bool insert(Order* order) {
        if (order == nullptr || find(order->token()) != nullptr) {
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

    Order* find(int token) const {
        for (Order* current = head_; current != nullptr; current = current->next_) {
            if (current->token_ == token) {
                return current;
            }
        }
        return nullptr;
    }

    bool updateStatus(int token, OrderStatus status) {
        Order* order = find(token);
        if (order == nullptr || !isValidStatusTransition(order->status_, status)) {
            return false;
        }
        order->status_ = status;
        return true;
    }

    void displayAll() const {
        if (head_ == nullptr) {
            std::cout << "No orders have been placed yet.\n";
            return;
        }
        for (Order* current = head_; current != nullptr; current = current->next_) {
            displayOrder(*current);
        }
    }

    void displayByStatus(OrderStatus status) const {
        bool found = false;
        for (Order* current = head_; current != nullptr; current = current->next_) {
            if (current->status_ == status) {
                displayOrder(*current);
                found = true;
            }
        }
        if (!found) {
            std::cout << "No " << statusToString(status) << " orders.\n";
        }
    }

    Order* head() const { return head_; }
    int size() const { return size_; }

private:
    static void displayOrder(const Order& order) {
        std::cout << "Token: " << order.token()
                  << " | Items: " << order.items()
                  << " | Date: " << order.date()
                  << " | Time: " << order.time()
                  << " | Status: " << statusToString(order.status()) << '\n';
    }

    void clear() {
        while (head_ != nullptr) {
            Order* oldHead = head_;
            head_ = head_->next_;
            delete oldHead;
        }
        tail_ = nullptr;
        size_ = 0;
    }

    Order* head_;
    Order* tail_;
    int size_;
};

class OrderQueue {
private:
    class Node {
    public:
        explicit Node(int orderToken) : token(orderToken), next(nullptr) {}

        int token;
        Node* next;
    };

public:
    OrderQueue() : front_(nullptr), rear_(nullptr), size_(0) {}
    OrderQueue(const OrderQueue&) = delete;
    OrderQueue& operator=(const OrderQueue&) = delete;

    ~OrderQueue() { clear(); }

    bool enqueue(int token) {
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

    bool dequeue(int& token) {
        if (isEmpty()) {
            return false;
        }
        Node* oldFront = front_;
        token = oldFront->token;
        front_ = front_->next;
        if (front_ == nullptr) {
            rear_ = nullptr;
        }
        delete oldFront;
        --size_;
        return true;
    }

    bool front(int& token) const {
        if (isEmpty()) {
            return false;
        }
        token = front_->token;
        return true;
    }

    bool remove(int token) {
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

    bool isEmpty() const { return front_ == nullptr; }

    void display() const {
        if (isEmpty()) {
            std::cout << "Pending queue is empty.\n";
            return;
        }
        std::cout << "Pending queue (front to rear): ";
        for (Node* current = front_; current != nullptr; current = current->next) {
            std::cout << current->token;
            if (current->next != nullptr) {
                std::cout << " -> ";
            }
        }
        std::cout << '\n';
    }

    void clear() {
        while (front_ != nullptr) {
            Node* oldFront = front_;
            front_ = front_->next;
            delete oldFront;
        }
        rear_ = nullptr;
        size_ = 0;
    }

private:
    Node* front_;
    Node* rear_;
    int size_;
};

class CanteenManager {
public:
    explicit CanteenManager(const std::string& filePath = "data/orders.txt")
        : nextToken_(1), filePath_(filePath) {}

    CanteenManager(const CanteenManager&) = delete;
    CanteenManager& operator=(const CanteenManager&) = delete;

    bool loadFromFile() {
        std::error_code filesystemError;
        const bool fileExists = std::filesystem::exists(filePath_, filesystemError);
        if (filesystemError) {
            std::cout << "Could not check order data file: " << filesystemError.message()
                      << '\n';
            return false;
        }
        if (!fileExists) {
            return true;
        }

        std::ifstream input(filePath_);
        if (!input.is_open()) {
            std::cout << "Could not open order data file for reading: "
                      << filePath_ << '\n';
            return false;
        }

        std::string line;
        if (!std::getline(input, line) || line != "CANTEENFLOW 1") {
            std::cout << "Order data file has an unsupported or malformed header.\n";
            return false;
        }

        long long savedNextToken = 1;
        if (std::getline(input, line)) {
            std::istringstream metadata(line);
            std::string label;
            std::string extra;
            long long candidate = 0;
            if ((metadata >> label >> candidate) && !(metadata >> extra) &&
                label == "NEXT_TOKEN" && candidate > 0) {
                savedNextToken = candidate;
            } else {
                std::cout << "Warning: invalid token-counter metadata; "
                             "recovering from records.\n";
            }
        } else {
            std::cout << "Warning: token-counter metadata is missing; "
                         "recovering from records.\n";
        }

        int maximumToken = 0;
        int lineNumber = 2;
        while (std::getline(input, line)) {
            ++lineNumber;
            if (line.empty()) {
                continue;
            }

            int token = 0;
            std::string date;
            std::string time;
            std::string statusText;
            std::string items;
            std::string marker;
            std::string extra;
            OrderStatus status = OrderStatus::PENDING;
            std::istringstream record(line);

            if (!(record >> marker >> token >> std::quoted(date) >> std::quoted(time) >>
                  statusText >> std::quoted(items)) ||
                (record >> extra) || marker != "ORDER" || token <= 0 ||
                date.empty() || time.empty() || items.empty() ||
                !parseOrderStatus(statusText, status)) {
                std::cout << "Warning: skipped malformed order record at line "
                          << lineNumber << ".\n";
                continue;
            }
            if (orders_.find(token) != nullptr) {
                std::cout << "Warning: skipped duplicate token " << token
                          << " at line " << lineNumber << ".\n";
                continue;
            }

            Order* order = new (std::nothrow)
                Order(token, items, date, time, status);
            if (order == nullptr || !orders_.insert(order)) {
                delete order;
                std::cout << "Warning: could not load order at line "
                          << lineNumber << ".\n";
                continue;
            }
            if (token > maximumToken) {
                maximumToken = token;
            }
        }

        if (input.bad()) {
            std::cout << "An error occurred while reading order data.\n";
            return false;
        }

        nextToken_ = savedNextToken;
        const long long minimumNextToken = static_cast<long long>(maximumToken) + 1;
        if (nextToken_ < minimumNextToken) {
            nextToken_ = minimumNextToken;
        }
        return rebuildPendingQueue();
    }

    bool saveToFile() const {
        std::ofstream output(filePath_, std::ios::out | std::ios::trunc);
        if (!output.is_open()) {
            std::cout << "Error: could not open " << filePath_ << " for saving.\n";
            return false;
        }

        output << "CANTEENFLOW 1\n";
        output << "NEXT_TOKEN " << nextToken_ << '\n';
        for (Order* order = orders_.head(); order != nullptr; order = order->next()) {
            output << "ORDER " << order->token() << ' '
                   << std::quoted(order->date()) << ' '
                   << std::quoted(order->time()) << ' '
                   << statusToString(order->status()) << ' '
                   << std::quoted(order->items()) << '\n';
        }
        output.close();
        if (!output) {
            std::cout << "Error: failed while writing order data to "
                      << filePath_ << ".\n";
            return false;
        }
        return true;
    }

    int placeOrder(const std::string& items) {
        if (items.find_first_not_of(" \t\r\n") == std::string::npos) {
            std::cout << "Order items cannot be empty.\n";
            return 0;
        }
        if (nextToken_ > std::numeric_limits<int>::max()) {
            std::cout << "No more integer tokens are available.\n";
            return 0;
        }

        const int token = static_cast<int>(nextToken_);
        std::string date;
        std::string time;
        getCurrentDateTime(date, time);

        Order* order = new (std::nothrow)
            Order(token, items, date, time, OrderStatus::PENDING);
        if (order == nullptr) {
            std::cout << "Unable to allocate memory for the new order.\n";
            return 0;
        }

        try {
            pendingQueue_.enqueue(token);
        } catch (const std::bad_alloc&) {
            delete order;
            std::cout << "Unable to allocate memory for the pending queue.\n";
            return 0;
        }

        if (!orders_.insert(order)) {
            pendingQueue_.remove(token);
            delete order;
            std::cout << "Unable to add the order to the master order list.\n";
            return 0;
        }

        ++nextToken_;
        std::cout << "Order placed successfully. Your token is " << token << ".\n";
        saveToFile();
        return token;
    }

    bool processNextOrder() {
        int token = 0;
        while (pendingQueue_.front(token)) {
            Order* order = orders_.find(token);
            if (order == nullptr || order->status() != OrderStatus::PENDING) {
                int discardedToken = 0;
                pendingQueue_.dequeue(discardedToken);
                std::cout << "Removed stale queue entry for token "
                          << discardedToken << ".\n";
                continue;
            }

            if (!orders_.updateStatus(token, OrderStatus::PREPARING)) {
                std::cout << "Could not process token " << token
                          << " because its status transition is invalid.\n";
                return false;
            }
            int processedToken = 0;
            if (!pendingQueue_.dequeue(processedToken)) {
                orders_.updateStatus(token, OrderStatus::PENDING);
                std::cout << "Could not remove the processed order from the queue.\n";
                return false;
            }
            std::cout << "Order " << processedToken << " is now PREPARING.\n";
            saveToFile();
            return true;
        }
        std::cout << "There are no pending orders to process.\n";
        return false;
    }

    bool completeOrder(int token) {
        Order* order = orders_.find(token);
        if (order == nullptr) {
            std::cout << "No order exists with token " << token << ".\n";
            return false;
        }
        if (order->status() != OrderStatus::PREPARING) {
            std::cout << "Only PREPARING orders can be marked COMPLETED. "
                      << "Current status: " << statusToString(order->status()) << ".\n";
            return false;
        }
        if (!orders_.updateStatus(token, OrderStatus::COMPLETED)) {
            std::cout << "The status transition was rejected.\n";
            return false;
        }
        std::cout << "Order " << token << " is now COMPLETED.\n";
        saveToFile();
        return true;
    }

    bool cancelOrder(int token) {
        Order* order = orders_.find(token);
        if (order == nullptr) {
            std::cout << "No order exists with token " << token << ".\n";
            return false;
        }
        if (order->status() != OrderStatus::PENDING) {
            std::cout << "Only PENDING orders can be cancelled. Current status: "
                      << statusToString(order->status()) << ".\n";
            return false;
        }
        if (!pendingQueue_.remove(token)) {
            std::cout << "Could not cancel the order: its pending queue entry is missing.\n";
            return false;
        }
        if (!orders_.updateStatus(token, OrderStatus::CANCELLED)) {
            pendingQueue_.enqueue(token);
            std::cout << "The status transition was rejected; queue entry was restored.\n";
            return false;
        }
        std::cout << "Order " << token << " has been CANCELLED.\n";
        saveToFile();
        return true;
    }

    void displayAllOrders() const { orders_.displayAll(); }

    void displayPendingOrdersAndQueue() const {
        std::cout << "Pending orders:\n";
        orders_.displayByStatus(OrderStatus::PENDING);
        pendingQueue_.display();
    }

    void displayOrderStatus(int token) const {
        const Order* order = orders_.find(token);
        if (order == nullptr) {
            std::cout << "No order exists with token " << token << ".\n";
            return;
        }
        std::cout << "Token: " << order->token()
                  << "\nItems: " << order->items()
                  << "\nDate: " << order->date()
                  << "\nTime: " << order->time()
                  << "\nStatus: " << statusToString(order->status()) << '\n';
    }

    void generateTodaysReport() const {
        std::string today;
        std::string ignoredTime;
        getCurrentDateTime(today, ignoredTime);

        int total = 0;
        int pending = 0;
        int preparing = 0;
        int completed = 0;
        int cancelled = 0;
        for (Order* order = orders_.head(); order != nullptr; order = order->next()) {
            if (order->date() != today) {
                continue;
            }
            ++total;
            switch (order->status()) {
            case OrderStatus::PENDING: ++pending; break;
            case OrderStatus::PREPARING: ++preparing; break;
            case OrderStatus::COMPLETED: ++completed; break;
            case OrderStatus::CANCELLED: ++cancelled; break;
            }
        }

        std::cout << "Daily report for " << today << ":\n"
                  << "Total orders: " << total << '\n'
                  << "PENDING: " << pending << '\n'
                  << "PREPARING: " << preparing << '\n'
                  << "COMPLETED: " << completed << '\n'
                  << "CANCELLED: " << cancelled << '\n';
    }

private:
    static void getCurrentDateTime(std::string& date, std::string& time) {
        const std::time_t now = std::time(nullptr);
        std::tm localTime{};
#ifdef _WIN32
        localtime_s(&localTime, &now);
#else
        localtime_r(&now, &localTime);
#endif
        std::ostringstream dateStream;
        std::ostringstream timeStream;
        dateStream << std::put_time(&localTime, "%Y-%m-%d");
        timeStream << std::put_time(&localTime, "%H:%M:%S");
        date = dateStream.str();
        time = timeStream.str();
    }

    bool rebuildPendingQueue() {
        try {
            for (Order* order = orders_.head(); order != nullptr; order = order->next()) {
                if (order->status() == OrderStatus::PENDING) {
                    pendingQueue_.enqueue(order->token());
                }
            }
        } catch (const std::bad_alloc&) {
            pendingQueue_.clear();
            std::cout << "Error: insufficient memory to rebuild the pending queue.\n";
            return false;
        }
        return true;
    }

    OrderList orders_;
    OrderQueue pendingQueue_;
    long long nextToken_;
    std::string filePath_;
};

bool readInteger(const std::string& prompt, int& value) {
    std::string input;
    std::cout << prompt;
    if (!std::getline(std::cin, input)) {
        return false;
    }

    std::istringstream parser(input);
    std::string extra;
    if (!(parser >> value) || (parser >> extra)) {
        std::cout << "Please enter a valid whole number.\n";
        return false;
    }
    return true;
}

void displayMenu() {
    std::cout << "\n========== CANTEEN ORDER MANAGEMENT SYSTEM ==========\n"
              << "1. Place New Order\n"
              << "2. Display All Orders\n"
              << "3. Display Pending Orders and Queue\n"
              << "4. Process Next Order\n"
              << "5. Mark Order as Completed\n"
              << "6. Check Order Status by Token\n"
              << "7. Cancel Pending Order\n"
              << "8. Generate Today's Daily Report\n"
              << "9. Save Orders to File\n"
              << "10. Exit\n";
}

int main() {
    CanteenManager manager;
    if (!manager.loadFromFile()) {
        std::cout << "Order history could not be fully loaded. "
                     "Check the messages above.\n";
    }

    bool running = true;
    while (running) {
        displayMenu();
        int choice = 0;
        if (!readInteger("Enter your choice: ", choice)) {
            if (!std::cin) {
                break;
            }
            continue;
        }

        switch (choice) {
        case 1: {
            std::string items;
            std::cout << "Enter one or more food items: ";
            if (!std::getline(std::cin, items)) {
                running = false;
                break;
            }
            manager.placeOrder(items);
            break;
        }
        case 2:
            manager.displayAllOrders();
            break;
        case 3:
            manager.displayPendingOrdersAndQueue();
            break;
        case 4:
            manager.processNextOrder();
            break;
        case 5:
        case 6:
        case 7: {
            int token = 0;
            if (!readInteger("Enter order token: ", token)) {
                if (!std::cin) {
                    running = false;
                }
                break;
            }
            if (choice == 5) {
                manager.completeOrder(token);
            } else if (choice == 6) {
                manager.displayOrderStatus(token);
            } else {
                manager.cancelOrder(token);
            }
            break;
        }
        case 8:
            manager.generateTodaysReport();
            break;
        case 9:
            if (manager.saveToFile()) {
                std::cout << "Orders saved successfully.\n";
            }
            break;
        case 10:
            manager.saveToFile();
            std::cout << "Orders saved. Goodbye.\n";
            running = false;
            break;
        default:
            std::cout << "Invalid menu choice. Enter a number from 1 to 10.\n";
            break;
        }
    }
    return 0;
}