#include "inventory.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_SKIP_LEVEL 16

// Skip List Node for inventory
typedef struct SkipNode {
    Product product;
    struct SkipNode** forward;  // Array of forward pointers
    int level;
} SkipNode;

// Skip List structure
typedef struct SkipList {
    SkipNode* header;
    int currentLevel;
    int size;
} SkipList;

typedef struct HeapEntry {
    int stock;
    int productId;
    unsigned long version;
} HeapEntry;

typedef enum { ACT_ADD, ACT_REMOVE, ACT_UPDATE_STOCK } ActionType;

typedef struct UndoAction {
    ActionType type;
    Product before;
    Product after;
} UndoAction;

typedef struct UndoNode {
    UndoAction action;
    struct UndoNode* next;
} UndoNode;

struct Inventory {
    SkipList* products;
    // Min-heap by stock
    HeapEntry* heap;
    int heapSize;
    int heapCapacity;
    // Versioning for products
    unsigned long productVersion[MAX_PRODUCTS];
    // Undo stack
    UndoNode* undoTop;
};

// Skip List utility functions
static int random_level() {
    int level = 1;
    while (rand() % 2 && level < MAX_SKIP_LEVEL) {
        level++;
    }
    return level;
}

static SkipNode* create_skip_node(int level, Product product) {
    SkipNode* node = (SkipNode*)malloc(sizeof(SkipNode));
    node->product = product;
    node->level = level;
    node->forward = (SkipNode**)malloc(sizeof(SkipNode*) * (level + 1));
    for (int i = 0; i <= level; i++) {
        node->forward[i] = NULL;
    }
    return node;
}

static SkipList* create_skip_list() {
    SkipList* list = (SkipList*)malloc(sizeof(SkipList));
    list->currentLevel = 0;
    list->size = 0;
    
    // Create header node with empty product
    Product emptyProduct = {0};
    list->header = create_skip_node(MAX_SKIP_LEVEL, emptyProduct);
    
    return list;
}

// Skip list search
static SkipNode* skip_list_search(SkipList* list, int productId) {
    SkipNode* current = list->header;
    
    // Start from highest level
    for (int i = list->currentLevel; i >= 0; i--) {
        while (current->forward[i] && current->forward[i]->product.id < productId) {
            current = current->forward[i];
        }
    }
    
    current = current->forward[0];
    
    if (current && current->product.id == productId) {
        return current;
    }
    
    return NULL;
}

// Skip list insert
static bool skip_list_insert(SkipList* list, Product product) {
    SkipNode* update[MAX_SKIP_LEVEL + 1];
    SkipNode* current = list->header;
    
    // Find position to insert
    for (int i = list->currentLevel; i >= 0; i--) {
        while (current->forward[i] && current->forward[i]->product.id < product.id) {
            current = current->forward[i];
        }
        update[i] = current;
    }
    
    current = current->forward[0];
    
    // If product already exists, update it
    if (current && current->product.id == product.id) {
        current->product = product;
        return true;
    }
    
    // Generate random level for new node
    int newLevel = random_level();
    
    if (newLevel > list->currentLevel) {
        for (int i = list->currentLevel + 1; i <= newLevel; i++) {
            update[i] = list->header;
        }
        list->currentLevel = newLevel;
    }
    
    // Create new node
    SkipNode* newNode = create_skip_node(newLevel, product);
    
    // Update forward pointers
    for (int i = 0; i <= newLevel; i++) {
        newNode->forward[i] = update[i]->forward[i];
        update[i]->forward[i] = newNode;
    }
    
    list->size++;
    return true;
}

