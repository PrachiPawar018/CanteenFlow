#include "CanteenManager.h"

#include <iostream>
#include <sstream>
#include <string>

namespace {

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

} // namespace

int main() {
    CanteenManager manager;
    if (!manager.loadFromFile()) {
        std::cout << "Order history could not be fully loaded. Check the messages above.\n";
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
