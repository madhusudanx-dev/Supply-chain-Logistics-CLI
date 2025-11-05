#ifndef SUPPLIERS_H
#define SUPPLIERS_H

#include <stdbool.h>
#include "common.h"

typedef struct SuppliersDB SuppliersDB;

SuppliersDB* suppliers_create(void);
void suppliers_destroy(SuppliersDB* db);

bool suppliers_insert(SuppliersDB* db, Supplier s);
bool suppliers_delete(SuppliersDB* db, int supplierId);
Supplier* suppliers_find_by_id(SuppliersDB* db, int supplierId);

// Display suppliers sorted by overall rating (descending)
void suppliers_print_ranked(SuppliersDB* db);

// Range query by minimum overall rating
void suppliers_print_min_rating(SuppliersDB* db, double minOverall);

#endif // SUPPLIERS_H