// Skip list delete
static bool skip_list_delete(SkipList* list, int productId) {
    SkipNode* update[MAX_SKIP_LEVEL + 1];
    SkipNode* current = list->header;
    
    // Find node to delete
    for (int i = list->currentLevel; i >= 0; i--) {
        while (current->forward[i] && current->forward[i]->product.id < productId) {
            current = current->forward[i];
        }
        update[i] = current;
    }
    
    current = current->forward[0];
    
    if (!current || current->product.id != productId) {
        return false;
    }
    
    // Update forward pointers
    for (int i = 0; i <= list->currentLevel; i++) {
        if (update[i]->forward[i] != current) break;
        update[i]->forward[i] = current->forward[i];
    }
    
    // Free the node
    free(current->forward);
    free(current);
    
    // Update current level
    while (list->currentLevel > 0 && list->header->forward[list->currentLevel] == NULL) {
        list->currentLevel--;
    }
    
    list->size--;
    return true;
}

// Heap utility functions
static void heap_swap(HeapEntry* a, HeapEntry* b) { 
    HeapEntry t = *a; *a = *b; *b = t; 
}

static void heap_sift_up(HeapEntry* heap, int idx) {
    while (idx > 0) {
        int parent = (idx - 1) / 2;
        if (heap[parent].stock <= heap[idx].stock) break;
        heap_swap(&heap[parent], &heap[idx]);
        idx = parent;
    }
}

static void heap_sift_down(HeapEntry* heap, int size, int idx) {
    while (1) {
        int l = idx * 2 + 1, r = l + 1, smallest = idx;
        if (l < size && heap[l].stock < heap[smallest].stock) smallest = l;
        if (r < size && heap[r].stock < heap[smallest].stock) smallest = r;
        if (smallest == idx) break;
        heap_swap(&heap[idx], &heap[smallest]);
        idx = smallest;
    }
}

static void heap_push(struct Inventory* inv, HeapEntry e) {
    if (inv->heapSize == inv->heapCapacity) {
        inv->heapCapacity = inv->heapCapacity ? inv->heapCapacity * 2 : 64;
        inv->heap = (HeapEntry*)realloc(inv->heap, inv->heapCapacity * sizeof(HeapEntry));
    }
    inv->heap[inv->heapSize] = e;
    heap_sift_up(inv->heap, inv->heapSize);
    inv->heapSize++;
}

static void undo_push(Inventory* inv, UndoAction action) {
    UndoNode* node = (UndoNode*)malloc(sizeof(UndoNode));
    node->action = action;
    node->next = inv->undoTop;
    inv->undoTop = node;
}

Inventory* inventory_create(void) {
    Inventory* inv = (Inventory*)calloc(1, sizeof(Inventory));
    inv->products = create_skip_list();
    return inv;
}

void inventory_destroy(Inventory* inv) {
    if (!inv) return;
    
    // Clean up skip list
    SkipNode* current = inv->products->header->forward[0];
    while (current) {
        SkipNode* next = current->forward[0];
        free(current->forward);
        free(current);
        current = next;
    }
    free(inv->products->header->forward);
    free(inv->products->header);
    free(inv->products);
    
    // Clean up heap
    free(inv->heap);
    
    // Clean up undo stack
    UndoNode* undoCurrent = inv->undoTop;
    while (undoCurrent) {
        UndoNode* next = undoCurrent->next;
        free(undoCurrent);
        undoCurrent = next;
    }
    
    free(inv);
}

bool inventory_add_product(Inventory* inv, Product p) {
    if (!inv) return false;
    
    // Check if product already exists
    SkipNode* existing = skip_list_search(inv->products, p.id);
    
    UndoAction action;
    action.type = ACT_ADD;
    action.after = p;
    if (existing) {
        action.before = existing->product;
    } else {
        memset(&action.before, 0, sizeof(Product));
    }
    
    // Insert/update product
    bool result = skip_list_insert(inv->products, p);
    
    if (result) {
        // Update version and heap
        if (p.id < MAX_PRODUCTS) {
            inv->productVersion[p.id]++;
            HeapEntry he = { p.stock, p.id, inv->productVersion[p.id] };
            heap_push(inv, he);
        }
        
        // Push to undo stack
        undo_push(inv, action);
    }
    
    return result;
}

Product* inventory_get_product(Inventory* inv, int productId) {
    if (!inv) return NULL;
    
    SkipNode* node = skip_list_search(inv->products, productId);
    return node ? &node->product : NULL;
}

