# CanteenFlow-DSA

A menu-driven C++17 console application for managing canteen orders, preparation tokens, cancellations, and daily order reports. This academic project demonstrates manually implemented linked data structures and sequential file handling.

## Problem statement

A college canteen needs a simple way to identify orders, prepare them in arrival order, prevent invalid state changes, retain order history, and produce a daily summary. CanteenFlow stores each order once in a master singly linked list and keeps only token references in a separate FIFO linked queue.

## Objectives

- Generate unique, increasing integer order tokens and retain the token counter between runs.
- Preserve order placement sequence and process pending orders in FIFO order.
- Enforce the allowed `PENDING`, `PREPARING`, `COMPLETED`, and `CANCELLED` transitions.
- Keep completed and cancelled orders available for history and reporting.
- Demonstrate manual linked-list and linked-queue operations, linear search, and text-file persistence.

## Features

- Place orders with one or more items in a free-form text description.
- Automatically record local date and time.
- Display all orders, pending orders, and the pending token queue.
- Process the oldest pending order, complete a preparing order, or cancel a pending order.
- Search an order by token and display its details.
- Generate today's total and per-status counts.
- Load order records on startup and save after successful changes and on exit.
- Validate malformed menu input and malformed/duplicate file records without crashing.

## Data structures and algorithms

### Master singly linked list

`OrderList` owns the master order records. Each `Order` is one node and contains its token, items, date, time, status, and `next` pointer. New records are appended in placement order. Search and status filtering traverse the list linearly.

### Pending linked queue

`OrderQueue` stores integer token references, not copies of order records. It maintains front and rear pointers, so enqueue and dequeue are O(1). Cancellation removes a selected token while correctly maintaining both pointers. On startup, the queue is rebuilt by traversing the saved master records in their persisted placement order.

### Status validation

The only permitted transitions are:

- `PENDING` to `PREPARING`
- `PENDING` to `CANCELLED`
- `PREPARING` to `COMPLETED`

The linked list validates transitions before changing the authoritative order record. A token is dequeued only when it is successfully moved to `PREPARING`. Only a `PENDING` order can be cancelled, and its queue token must be removed before its status changes.

### File format and token persistence

`data/orders.txt` is a line-oriented text file. The first line identifies the format version, the second stores the next token number, and each following `ORDER` line stores one full order. Quoted strings allow spaces and punctuation in item descriptions. Records are written in placement order; that order—not numeric sorting—is used to rebuild the pending queue. Duplicate tokens and malformed records are reported and skipped. The runtime data file is ignored by Git; `data/.gitkeep` preserves the empty folder in the repository.

## Source files

- `src/main.cpp` — menu display, input validation, and switch-based user interaction.
- `src/Order.h` — declarations for the order node, order status values, and status helper functions.
- `src/Order.cpp` — definitions for order methods and status helper functions.
- `src/OrderList.h` / `src/OrderList.cpp` — declarations and implementation of the manually managed master singly linked list, linear search, display, and status updates.
- `src/OrderQueue.h` / `src/OrderQueue.cpp` — declarations and implementation of the manually managed FIFO linked queue of pending token references.
- `src/CanteenManager.h` / `src/CanteenManager.cpp` — declarations and implementation of the business rules, persistence, queue reconstruction, timestamps, and daily reports.
- `data/.gitkeep` — keeps the runtime-data directory in the repository.

### What are `.h` and `.cpp` files?

A `.h` file is a header. It tells other source files which classes and functions exist (their declarations), so those files can use them. A `.cpp` file contains the function definitions and actual implementation. Headers do not contain or include all the `.cpp` files; instead, include the needed header with `#include "Order.h"` and compile all the project's `.cpp` files together.

## Requirements

- A C++17 compiler such as g++ (MinGW-w64 on Windows).
- Visual Studio Code or another text editor (optional).
- No external libraries or frameworks.

## Compile and run on Windows

Open PowerShell or the VS Code terminal in the project root:

```powershell
g++ -std=c++17 -Wall -Wextra -pedantic src\main.cpp src\Order.cpp src\OrderList.cpp src\OrderQueue.cpp src\CanteenManager.cpp -o CanteenFlow.exe
.\CanteenFlow.exe
```

