#include <iostream>
#include <fstream>
#include <string>
#include <ctime>
#include <cstdlib>
#include <sstream>

using namespace std;

enum OrderStatus { PENDING, PREPARING, COMPLETED, CANCELLED };

string statusToString(OrderStatus status) {
    switch (status) {
        case PENDING: return "PENDING";
        case PREPARING: return "PREPARING";
        case COMPLETED: return "COMPLETED";
        case CANCELLED: return "CANCELLED";
    }
    return "UNKNOWN";
}

OrderStatus stringToStatus(string status) {
    if (status == "PREPARING") return PREPARING;
    if (status == "COMPLETED") return COMPLETED;
    if (status == "CANCELLED") return CANCELLED;
    return PENDING;
}

bool validTransition(OrderStatus current, OrderStatus next) {
    if (current == PENDING && (next == PREPARING || next == CANCELLED)) return true;
    if (current == PREPARING && next == COMPLETED) return true;
    return false;
}

void getDateTime(string &date, string &timeText) {
    time_t now = time(0);
    tm *local = localtime(&now);
    char dateBuffer[20], timeBuffer[20];
    strftime(dateBuffer, sizeof(dateBuffer), "%Y-%m-%d", local);
    strftime(timeBuffer, sizeof(timeBuffer), "%H:%M:%S", local);
    date = dateBuffer;
    timeText = timeBuffer;
}

class Order {
public:
    int token;
    string items, date, timeText;
    OrderStatus status;
    Order *next;

    Order(int t, string i, string d, string tm, OrderStatus s) {
        token = t;
        items = i;
        date = d;
        timeText = tm;
        status = s;
        next = NULL;
    }
};

class OrderList {
public:
    Order *head, *tail;

    OrderList() {
        head = tail = NULL;
    }

    ~OrderList() {
        Order *current = head;
        while (current != NULL) {
            Order *temp = current;
            current = current->next;
            delete temp;
        }
    }

    void insert(Order *order) {
        if (head == NULL) head = tail = order;
        else {
            tail->next = order;
            tail = order;
        }
    }

    Order* search(int token) {
        Order *current = head;
        while (current != NULL) {
            if (current->token == token) return current;
            current = current->next;
        }
        return NULL;
    }
    
    void displayAll() {
        if (head == NULL) {
            cout << "No orders found.\n";
            return;
        }
        Order *current = head;
        while (current != NULL) {
            cout << "Token: " << current->token
                 << " | Items: " << current->items
                 << " | Date: " << current->date
                 << " | Time: " << current->timeText
                 << " | Status: " << statusToString(current->status) << "\n";
            current = current->next;
        }
    }

    void displayPending() {
        bool found = false;
        Order *current = head;
        while (current != NULL) {
            if (current->status == PENDING) {
                cout << "Token: " << current->token << " | Items: " << current->items << "\n";
                found = true;
            }
            current = current->next;
        }
        if (!found) cout << "No pending orders.\n";
    }
};

class OrderQueue {
private:
    class Node {
    public:
        int token;
        Node *next;
        Node(int t) { token = t; next = NULL; }
    };

    Node *frontNode, *rearNode;

public:
    OrderQueue() { frontNode = rearNode = NULL; }

    ~OrderQueue() { clear(); }

    bool isEmpty() { return frontNode == NULL; }

    void enqueue(int token) {
        Node *newNode = new Node(token);
        if (rearNode == NULL) frontNode = rearNode = newNode;
        else {
            rearNode->next = newNode;
            rearNode = newNode;
        }
    }

    bool dequeue(int &token) {
        if (isEmpty()) return false;
        Node *temp = frontNode;
        token = temp->token;
        frontNode = frontNode->next;
        if (frontNode == NULL) rearNode = NULL;
        delete temp;
        return true;
    }

    bool getFront(int &token) {
        if (isEmpty()) return false;
        token = frontNode->token;
        return true;
    }

    bool remove(int token) {
        Node *current = frontNode, *previous = NULL;
        while (current != NULL && current->token != token) {
            previous = current;
            current = current->next;
        }
        if (current == NULL) return false;

        if (previous == NULL) frontNode = current->next;
        else previous->next = current->next;

        if (current == rearNode) rearNode = previous;
        delete current;
        return true;
    }

    void display() {
        Node *current = frontNode;
        if (current == NULL) {
            cout << "Queue is empty.\n";
            return;
        }
        while (current != NULL) {
            cout << current->token;
            if (current->next != NULL) cout << " -> ";
            current = current->next;
        }
        cout << "\n";
    }

    void clear() {
        int token;
        while (dequeue(token)) { }
    }
};

class CanteenManager {
private:
    OrderList orders;
    OrderQueue pendingQueue;
    int nextToken;
    string fileName;

public:
    CanteenManager() {
        nextToken = 1;
        fileName = "orders.txt";
    }

    void loadFromFile() {
        ifstream file(fileName.c_str());
        if (!file) return;

        string line;
        int largestToken = 0;
        while (getline(file, line)) {
            stringstream ss(line);
            string tokenText, date, timeText, statusText, items;
            getline(ss, tokenText, '|');
            getline(ss, date, '|');
            getline(ss, timeText, '|');
            getline(ss, statusText, '|');
            getline(ss, items);

            if (tokenText == "" || items == "") continue;
            int token = atoi(tokenText.c_str());
            if (token <= 0 || orders.search(token) != NULL) continue;

            OrderStatus status = stringToStatus(statusText);
            Order *order = new Order(token, items, date, timeText, status);
            orders.insert(order);
            if (token > largestToken) largestToken = token;
        }
        nextToken = largestToken + 1;

        Order *current = orders.head;
        while (current != NULL) {
            if (current->status == PENDING) pendingQueue.enqueue(current->token);
            current = current->next;
        }
    }

