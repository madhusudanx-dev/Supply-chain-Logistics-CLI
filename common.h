#ifndef COMMON_H
#define COMMON_H

#include <stddef.h>
#include <stdbool.h>

#define MAX_NAME_LEN 64
#define MAX_CATEGORY_LEN 32
#define MAX_SUPPLIER_NAME 64
#define MAX_ORDER_ITEMS 16
#define MAX_PRODUCTS 1024
#define MAX_SUPPLIERS 512

// ANSI color codes for CLI styling
#define COL_RESET "\x1b[0m"
#define COL_BOLD  "\x1b[1m"
#define COL_DIM   "\x1b[2m"
#define COL_RED   "\x1b[31m"
#define COL_GREEN "\x1b[32m"
#define COL_YELLOW "\x1b[33m"
#define COL_BLUE  "\x1b[34m"
#define COL_MAGENTA "\x1b[35m"
#define COL_CYAN  "\x1b[36m"
#define COL_WHITE "\x1b[37m"

typedef struct Product {
	int id;
	char name[MAX_NAME_LEN];
	char category[MAX_CATEGORY_LEN];
	int supplierId;
	double price;
	int stock;
} Product;

typedef struct OrderItem {
	int productId;
	int quantity;
} OrderItem;

typedef struct Order {
	int id;
	char customer[MAX_NAME_LEN];
	int numItems;
	OrderItem items[MAX_ORDER_ITEMS];
} Order;

typedef struct SupplierRatings {
	// 0..10 scale per criterion
	double quality;
	double deliveryTime;
	double price;
	double reliability;
	double customerService;
} SupplierRatings;

typedef struct Supplier {
	int id;
	char name[MAX_SUPPLIER_NAME];
	SupplierRatings ratings;
} Supplier;

// Utility
double supplier_overall_score(const SupplierRatings *r);
void trim_newline(char *s);
int safe_read_int();
double safe_read_double();

#endif // COMMON_H


