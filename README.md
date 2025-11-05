🚚 Supply Chain Management System using Data Structures and Algorithms
👥 Team Name: 6Seven

Team Members:

Madhusudan (PES1UG24AM375)

Dikshith (PES1UG24AM363)

Samarth (PES1UG24AM396)

Pranaav (PES1UG24AM387)

📘 Project Overview

The Supply Chain Management System (SCMS) is a Command-Line Interface (CLI) based project built in C that efficiently manages suppliers, inventory, and orders using fundamental Data Structures and Algorithms (DSA).

This project simulates a mini supply chain workflow, ensuring optimized performance, easy data lookup, and organized tracking of operations.

⚙️ Key Features

🏭 Supplier Management

Add, remove, and search suppliers using AVL Trees for balanced and efficient lookups.

📦 Inventory Management

Maintain stock information using Linked Lists and Stacks for efficient storage and retrieval.

📋 Order Management

Process customer orders, handle delivery queues, and prioritize using Queues and Heaps.

🔍 Search & Analytics

Fast searching through supplier and inventory data using Binary Search Trees (BSTs) and Hashing alternatives.

⚡ Performance-Oriented Architecture

Implemented various DSA concepts like dynamic memory allocation, trees, queues, and graphs to simulate real-world logistics.

🧩 Data Structures Used
Data Structure	Purpose
Linked List	Manage inventory and item lists
Stack	Track undo/redo operations or transaction history
Queue (Circular/Priority)	Manage order and delivery queues
AVL Tree / BST	Supplier and product indexing
Heap	Handle priority orders and cost optimizations
Graph	Model supply routes and warehouse connectivity
🧠 Algorithms Implemented

Insertion, deletion, and traversal algorithms for Trees

Sorting and searching techniques (Binary Search, Quick Sort)

Shortest path algorithms for graph-based route simulation

Efficient heapify operations for order prioritization

🧑‍💻 System Workflow
+----------------------------+
|        Main Menu           |
+----------------------------+
        |       |       |
   +----+   +---+---+   +----+
   |Suppliers|Inventory|Orders|
   +----+---+---+---+---+----+
        |       |       |
  AVL Trees  Linked Lists  Queues


Each module interacts seamlessly through a modular C architecture to ensure clarity, scalability, and maintainability.

🏗️ Project Structure
📁 Supply-chain-Logistics-CLI/
├── main.c              # CLI Controller
├── suppliers.c/.h      # Supplier Module (AVL Tree)
├── inventory.c/.h      # Inventory Module (Linked List)
├── orders.c/.h         # Orders & Queue Management
├── search.c/.h         # Searching and Filtering Functions
├── common.c/.h         # Shared Utilities
├── Makefile            # Build Automation
└── README.md           # Project Documentation

🧾 How to Run
🔹 Using GCC (Manual Compilation)
gcc -o output common.c inventory.c main.c orders.c search.c suppliers.c
./output

🔹 Using Makefile (Recommended)
make
./output


To clean build files:

make clean

🔗 GitHub Repository

https://github.com/madhusudanx-dev/Supply-chain-Logistics-CLI.git

📊 Future Enhancements

Add file-based persistent storage for supplier and inventory data

Include an interactive graphical interface using C graphics or web integration

Expand to multi-threaded operations for real-time order tracking

Integrate basic encryption for secure order management

🏁 Conclusion

This project demonstrates how Data Structures and Algorithms can be applied to build an efficient and scalable Supply Chain Management System. It showcases teamwork, logical thinking, and a strong understanding of C programming and DSA fundamentals.
