#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "common.h"
#include "inventory.h"
#include "orders.h"
#include "search.h"
#include "suppliers.h"

#define VERSION "1.0.0"

typedef struct Config {
    bool debug_mode;
    bool quiet_mode;
    bool batch_mode;
    char data_dir[256];
    char import_file[256];
    char export_file[256];
} Config;

static void print_help(const char* program_name) {
    printf("Supply Chain Management System v%s\n\n", VERSION);
    printf("Usage: %s [OPTIONS]\n\n", program_name);
    printf("Options:\n");
    printf("  -h, --help         Show this help message\n");
    printf("  -v, --version      Show version information\n");
    printf("  -d, --debug        Enable debug mode\n");
    printf("  -q, --quiet        Run in quiet mode\n");
    printf("  -b, --batch        Run in batch mode (non-interactive)\n");
    printf("  --data-dir DIR     Specify data directory (default: current)\n");
    printf("  -i, --import FILE  Import data from file\n");
    printf("  -e, --export FILE  Export data to file\n");
    printf("\nExamples:\n");
    printf("  %s                    # Run interactive mode\n", program_name);
    printf("  %s --debug            # Run with debug output\n", program_name);
    printf("  %s --import data.csv  # Import from CSV file\n", program_name);
}

static void print_version() {
    printf("Supply Chain Management System v%s\n", VERSION);
    printf("Built with advanced data structures for efficient operations.\n");
}

static bool parse_arguments(int argc, char* argv[], Config* config) {
    // Initialize config with defaults
    memset(config, 0, sizeof(Config));
    strcpy(config->data_dir, ".");
    
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "-h") == 0 || strcmp(argv[i], "--help") == 0) {
            print_help(argv[0]);
            return false;
        } else if (strcmp(argv[i], "-v") == 0 || strcmp(argv[i], "--version") == 0) {
            print_version();
            return false;
        } else if (strcmp(argv[i], "-d") == 0 || strcmp(argv[i], "--debug") == 0) {
            config->debug_mode = true;
        } else if (strcmp(argv[i], "-q") == 0 || strcmp(argv[i], "--quiet") == 0) {
            config->quiet_mode = true;
        } else if (strcmp(argv[i], "-b") == 0 || strcmp(argv[i], "--batch") == 0) {
            config->batch_mode = true;
        } else if (strcmp(argv[i], "--data-dir") == 0) {
            if (i + 1 < argc) {
                strncpy(config->data_dir, argv[++i], sizeof(config->data_dir) - 1);
            } else {
                fprintf(stderr, "Error: --data-dir requires a directory path\n");
                return false;
            }
        } else if (strcmp(argv[i], "-i") == 0 || strcmp(argv[i], "--import") == 0) {
            if (i + 1 < argc) {
                strncpy(config->import_file, argv[++i], sizeof(config->import_file) - 1);
            } else {
                fprintf(stderr, "Error: --import requires a file path\n");
                return false;
            }
        } else if (strcmp(argv[i], "-e") == 0 || strcmp(argv[i], "--export") == 0) {
            if (i + 1 < argc) {
                strncpy(config->export_file, argv[++i], sizeof(config->export_file) - 1);
            } else {
                fprintf(stderr, "Error: --export requires a file path\n");
                return false;
            }
        } else {
            fprintf(stderr, "Error: Unknown option '%s'\n", argv[i]);
            fprintf(stderr, "Use -h or --help for usage information\n");
            return false;
        }
    }
    return true;
}

static void seed_sample_data(Inventory* inv, SuppliersDB* sdb, OrdersQueue* oq) {
    Supplier s1 = { .id = 1, .name = "Alpha Supplies", .ratings = {9, 8, 7, 9, 8} };
    Supplier s2 = { .id = 2, .name = "Beta Logistics", .ratings = {8, 9, 8, 8, 7} };
    Supplier s3 = { .id = 3, .name = "Gamma Traders", .ratings = {7, 7, 9, 7, 8} };
    suppliers_insert(sdb, s1); suppliers_insert(sdb, s2); suppliers_insert(sdb, s3);

    Product p1 = { .id=101, .name="Widget A", .category="Gadgets", .supplierId=1, .price=19.99, .stock=50 };
    Product p2 = { .id=102, .name="Widget B", .category="Gadgets", .supplierId=2, .price=24.50, .stock=5 };
    Product p3 = { .id=201, .name="Tool X", .category="Tools", .supplierId=3, .price=49.00, .stock=0 };
    Product p4 = { .id=202, .name="Tool Y", .category="Tools", .supplierId=1, .price=75.00, .stock=12 };
    inventory_add_product(inv, p1);
    inventory_add_product(inv, p2);
    inventory_add_product(inv, p3);
    inventory_add_product(inv, p4);

    Order o1 = { .id = 1, .customer = "Alice", .numItems = 2, .items = {{101, 2}, {102, 1}} };
    Order o2 = { .id = 2, .customer = "Bob", .numItems = 1, .items = {{201, 1}} };
    orders_enqueue(oq, o1); orders_enqueue(oq, o2);
}

