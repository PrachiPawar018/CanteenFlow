#include "CanteenManager.h"

#include <ctime>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <limits>
#include <new>
#include <sstream>

CanteenManager::CanteenManager(const std::string& filePath)
    : nextToken_(1), filePath_(filePath) {}

bool CanteenManager::loadFromFile(std::ostream& messages) {
    std::error_code filesystemError;
    const bool fileExists = std::filesystem::exists(filePath_, filesystemError);
    if (filesystemError) {
        messages << "Could not check order data file: " << filesystemError.message()
                 << '\n';
        return false;
    }
    if (!fileExists) {
        return true;
    }

    std::ifstream input(filePath_);
    if (!input.is_open()) {
        messages << "Could not open order data file for reading: " << filePath_ << '\n';
        return false;
    }

    std::string line;
    if (!std::getline(input, line) || line != "CANTEENFLOW 1") {
        messages << "Order data file has an unsupported or malformed header.\n";
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
            messages << "Warning: invalid token-counter metadata; recovering from records.\n";
        }
    } else {
        messages << "Warning: token-counter metadata is missing; recovering from records.\n";
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
            messages << "Warning: skipped malformed order record at line " << lineNumber
                     << ".\n";
            continue;
        }
        if (orders_.find(token) != nullptr) {
            messages << "Warning: skipped duplicate token " << token
                     << " at line " << lineNumber << ".\n";
            continue;
        }

        Order* order = new (std::nothrow) Order(token, items, date, time, status);
        if (order == nullptr || !orders_.insert(order)) {
            delete order;
            messages << "Warning: could not load order at line " << lineNumber << ".\n";
            continue;
        }
        if (token > maximumToken) {
            maximumToken = token;
        }
    }

    if (input.bad()) {
        messages << "An error occurred while reading order data.\n";
        return false;
    }

    nextToken_ = savedNextToken;
    const long long minimumNextToken = static_cast<long long>(maximumToken) + 1;
    if (nextToken_ < minimumNextToken) {
        nextToken_ = minimumNextToken;
    }
    return rebuildPendingQueue(messages);
}

bool CanteenManager::saveToFile(std::ostream& messages) const {
    std::ofstream output(filePath_, std::ios::out | std::ios::trunc);
    if (!output.is_open()) {
        messages << "Error: could not open " << filePath_ << " for saving.\n";
        return false;
    }

    output << "CANTEENFLOW 1\n";
    output << "NEXT_TOKEN " << nextToken_ << '\n';
    for (Order* order = orders_.getHead(); order != nullptr; order = order->getNext()) {
        output << "ORDER " << order->getToken() << ' '
               << std::quoted(order->getDate()) << ' '
               << std::quoted(order->getTime()) << ' '
               << statusToString(order->getStatus()) << ' '
               << std::quoted(order->getItems()) << '\n';
    }
    output.close();
    if (!output) {
        messages << "Error: failed while writing order data to " << filePath_ << ".\n";
        return false;
    }
    return true;
}

int CanteenManager::placeOrder(const std::string& items) {
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
        if (!pendingQueue_.enqueue(token)) {
            delete order;
            std::cout << "Unable to add the order to the pending queue.\n";
            return 0;
        }
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

bool CanteenManager::processNextOrder() {
    int token = 0;
    while (pendingQueue_.front(token)) {
        Order* order = orders_.find(token);
        if (order == nullptr || order->getStatus() != OrderStatus::PENDING) {
            int discardedToken = 0;
            pendingQueue_.dequeue(discardedToken);
            std::cout << "Removed stale queue entry for token " << discardedToken << ".\n";
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

bool CanteenManager::completeOrder(int token) {
    Order* order = orders_.find(token);
    if (order == nullptr) {
        std::cout << "No order exists with token " << token << ".\n";
        return false;
    }
    if (order->getStatus() != OrderStatus::PREPARING) {
        std::cout << "Only PREPARING orders can be marked COMPLETED. Current status: "
                  << statusToString(order->getStatus()) << ".\n";
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

bool CanteenManager::cancelOrder(int token) {
    Order* order = orders_.find(token);
    if (order == nullptr) {
        std::cout << "No order exists with token " << token << ".\n";
        return false;
    }
    if (order->getStatus() != OrderStatus::PENDING) {
        std::cout << "Only PENDING orders can be cancelled. Current status: "
                  << statusToString(order->getStatus()) << ".\n";
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

const Order* CanteenManager::findOrder(int token) const {
    return orders_.find(token);
}

void CanteenManager::displayAllOrders() const {
    orders_.displayAll(std::cout);
}

void CanteenManager::displayPendingOrdersAndQueue() const {
    std::cout << "Pending orders:\n";
    orders_.displayByStatus(OrderStatus::PENDING, std::cout);
    pendingQueue_.display(std::cout);
}

void CanteenManager::displayOrderStatus(int token) const {
    const Order* order = orders_.find(token);
    if (order == nullptr) {
        std::cout << "No order exists with token " << token << ".\n";
        return;
    }
    std::cout << "Token: " << order->getToken()
              << "\nItems: " << order->getItems()
              << "\nDate: " << order->getDate()
              << "\nTime: " << order->getTime()
              << "\nStatus: " << statusToString(order->getStatus()) << '\n';
}

void CanteenManager::generateTodaysReport() const {
    std::string today;
    std::string ignoredTime;
    getCurrentDateTime(today, ignoredTime);

    int total = 0;
    int pending = 0;
    int preparing = 0;
    int completed = 0;
    int cancelled = 0;
    for (Order* order = orders_.getHead(); order != nullptr; order = order->getNext()) {
        if (order->getDate() != today) {
            continue;
        }
        ++total;
        switch (order->getStatus()) {
        case OrderStatus::PENDING:
            ++pending;
            break;
        case OrderStatus::PREPARING:
            ++preparing;
            break;
        case OrderStatus::COMPLETED:
            ++completed;
            break;
        case OrderStatus::CANCELLED:
            ++cancelled;
            break;
        }
    }

    std::cout << "Daily report for " << today << ":\n"
              << "Total orders: " << total << '\n'
              << "PENDING: " << pending << '\n'
              << "PREPARING: " << preparing << '\n'
              << "COMPLETED: " << completed << '\n'
              << "CANCELLED: " << cancelled << '\n';
}

void CanteenManager::getCurrentDateTime(std::string& date, std::string& time) {
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

bool CanteenManager::rebuildPendingQueue(std::ostream& messages) {
    try {
        for (Order* order = orders_.getHead(); order != nullptr; order = order->getNext()) {
            if (order->getStatus() == OrderStatus::PENDING) {
                pendingQueue_.enqueue(order->getToken());
            }
        }
    } catch (const std::bad_alloc&) {
        pendingQueue_.clear();
        messages << "Error: insufficient memory to rebuild the pending queue.\n";
        return false;
    }
    return true;
}
