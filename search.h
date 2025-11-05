#ifndef SEARCH_H
#define SEARCH_H

#include <stdbool.h>
#include "common.h"
#include "inventory.h"
#include "suppliers.h"

typedef struct SearchCriteria {
	bool hasPriceMin;
	double priceMin;
	bool hasPriceMax;
	double priceMax;
	bool hasCategory;
	char category[MAX_CATEGORY_LEN];
	bool onlyInStock;
	char sortBy; // 'p' price, 'n' name
} SearchCriteria;

void search_build_and_execute(Inventory* inv, SuppliersDB* sdb, SearchCriteria c);

#endif // SEARCH_H