static void menu_inventory(Inventory* inv, const Config* config) {
    int ch = -1;
    while (ch != 0) {
        if (!config->quiet_mode) {
            printf("\n" COL_CYAN COL_BOLD "Inventory Management" COL_RESET "\n");
            printf(COL_YELLOW "1" COL_RESET ". Add product      " COL_DIM "(create new catalog item)" COL_RESET "\n");
            printf(COL_YELLOW "2" COL_RESET ". Update stock     " COL_DIM "(change product quantity levels)" COL_RESET "\n");
            printf(COL_YELLOW "3" COL_RESET ". Remove product   " COL_DIM "(delete item from catalog)" COL_RESET "\n");
            printf(COL_YELLOW "4" COL_RESET ". Print inventory  " COL_DIM "(display all current products)" COL_RESET "\n");
            printf(COL_YELLOW "5" COL_RESET ". Undo last action " COL_DIM "(reverse previous inventory change)" COL_RESET "\n");
            printf(COL_YELLOW "6" COL_RESET ". Low stock alerts " COL_DIM "(find items below threshold)" COL_RESET "\n");
            printf(COL_YELLOW "0" COL_RESET ". Back             " COL_DIM "(return to main menu)" COL_RESET "\n> ");
        }
        ch = safe_read_int();
        if (ch == 1) {
            Product p; memset(&p, 0, sizeof(p));
            printf("ID: "); p.id = safe_read_int();
            printf("Name: "); scanf(" %63[^\n]", p.name);
            printf("Category: "); scanf(" %31[^\n]", p.category);
            printf("Supplier ID: "); p.supplierId = safe_read_int();
            printf("Price: "); p.price = safe_read_double();
            printf("Stock: "); p.stock = safe_read_int();
            if (inventory_add_product(inv, p)) {
                if (config->debug_mode) printf("[DEBUG] Product %d added successfully\n", p.id);
                if (!config->quiet_mode) printf("Added.\n");
            } else {
                if (!config->quiet_mode) printf("Add failed (duplicate ID?).\n");
            }
        } else if (ch == 2) {
            printf("Product ID: "); int id = safe_read_int();
            printf("New stock: "); int ns = safe_read_int();
            if (inventory_update_stock(inv, id, ns)) {
                if (config->debug_mode) printf("[DEBUG] Stock updated for product %d to %d\n", id, ns);
                if (!config->quiet_mode) printf("Updated.\n");
            } else {
                if (!config->quiet_mode) printf("Update failed.\n");
            }
        } else if (ch == 3) {
            printf("Product ID: "); int id = safe_read_int();
            if (inventory_remove_product(inv, id)) {
                if (config->debug_mode) printf("[DEBUG] Product %d removed\n", id);
                if (!config->quiet_mode) printf("Removed.\n");
            } else {
                if (!config->quiet_mode) printf("Remove failed.\n");
            }
        } else if (ch == 4) {
            inventory_print_all(inv);
        } else if (ch == 5) {
            if (!inventory_undo_last(inv)) {
                if (!config->quiet_mode) printf("Nothing to undo.\n");
            }
        } else if (ch == 6) {
            printf("Threshold: "); int t = safe_read_int();
            int n = inventory_pop_low_stock_alerts(inv, t, 10);
            if (!n && !config->quiet_mode) printf("No low-stock items at/under %d.\n", t);
        }
    }
}

