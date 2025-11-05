#include "suppliers.h"
#include <stdio.h>
#include <stdlib.h>

typedef struct AVLNode {
	Supplier supplier;
	double key; // overall score
	int height;
	struct AVLNode* left;
	struct AVLNode* right;
} AVLNode;

struct SuppliersDB {
	AVLNode* root;
};
static int height(AVLNode* n) { return n ? n->height : 0; }
#ifdef max
#undef max
#endif
static int max(int a, int b) { return a > b ? a : b; }

static AVLNode* rotate_right(AVLNode* y) {
	AVLNode* x = y->left; AVLNode* T2 = x->right;
	x->right = y; y->left = T2;
	y->height = max(height(y->left), height(y->right)) + 1;
	x->height = max(height(x->left), height(x->right)) + 1;
	return x;
}

static AVLNode* rotate_left(AVLNode* x) {
	AVLNode* y = x->right; AVLNode* T2 = y->left;
	y->left = x; x->right = T2;
	x->height = max(height(x->left), height(x->right)) + 1;
	y->height = max(height(y->left), height(y->right)) + 1;
	return y;
}

static int get_balance(AVLNode* n) { return n ? height(n->left) - height(n->right) : 0; }

static int cmp(double a, double b) {
	if (a < b) return -1; if (a > b) return 1; return 0;
}

static AVLNode* avl_insert(AVLNode* node, Supplier s) {
	double key = supplier_overall_score(&s.ratings);
	if (!node) {
		AVLNode* n = (AVLNode*)malloc(sizeof(AVLNode));
		n->supplier = s; n->key = key; n->height = 1; n->left = n->right = NULL; return n;
	}
	int c = cmp(key, node->key);
	if (c < 0 || (c == 0 && s.id < node->supplier.id)) node->left = avl_insert(node->left, s);
	else node->right = avl_insert(node->right, s);

	node->height = 1 + max(height(node->left), height(node->right));
	int balance = get_balance(node);
	if (balance > 1 && cmp(key, node->left->key) < 0) return rotate_right(node);
	if (balance < -1 && cmp(key, node->right->key) > 0) return rotate_left(node);
	if (balance > 1 && cmp(key, node->left->key) > 0) { node->left = rotate_left(node->left); return rotate_right(node);}
	if (balance < -1 && cmp(key, node->right->key) < 0) { node->right = rotate_right(node->right); return rotate_left(node);}
	return node;
}

static AVLNode* min_value_node(AVLNode* node) { AVLNode* cur = node; while (cur && cur->left) cur = cur->left; return cur; }

static AVLNode* avl_delete(AVLNode* root, double key, int id) {
	if (!root) return NULL;
	int c = cmp(key, root->key);
	if (c < 0 || (c == 0 && id < root->supplier.id)) root->left = avl_delete(root->left, key, id);
	else if (c > 0 || (c == 0 && id > root->supplier.id)) root->right = avl_delete(root->right, key, id);
	else {
		if (!root->left || !root->right) {
			AVLNode* tmp = root->left ? root->left : root->right;
			if (!tmp) { free(root); return NULL; } else { *root = *tmp; free(tmp); }
		} else {
			AVLNode* tmp = min_value_node(root->right);
			root->supplier = tmp->supplier; root->key = tmp->key;
			root->right = avl_delete(root->right, tmp->key, tmp->supplier.id);
		}
	}
	root->height = 1 + max(height(root->left), height(root->right));
	int balance = get_balance(root);
	if (balance > 1 && get_balance(root->left) >= 0) return rotate_right(root);
	if (balance > 1 && get_balance(root->left) < 0) { root->left = rotate_left(root->left); return rotate_right(root);}
	if (balance < -1 && get_balance(root->right) <= 0) return rotate_left(root);
	if (balance < -1 && get_balance(root->right) > 0) { root->right = rotate_right(root->right); return rotate_left(root);}
	return root;
}

static void avl_free(AVLNode* n) { if (!n) return; avl_free(n->left); avl_free(n->right); free(n); }

SuppliersDB* suppliers_create(void) { return (SuppliersDB*)calloc(1, sizeof(SuppliersDB)); }

void suppliers_destroy(SuppliersDB* db) { if (!db) return; avl_free(db->root); free(db); }

bool suppliers_insert(SuppliersDB* db, Supplier s) { if (!db) return false; db->root = avl_insert(db->root, s); return true; }

static void inorder_desc(AVLNode* n) {
	if (!n) return;
	inorder_desc(n->right);
	printf("ID:%d Name:%s Score:%.2f (Q:%.1f D:%.1f P:%.1f R:%.1f S:%.1f)\n",
		n->supplier.id, n->supplier.name, n->key,
		n->supplier.ratings.quality, n->supplier.ratings.deliveryTime,
		n->supplier.ratings.price, n->supplier.ratings.reliability,
		n->supplier.ratings.customerService);
	inorder_desc(n->left);
}

void suppliers_print_ranked(SuppliersDB* db) { printf("\n-- Suppliers Ranked (High to Low) --\n"); inorder_desc(db->root); }

static void inorder_min(AVLNode* n, double minScore) {
	if (!n) return;
	if (n->key >= minScore) inorder_min(n->left, minScore);
	if (n->key >= minScore) {
		printf("ID:%d Name:%s Score:%.2f\n", n->supplier.id, n->supplier.name, n->key);
	}
	inorder_min(n->right, minScore);
}

void suppliers_print_min_rating(SuppliersDB* db, double minOverall) {
	printf("\n-- Suppliers with overall >= %.2f --\n", minOverall);
	inorder_min(db->root, minOverall);
}

static Supplier* find_by_id_rec(AVLNode* n, int id) {
	if (!n) return NULL;
	if (n->supplier.id == id) return &n->supplier;
	Supplier* left = find_by_id_rec(n->left, id); if (left) return left;
	return find_by_id_rec(n->right, id);
}

Supplier* suppliers_find_by_id(SuppliersDB* db, int supplierId) { return find_by_id_rec(db->root, supplierId); }

bool suppliers_delete(SuppliersDB* db, int supplierId) {
	// Need the key; find node first
	Supplier* s = suppliers_find_by_id(db, supplierId);
	if (!s) return false;
	double key = supplier_overall_score(&s->ratings);
	db->root = avl_delete(db->root, key, supplierId);
	return true;
}
