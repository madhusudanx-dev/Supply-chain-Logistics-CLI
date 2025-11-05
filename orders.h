#ifndef ORDERS_H
#define ORDERS_H

#include <stdbool.h>
#include "common.h"
#include "inventory.h"

typedef struct OrdersQueue OrdersQueue;

OrdersQueue* orders_create(void);
void orders_destroy(OrdersQueue* q);

bool orders_enqueue(OrdersQueue* q, Order o);
bool orders_dequeue(OrdersQueue* q, Order* out);
int orders_count(OrdersQueue* q);
void orders_print(OrdersQueue* q);

// Process the next order in FIFO, validating against inventory and updating stock
bool orders_process_next(OrdersQueue* q, Inventory* inv);

#endif // ORDERS_H