    void saveToFile() {
        ofstream file(fileName.c_str());
        Order *current = orders.head;
        while (current != NULL) {
            file << current->token << "|" << current->date << "|"
                 << current->timeText << "|" << statusToString(current->status)
                 << "|" << current->items << "\n";
            current = current->next;
        }
        cout << "Orders saved to orders.txt.\n";
    }

    void placeOrder(string items) {
        if (items.find_first_not_of(" \t\n") == string::npos) {
            cout << "Food item cannot be empty.\n";
            return;
        }
        string date, timeText;
        getDateTime(date, timeText);
        Order *order = new Order(nextToken, items, date, timeText, PENDING);
        orders.insert(order);
        pendingQueue.enqueue(nextToken);
        cout << "Order placed successfully. Token: " << nextToken << "\n";
        nextToken++;
        saveToFile();
    }

    void processNextOrder() {
        int token;
        while (pendingQueue.getFront(token)) {
            Order *order = orders.search(token);
            if (order == NULL || order->status != PENDING) {
                pendingQueue.dequeue(token);
                continue;
            }
            order->status = PREPARING;
            pendingQueue.dequeue(token);
            cout << "Order " << token << " is now PREPARING.\n";
            saveToFile();
            return;
        }
        cout << "No pending orders to process.\n";
    }

    void completeOrder(int token) {
        Order *order = orders.search(token);
        if (order == NULL) cout << "Order not found.\n";
        else if (!validTransition(order->status, COMPLETED))
            cout << "Only PREPARING orders can be completed.\n";
        else {
            order->status = COMPLETED;
            cout << "Order completed.\n";
            saveToFile();
        }
    }

    void cancelOrder(int token) {
        Order *order = orders.search(token);
        if (order == NULL) cout << "Order not found.\n";
        else if (order->status != PENDING)
            cout << "Only PENDING orders can be cancelled.\n";
        else if (!pendingQueue.remove(token))
            cout << "Order was not found in pending queue.\n";
        else {
            if (validTransition(order->status, CANCELLED)) {
                order->status = CANCELLED;
                cout << "Order cancelled.\n";
                saveToFile();
            }
        }
    }

    void displayAllOrders() { orders.displayAll(); }

    void displayPendingOrders() {
        cout << "Pending orders:\n";
        orders.displayPending();
        pendingQueue.display();
    }

    void checkStatus(int token) {
        Order *order = orders.search(token);
        if (order == NULL) cout << "Order not found.\n";
        else cout << "Token: " << order->token << "\nItems: " << order->items
                  << "\nStatus: " << statusToString(order->status) << "\n";
    }

    void dailyReport() {
        string today, timeText;
        getDateTime(today, timeText);
        int total = 0, pending = 0, preparing = 0, completed = 0, cancelled = 0;
        Order *current = orders.head;
        while (current != NULL) {
            if (current->date == today) {
                total++;
                if (current->status == PENDING) pending++;
                else if (current->status == PREPARING) preparing++;
                else if (current->status == COMPLETED) completed++;
                else if (current->status == CANCELLED) cancelled++;
            }
            current = current->next;
        }
        cout << "Daily report for " << today << "\n"
             << "Total orders: " << total << "\n"
             << "PENDING: " << pending << "\n"
             << "PREPARING: " << preparing << "\n"
             << "COMPLETED: " << completed << "\n"
             << "CANCELLED: " << cancelled << "\n";
    }
};

int main() {
    CanteenManager manager;
    manager.loadFromFile();

    int choice, token;
    string items;

    do {
        cout << "\n===== CANTEEN ORDER MANAGEMENT =====\n"
             << "1. Place New Order\n"
             << "2. Display All Orders\n"
             << "3. Display Pending Orders and Queue\n"
             << "4. Process Next Order\n"
             << "5. Mark Order as Completed\n"
             << "6. Check Order Status\n"
             << "7. Cancel Pending Order\n"
             << "8. Generate Daily Report\n"
             << "9. Save Orders\n"
             << "10. Exit\n"
             << "Enter choice: ";
        cin >> choice;
        cin.ignore(10000, '\n');

        switch (choice) {
            case 1:
                cout << "Enter food items: ";
                getline(cin, items);
                manager.placeOrder(items);
                break;
            case 2:
                manager.displayAllOrders();
                break;
            case 3:
                manager.displayPendingOrders();
                break;
            case 4:
                manager.processNextOrder();
                break;
            case 5:
                cout << "Enter token: "; cin >> token; cin.ignore(10000, '\n');
                manager.completeOrder(token);
                break;
            case 6:
                cout << "Enter token: "; cin >> token; cin.ignore(10000, '\n');
                manager.checkStatus(token);
                break;
            case 7:
                cout << "Enter token: "; cin >> token; cin.ignore(10000, '\n');
                manager.cancelOrder(token);
                break;
            case 8:
                manager.dailyReport();
                break;
            case 9:
                manager.saveToFile();
                break;
            case 10:
                manager.saveToFile();
                cout << "Goodbye!\n";
                break;
            default:
                cout << "Invalid choice. Please enter 1 to 10.\n";
        }
    } while (choice != 10);

    return 0;
}