static void menu_orders(OrdersQueue* oq, Inventory* inv, const Config* config) {
    int ch = -1;
    while (ch != 0) {
        if (!config->quiet_mode) {
            printf("\n" COL_CYAN COL_BOLD "Order Processing" COL_RESET "\n");
            printf(COL_YELLOW "1" COL_RESET ". Enqueue order    " COL_DIM "(add new customer order)" COL_RESET "\n");
            printf(COL_YELLOW "2" COL_RESET ". Process next     " COL_DIM "(fulfill oldest pending order)" COL_RESET "\n");
            printf(COL_YELLOW "3" COL_RESET ". Print queue      " COL_DIM "(show all pending orders)" COL_RESET "\n");
            printf(COL_YELLOW "0" COL_RESET ". Back             " COL_DIM "(return to main menu)" COL_RESET "\n> ");
        }
        ch = safe_read_int();
        if (ch == 1) {
            Order o; memset(&o, 0, sizeof(o));
            printf("Order ID: "); o.id = safe_read_int();
            printf("Customer: "); scanf(" %63[^\n]", o.customer);
            printf("Num items (<=%d): ", MAX_ORDER_ITEMS); o.numItems = safe_read_int(); 
            if (o.numItems > MAX_ORDER_ITEMS) o.numItems = MAX_ORDER_ITEMS;
            for (int i = 0; i < o.numItems; ++i) { 
                printf("Item %d - Product ID: ", i+1); 
                o.items[i].productId = safe_read_int(); 
                printf("Item %d - Quantity: ", i+1); 
                o.items[i].quantity = safe_read_int(); 
            }
            orders_enqueue(oq, o); 
            if (config->debug_mode) printf("[DEBUG] Order %d enqueued for %s\n", o.id, o.customer);
            if (!config->quiet_mode) printf("Enqueued.\n");
        } else if (ch == 2) {
            if (config->debug_mode) printf("[DEBUG] Processing next order...\n");
            orders_process_next(oq, inv);
        } else if (ch == 3) {
            orders_print(oq);
        }
    }
}

static void menu_search(Inventory* inv, SuppliersDB* sdb, const Config* config) {
    SearchCriteria c; memset(&c, 0, sizeof(c));
    int ch = -1;
    while (ch != 0) {
        if (!config->quiet_mode) {
            printf("\n" COL_CYAN COL_BOLD "Search & Filter" COL_RESET "\n");
            printf(COL_YELLOW "1" COL_RESET ". Set price range  " COL_DIM "(min and max price)" COL_RESET "\n");
            printf(COL_YELLOW "2" COL_RESET ". Set category     " COL_DIM "(filter by product type)" COL_RESET "\n");
            printf(COL_YELLOW "3" COL_RESET ". Only in stock    " COL_DIM "(show available items only)" COL_RESET "\n");
            printf(COL_YELLOW "4" COL_RESET ". Sort by price    " COL_DIM "(order results by cost)" COL_RESET "\n");
            printf(COL_YELLOW "5" COL_RESET ". Run search       " COL_DIM "(execute search with filters)" COL_RESET "\n");
            printf(COL_YELLOW "0" COL_RESET ". Back             " COL_DIM "(return to main menu)" COL_RESET "\n> ");
        }
        ch = safe_read_int();
        if (ch == 1) { 
            printf("Min price (0 to skip): "); 
            double min = safe_read_double(); 
            if (min > 0) { c.priceMin = min; c.hasPriceMin = true; }
            printf("Max price (0 to skip): "); 
            double max = safe_read_double(); 
            if (max > 0) { c.priceMax = max; c.hasPriceMax = true; }
            if (config->debug_mode) printf("[DEBUG] Price range: %.2f - %.2f\n", 
                c.hasPriceMin ? c.priceMin : 0.0, c.hasPriceMax ? c.priceMax : 0.0);
        } else if (ch == 2) { 
            printf("Category: "); scanf(" %31[^\n]", c.category); c.hasCategory = true;
            if (config->debug_mode) printf("[DEBUG] Category filter: %s\n", c.category);
        } else if (ch == 3) { 
            c.onlyInStock = !c.onlyInStock; 
            if (!config->quiet_mode) printf("Only in stock: %s\n", c.onlyInStock ? "enabled" : "disabled");
        } else if (ch == 4) { 
            c.sortBy = 'p';
            if (!config->quiet_mode) printf("Sorting by price enabled.\n");
        } else if (ch == 5) { 
            if (config->debug_mode) printf("[DEBUG] Executing search...\n");
            search_build_and_execute(inv, sdb, c); 
        }
    }
}