bool inventory_remove_product(Inventory* inv, int productId) {
    if (!inv) return false;
    
    SkipNode* existing = skip_list_search(inv->products, productId);
    if (!existing) return false;
    
    UndoAction action;
    action.type = ACT_REMOVE;
    action.before = existing->product;
    memset(&action.after, 0, sizeof(Product));
    
    bool result = skip_list_delete(inv->products, productId);
    
    if (result) {
        undo_push(inv, action);
    }
    
    return result;
}

bool inventory_update_stock(Inventory* inv, int productId, int newStock) {
    if (!inv) return false;
    
    SkipNode* node = skip_list_search(inv->products, productId);
    if (!node) return false;
    
    UndoAction action;
    action.type = ACT_UPDATE_STOCK;
    action.before = node->product;
    action.after = node->product;
    action.after.stock = newStock;
    
    int oldStock = node->product.stock;
    node->product.stock = newStock;
    
    // Update version and heap
    if (productId < MAX_PRODUCTS) {
        inv->productVersion[productId]++;
        HeapEntry he = { newStock, productId, inv->productVersion[productId] };
        heap_push(inv, he);
    }
    
    undo_push(inv, action);
    
    return true;
}

void inventory_print_all(Inventory* inv) {
    if (!inv) return;
    
    printf("\n-- Inventory --\n");
    
    SkipNode* current = inv->products->header->forward[0];
    while (current) {
        Product* p = &current->product;
        printf("ID:%d Name:%s Cat:%s Supplier:%d Price:%.2f Stock:%d\n",
               p->id, p->name, p->category, p->supplierId, p->price, p->stock);
        current = current->forward[0];
    }
}

void inventory_heap_refresh_all(Inventory* inv) {
    if (!inv) return;
    
    // Clear existing heap
    inv->heapSize = 0;
    
    // Rebuild heap from current inventory
    SkipNode* current = inv->products->header->forward[0];
    while (current) {
        Product* p = &current->product;
        if (p->id < MAX_PRODUCTS) {
            inv->productVersion[p->id]++;
            HeapEntry he = { p->stock, p->id, inv->productVersion[p->id] };
            heap_push(inv, he);
        }
        current = current->forward[0];
    }
}

int inventory_pop_low_stock_alerts(Inventory* inv, int threshold, int maxCount) {
    if (!inv) return 0;
    
    int count = 0;
    printf("\n-- Low Stock Alerts (threshold <= %d) --\n", threshold);
    
    // Process heap entries
    while (inv->heapSize > 0 && count < maxCount) {
        HeapEntry top = inv->heap[0];
        
        // Remove from heap
        inv->heap[0] = inv->heap[--inv->heapSize];
        if (inv->heapSize > 0) {
            heap_sift_down(inv->heap, inv->heapSize, 0);
        }
        
        // Check if entry is still valid
        if (top.productId < MAX_PRODUCTS && inv->productVersion[top.productId] == top.version) {
            if (top.stock <= threshold) {
                Product* p = inventory_get_product(inv, top.productId);
                if (p && p->stock == top.stock) {
                    printf("ALERT: ID:%d Name:%s Stock:%d\n", p->id, p->name, p->stock);
                    count++;
                }
            }
        }
        
        if (top.stock > threshold) break;
    }
    
    return count;
}

bool inventory_undo_last(Inventory* inv) {
    if (!inv || !inv->undoTop) return false;
    
    UndoNode* node = inv->undoTop;
    UndoAction action = node->action;
    inv->undoTop = node->next;
    
    switch (action.type) {
        case ACT_ADD:
            if (action.before.id != 0) {
                // Restore previous version
                skip_list_insert(inv->products, action.before);
            } else {
                // Remove the added product
                skip_list_delete(inv->products, action.after.id);
            }
            break;
            
        case ACT_REMOVE:
            // Re-add the removed product
            skip_list_insert(inv->products, action.before);
            break;
            
        case ACT_UPDATE_STOCK:
            // Restore previous stock level
            skip_list_insert(inv->products, action.before);
            break;
    }
    
    free(node);
    printf("Undo completed.\n");
    return true;
}