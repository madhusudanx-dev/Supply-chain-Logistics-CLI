#ifndef INVENTORY_H
#define INVENTORY_H

#include <stdbool.h>
#include "common.h"

// Inventory hash table API (chaining)
typedef struct Inventory Inventory;

Inventory* inventory_create(void);
void inventory_destroy(Inventory* inv);

bool inventory_add_product(Inventory* inv, Product p);
bool inventory_remove_product(Inventory* inv, int productId);
Product* inventory_get_product(Inventory* inv, int productId);
bool inventory_update_stock(Inventory* inv, int productId, int newStock);
void inventory_print_all(Inventory* inv);

// Low-stock min-heap API
// Push updated product into heap (called internally on stock changes)
void inventory_heap_refresh_all(Inventory* inv);
// Pop products under threshold; returns number printed
int inventory_pop_low_stock_alerts(Inventory* inv, int threshold, int maxCount);

// Undo stack (simple)
bool inventory_undo_last(Inventory* inv);

#endif // INVENTORY_H


