#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "errors.h"
#include "io.h"
#include "menu.h"

menu_page_t* menu_current_page(menu_t* menu) {
    assert(menu != NULL);
    assert(menu->current_index < menu->pages_len);
    return menu->pages + menu->current_index;
}

error_code menu_init(menu_t* menu, uint32_t pages_cap) {
    assert(menu != NULL);

    menu->current_index = 0;
    menu->pages_cap =  pages_cap;
    menu->pages_len = 0;
    menu->pages = calloc(sizeof(menu_page_t), pages_cap);
    if (menu->pages == NULL) {
        return ERR_ALLOC;
    }

    return ERR_NONE;
}

void menu_set_page(menu_t* menu, uint32_t start_index) {
    assert(start_index < menu->pages_len);
    menu->current_index = start_index;
}


void menu_add_page(menu_t* menu, menu_page_t page) {
    assert(menu != NULL);
    assert(menu->pages_len < menu->pages_cap);

    menu->pages[menu->pages_len] = page;
    menu->pages_len++;
}

error_code menu_page_init(menu_page_t* page, uint32_t items_cap) {
    assert(page != NULL);

    page->items_cap = items_cap;
    page->items_len = 0;
    page->items = calloc(sizeof(menu_item_t), items_cap);
    if (page->items == NULL) {
        return ERR_ALLOC;
    }

    return ERR_NONE;
}

void menu_page_deinit(menu_page_t* page) {
    assert(page->items != NULL);
    free(page->items);
    page->items = NULL;
    page->items_cap = 0;
    page->items_len = 0;
}

void menu_page_add_item(menu_page_t* page, menu_item_t item) {
    assert(page != NULL);
    assert(page->items_len < page->items_cap);

    page->items[page->items_len] = item;
    page->items_len++;
}


void menu_page_display(menu_page_t* page, void (*item_display_func)(menu_item_t*)) {
    assert(page != NULL);
    assert(item_display_func != NULL);
    for (uint32_t item_index = 0; item_index < page->items_len; item_index++) {
        item_display_func(page->items + item_index);
    }
}

void menu_deinit(menu_t* menu) {
    assert(menu != NULL);
    assert(menu->pages != NULL);

    for (uint32_t page_index = 0; page_index < menu->pages_len; page_index++) {
        menu_page_deinit(menu->pages + page_index);
    }

    free(menu->pages);
    menu->pages = NULL;
    menu->pages_cap = 0;
    menu->pages_len = 0;
    menu->current_index = 0;
}


error_code menu_display(menu_t* menu, uint32_t* out, void (*item_dispay_func)(menu_item_t*)) {
    menu_page_t* page = menu_current_page(menu);

    menu_page_display(page, item_dispay_func);
    printf("#> ");
    error_code res = read_uint32(out);
    // check if there is an item with the index we got from the user
    if (res == ERR_NONE) {
        for (uint32_t i = 0; i < page->items_len; i++) {
            if (*out == page->items[i].index) {
                return ERR_NONE;
            }
        }

        // Invalid option selected
        return ERR_MENU_IOPTION;
    }

    return res;
}