The program uses `data/orders.txt` relative to the current working directory. Run the executable from the project root so the expected data path is used. The `data` directory is included in the repository.

## Menu

```text
========== CANTEEN ORDER MANAGEMENT SYSTEM ==========
1. Place New Order
2. Display All Orders
3. Display Pending Orders and Queue
4. Process Next Order
5. Mark Order as Completed
6. Check Order Status by Token
7. Cancel Pending Order
8. Generate Today's Daily Report
9. Save Orders to File
10. Exit
Enter your choice:
```

## Sample session

```text
Enter your choice: 1
Enter one or more food items: 2 Vada Pav, 1 Tea
Order placed successfully. Your token is 1.

Enter your choice: 1
Enter one or more food items: Masala Dosa
Order placed successfully. Your token is 2.

Enter your choice: 3
Pending orders:
Token: 1 | Items: 2 Vada Pav, 1 Tea | Date: 2026-10-19 | Time: 12:30:00 | Status: PENDING
Token: 2 | Items: Masala Dosa | Date: 2026-10-19 | Time: 12:30:05 | Status: PENDING
Pending queue (front to rear): 1 -> 2

Enter your choice: 4
Order 1 is now PREPARING.
```

Dates and times in actual output reflect the computer's local clock.

## Time complexity

Let `n` be the number of saved orders and `q` the number of pending queue entries.

| Operation | Complexity |
|---|---:|
| Append a master order | O(1) |
| Enqueue or dequeue a pending token | O(1) |
| Read queue front / check empty | O(1) |
| Remove a specific token from queue | O(q) |
| Search order by token | O(n) |
| Display all orders / filter by status | O(n) |
| Rebuild queue at startup | O(n) |
| Generate today's report | O(n) |
| Save or load the order file | O(n) |

## Test cases

The intended/manual validation scenarios and expected results are:

1. Place three orders: receive distinct increasing tokens and see them in arrival order.
2. Process orders: each successful processing selects the oldest pending token.
3. Complete a `PREPARING` order: status becomes `COMPLETED`.
4. Cancel a `PENDING` order: its token leaves the queue and its retained record becomes `CANCELLED`.
5. Cancel a `COMPLETED` order: rejected, with no status or queue change.
6. Complete a `CANCELLED` order: rejected, with no status change.
7. Process an empty queue: a clear no-pending-orders message is shown.
8. Search a nonexistent token: a clear not-found message is shown.
9. Exit and restart: valid records and next token are loaded from `data/orders.txt`.
10. Restart with multiple pending records: queue order matches record placement order.
11. Cancel an order, restart, then place another: the cancelled token is not reused.
12. Generate today's report: total equals the sum of the four status counts.
13. Enter nonnumeric or out-of-range menu input: it is rejected and the menu continues.
14. Display all orders after transitions: every token has exactly one current status.

These scenarios describe expected behavior; run the commands above and exercise the menu to verify them on your machine.

## Limitations and future improvements

- The application is single-user and console-based; it does not provide authentication or concurrent access.
- The daily report uses the computer's local date and time.
- Saving rewrites the text file; it reports failures, but the format is not a transactional database.
- There is no itemized price, inventory, or payment tracking.
- Hash tables, sorting algorithms, and priority queues are not implemented.
- Possible extensions include unit tests, prices and billing, input date validation, backup/atomic file replacement, and optional searching by item.

## Viva notes

- **Why a linked list?** It retains every order as a master record and allows traversal-based search without relying on an STL container for the core operation. Each node owns its `next` link, and the list destructor deletes all nodes.
- **Why a queue?** Orders awaiting preparation follow first-in, first-out order. The queue nodes contain tokens only, which prevents duplicated order state. Front and rear pointers make enqueue/dequeue constant time.
- **How are changes safe?** Status transitions are checked against the current status. Cancellation first verifies `PENDING` and removes the queue token before marking the record cancelled. Processing verifies the front token's master record before promoting it.
- **How does restart preserve FIFO and token uniqueness?** Master records are saved in insertion order, while a separate next-token counter is persisted. Startup traverses the file-ordered records and enqueues only pending tokens.
