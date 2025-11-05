#include "search.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// BST Node for price-based searching
typedef struct PriceNode {
    double key;
    int productId;
    struct PriceNode* left;
    struct PriceNode* right;
} PriceNode;

// BST Node for category-based searching
typedef struct CategoryNode {
    char category[MAX_CATEGORY_LEN];
    int* productIds;
    int count;
    int capacity;
    struct CategoryNode* left;
    struct CategoryNode* right;
} CategoryNode;

// Result structure for search output
typedef struct Result {
    int productId;
    double price;
    char name[MAX_NAME_LEN];
} Result;

// Price BST operations
static PriceNode* price_insert(PriceNode* root, double key, int productId) {
    if (!root) {
        PriceNode* node = (PriceNode*)malloc(sizeof(PriceNode));
        node->key = key;
        node->productId = productId;
        node->left = node->right = NULL;
        return node;
    }
    
    if (key < root->key || (key == root->key && productId < root->productId)) {
        root->left = price_insert(root->left, key, productId);
    } else {
        root->right = price_insert(root->right, key, productId);
    }
    
    return root;
}

static void price_inorder(PriceNode* root, int* arr, int* idx, int maxSize) {
    if (!root || *idx >= maxSize) return;
    
    price_inorder(root->left, arr, idx, maxSize);
    if (*idx < maxSize) {
        arr[(*idx)++] = root->productId;
    }
    price_inorder(root->right, arr, idx, maxSize);
}

static void free_price_tree(PriceNode* root) {
    if (!root) return;
    free_price_tree(root->left);
    free_price_tree(root->right);
    free(root);
}

// Category BST operations
static CategoryNode* category_search(CategoryNode* root, const char* category) {
    if (!root) return NULL;
    
    int cmp = strcmp(category, root->category);
    if (cmp == 0) return root;
    else if (cmp < 0) return category_search(root->left, category);
    else return category_search(root->right, category);
}

static CategoryNode* category_insert(CategoryNode* root, const char* category, int productId) {
    if (!root) {
        CategoryNode* node = (CategoryNode*)malloc(sizeof(CategoryNode));
        strncpy(node->category, category, MAX_CATEGORY_LEN - 1);
        node->category[MAX_CATEGORY_LEN - 1] = '\0';
        node->capacity = 10;
        node->productIds = (int*)malloc(sizeof(int) * node->capacity);
        node->productIds[0] = productId;
        node->count = 1;
        node->left = node->right = NULL;
        return node;
    }
    
    int cmp = strcmp(category, root->category);
    if (cmp == 0) {
        // Add product to existing category
        if (root->count >= root->capacity) {
            root->capacity *= 2;
            root->productIds = (int*)realloc(root->productIds, sizeof(int) * root->capacity);
        }
        root->productIds[root->count++] = productId;
    } else if (cmp < 0) {
        root->left = category_insert(root->left, category, productId);
    } else {
        root->right = category_insert(root->right, category, productId);
    }
    
    return root;
}

static void free_category_tree(CategoryNode* root) {
    if (!root) return;
    free_category_tree(root->left);
    free_category_tree(root->right);
    free(root->productIds);
    free(root);
}

// Comparison functions for sorting results
static int compare_by_price(const void* a, const void* b) {
    const Result* x = (const Result*)a;
    const Result* y = (const Result*)b;
    
    if (x->price < y->price) return -1;
    if (x->price > y->price) return 1;
    return 0;
}

static int compare_by_name(const void* a, const void* b) {
    const Result* x = (const Result*)a;
    const Result* y = (const Result*)b;
    
    return strcmp(x->name, y->name);
}

// Helper function to collect all products from inventory
static void collect_all_products(Inventory* inv, int* products, int* count) {
    *count = 0;
    
    // Scan all possible product IDs
    for (int id = 1; id < MAX_PRODUCTS && *count < MAX_PRODUCTS; id++) {
        Product* p = inventory_get_product(inv, id);
        if (p) {
            products[(*count)++] = id;
        }
    }
}

void search_build_and_execute(Inventory* inv, SuppliersDB* sdb, SearchCriteria criteria) {
    if (!inv) {
        printf("Invalid inventory!\n");
        return;
    }
    
    // Build BST structures for searching
    PriceNode* priceRoot = NULL;
    CategoryNode* categoryRoot = NULL;
    
    // Collect all products and build search structures
    int allProducts[MAX_PRODUCTS];
    int totalProducts = 0;
    collect_all_products(inv, allProducts, &totalProducts);
    
    // Build price BST and category BST
    for (int i = 0; i < totalProducts; i++) {
        Product* p = inventory_get_product(inv, allProducts[i]);
        if (!p) continue;
        
        // Add to price BST
        priceRoot = price_insert(priceRoot, p->price, p->id);
        
        // Add to category BST
        categoryRoot = category_insert(categoryRoot, p->category, p->id);
    }
    
    // Collect candidate products based on search criteria
    int candidates[MAX_PRODUCTS];
    int candidateCount = 0;
    
    if (criteria.hasCategory) {
        // Search by category using BST
        CategoryNode* categoryNode = category_search(categoryRoot, criteria.category);
        if (categoryNode) {
            for (int i = 0; i < categoryNode->count && candidateCount < MAX_PRODUCTS; i++) {
                candidates[candidateCount++] = categoryNode->productIds[i];
            }
        }
    } else {
        // Use all products (sorted by price from BST)
        price_inorder(priceRoot, candidates, &candidateCount, MAX_PRODUCTS);
    }
    
    // Filter candidates and create results
    Result results[MAX_PRODUCTS];
    int resultCount = 0;
    
    for (int i = 0; i < candidateCount; i++) {
        Product* p = inventory_get_product(inv, candidates[i]);
        if (!p) continue;
        
        // Apply filters
        if (criteria.onlyInStock && p->stock <= 0) continue;
        if (criteria.hasPriceMin && p->price < criteria.priceMin) continue;
        if (criteria.hasPriceMax && p->price > criteria.priceMax) continue;
        
        // Add to results
        if (resultCount < MAX_PRODUCTS) {
            results[resultCount].productId = p->id;
            results[resultCount].price = p->price;
            strncpy(results[resultCount].name, p->name, MAX_NAME_LEN - 1);
            results[resultCount].name[MAX_NAME_LEN - 1] = '\0';
            resultCount++;
        }
    }
    
    // Sort results based on criteria
    if (criteria.sortBy == 'p') {
        qsort(results, resultCount, sizeof(Result), compare_by_price);
    } else if (criteria.sortBy == 'n') {
        qsort(results, resultCount, sizeof(Result), compare_by_name);
    }
    
    // Display results
    printf("\n-- Search Results (%d items) --\n", resultCount);
    
    if (resultCount == 0) {
        printf("No products match your search criteria.\n");
    } else {
        printf("%-5s %-20s %-12s %-15s %-8s\n", "ID", "Name", "Price", "Category", "Stock");
        printf("-------------------------------------------------------------\n");
        
        for (int i = 0; i < resultCount; i++) {
            Product* p = inventory_get_product(inv, results[i].productId);
            if (p) {
                printf("%-5d %-20s $%-11.2f %-15s %-8d\n",
                       p->id, p->name, p->price, p->category, p->stock);
            }
        }
    }
    
    // Cleanup
    free_price_tree(priceRoot);
    free_category_tree(categoryRoot);
}