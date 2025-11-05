#include "orders.h"
#include <stdio.h>
#include <stdlib.h>

typedef struct OrderNode {
	Order order;
	struct OrderNode* next;
} OrderNode;

struct OrdersQueue {
	OrderNode* head;
	OrderNode* tail;
	int count;
};

OrdersQueue* orders_create(void) {
	return (OrdersQueue*)calloc(1, sizeof(OrdersQueue));
}

void orders_destroy(OrdersQueue* q) {
	if (!q) return;
	OrderNode* cur = q->head;
	while (cur) { OrderNode* n = cur->next; free(cur); cur = n; }
	free(q);
}

bool orders_enqueue(OrdersQueue* q, Order o) {
	if (!q) return false;
	OrderNode* n = (OrderNode*)malloc(sizeof(OrderNode));
	n->order = o; n->next = NULL;
	if (!q->tail) q->head = q->tail = n; else { q->tail->next = n; q->tail = n; }
	q->count++;
	return true;
}

bool orders_dequeue(OrdersQueue* q, Order* out) {
	if (!q || !q->head) return false;
	OrderNode* n = q->head; q->head = n->next; if (!q->head) q->tail = NULL;
	if (out) *out = n->order; free(n); q->count--; return true;
}

int orders_count(OrdersQueue* q) { return q ? q->count : 0; }

void orders_print(OrdersQueue* q) {
	printf("\n-- Orders Queue (count=%d) --\n", orders_count(q));
	for (OrderNode* cur = q->head; cur; cur = cur->next) {
		printf("Order #%d for %s (items=%d)\n", cur->order.id, cur->order.customer, cur->order.numItems);
	}
}

bool orders_process_next(OrdersQueue* q, Inventory* inv) {
	if (!q || !q->head) { printf("No orders to process.\n"); return false; }
	Order o; orders_dequeue(q, &o);
	// Validate inventory
	for (int i = 0; i < o.numItems; ++i) {
		OrderItem it = o.items[i];
		Product* p = inventory_get_product(inv, it.productId);
		if (!p) { printf("Order %d FAILED: product %d not found.\n", o.id, it.productId); return false; }
		if (p->stock < it.quantity) { printf("Order %d FAILED: insufficient stock for product %d.\n", o.id, it.productId); return false; }
	}
	// Deduct stock
	for (int i = 0; i < o.numItems; ++i) {
		OrderItem it = o.items[i];
		Product* p = inventory_get_product(inv, it.productId);
		inventory_update_stock(inv, p->id, p->stock - it.quantity);
	}
	printf("Order %d processed successfully for %s.\n", o.id, o.customer);
	return true;
}