static void menu_suppliers(SuppliersDB* sdb, const Config* config) {
    int ch = -1;
    while (ch != 0) {
        if (!config->quiet_mode) {
            printf("\n" COL_CYAN COL_BOLD "Suppliers Ranking" COL_RESET "\n");
            printf(COL_YELLOW "1" COL_RESET ". Add supplier     " COL_DIM "(create new supplier entry)" COL_RESET "\n");
            printf(COL_YELLOW "2" COL_RESET ". Delete supplier  " COL_DIM "(remove supplier by ID)" COL_RESET "\n");
            printf(COL_YELLOW "3" COL_RESET ". Show ranked      " COL_DIM "(display suppliers by score)" COL_RESET "\n");
            printf(COL_YELLOW "4" COL_RESET ". Show min score   " COL_DIM "(filter suppliers by rating)" COL_RESET "\n");
            printf(COL_YELLOW "0" COL_RESET ". Back             " COL_DIM "(return to main menu)" COL_RESET "\n> ");
        }
        ch = safe_read_int();
        if (ch == 1) {
            Supplier s; memset(&s, 0, sizeof(s));
            printf("ID: "); s.id = safe_read_int();
            printf("Name: "); scanf(" %63[^\n]", s.name);
            printf("Ratings Q D P R S (0..10): ");
            s.ratings.quality = safe_read_double(); 
            s.ratings.deliveryTime = safe_read_double(); 
            s.ratings.price = safe_read_double(); 
            s.ratings.reliability = safe_read_double(); 
            s.ratings.customerService = safe_read_double();
            suppliers_insert(sdb, s);
            if (config->debug_mode) printf("[DEBUG] Supplier %d (%s) added\n", s.id, s.name);
        } else if (ch == 2) {
            printf("Supplier ID: "); int id = safe_read_int(); 
            if (!suppliers_delete(sdb, id) && !config->quiet_mode) printf("Not found.\n");
            else if (config->debug_mode) printf("[DEBUG] Supplier %d deleted\n", id);
        } else if (ch == 3) {
            suppliers_print_ranked(sdb);
        } else if (ch == 4) {
            printf("Min overall score: "); double m = safe_read_double(); 
            suppliers_print_min_rating(sdb, m);
        }
    }
}

static void run_batch_mode(Inventory* inv, SuppliersDB* sdb, OrdersQueue* oq, const Config* config) {
    printf("Running in batch mode...\n");
    
    if (strlen(config->import_file) > 0) {
        printf("Importing data from: %s\n", config->import_file);
        // TODO: Implement import functionality
    }
    
    // Example batch operations
    printf("Displaying current inventory:\n");
    inventory_print_all(inv);
    
    printf("\nDisplaying supplier rankings:\n");
    suppliers_print_ranked(sdb);
    
    printf("\nDisplaying order queue:\n");
    orders_print(oq);
    
    if (strlen(config->export_file) > 0) {
        printf("Exporting data to: %s\n", config->export_file);
        // TODO: Implement export functionality
    }
}

int main(int argc, char* argv[]) {
    Config config;
    
    if (!parse_arguments(argc, argv, &config)) {
        return 0; // Help or version was shown, or error occurred
    }
    
    if (config.debug_mode) {
        printf("[DEBUG] Debug mode enabled\n");
        printf("[DEBUG] Data directory: %s\n", config.data_dir);
    }
    
    Inventory* inv = inventory_create();
    SuppliersDB* sdb = suppliers_create();
    OrdersQueue* oq = orders_create();
    seed_sample_data(inv, sdb, oq);
    
    if (config.batch_mode) {
        run_batch_mode(inv, sdb, oq, &config);
    } else {
        int choice = -1;
        while (choice != 0) {
            if (!config.quiet_mode) {
                printf("\n" COL_GREEN COL_BOLD "Supply Chain Management System" COL_RESET "\n");
                printf(COL_YELLOW "1" COL_RESET ". Manage Inventory  " COL_DIM "(add, update, remove products)" COL_RESET "\n");
                printf(COL_YELLOW "2" COL_RESET ". Process Orders    " COL_DIM "(handle customer order queue)" COL_RESET "\n");
                printf(COL_YELLOW "3" COL_RESET ". Search Products   " COL_DIM "(filter by price and category)" COL_RESET "\n");
                printf(COL_YELLOW "4" COL_RESET ". Supplier Ranking  " COL_DIM "(manage and rank suppliers)" COL_RESET "\n");
                printf(COL_YELLOW "0" COL_RESET ". Exit              " COL_DIM "(close the application safely)" COL_RESET "\n> ");
            }
            choice = safe_read_int();
            switch (choice) {
                case 1: menu_inventory(inv, &config); break;
                case 2: menu_orders(oq, inv, &config); break;
                case 3: menu_search(inv, sdb, &config); break;
                case 4: menu_suppliers(sdb, &config); break;
                case 0: break;
                default: if (!config.quiet_mode) printf("Invalid selection.\n");
            }
        }
    }

    orders_destroy(oq);
    suppliers_destroy(sdb);
    inventory_destroy(inv);
    if (!config.quiet_mode) printf("Goodbye.\n");
    return 0;
}