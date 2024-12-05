#ifndef MENU_H
#define MENU_H

#include <stdint.h>
#include "errors.h"


typedef struct {
    uint32_t index;
    char* prompt;
} menu_item_t;

typedef struct {
    menu_item_t* items;
    uint32_t items_len;
    uint32_t items_cap;
} menu_page_t;

typedef struct {
    menu_page_t* pages;
    uint32_t pages_len;
    uint32_t pages_cap;
    uint32_t current_index;
} menu_t;

error_code menu_page_init(menu_page_t* page, uint32_t items_cap);
void menu_page_deinit(menu_page_t* page);
void menu_page_add_item(menu_page_t* page, menu_item_t item);
void menu_page_display(menu_page_t* page, void (*item_display_func)(menu_item_t*));

error_code menu_init(menu_t* menu, uint32_t pages_cap);
void menu_deinit(menu_t* menu);
menu_page_t* menu_current_page(menu_t* menu);
void menu_add_page(menu_t* menu, menu_page_t page);
void menu_set_page(menu_t* menu, uint32_t start_index);
error_code menu_display(menu_t* menu, uint32_t* out, void (*item_dispay_func)(menu_item_t*));

#endif
