/*
 * Copyright (c) 2020 Gavin Hurlbut
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef __app_display_screens_h_
#define __app_display_screens_h_


struct display_menu_item_t {
    uint8_t *text;
    int menu_item_id;
};

struct display_menu_t {
    struct display_menu_item_t *items;
    int item_count;
    int index_top;
    int index_current;
};

struct display_item_t {
    int x;
    int y;
    int size;
    uint8_t *static_text;
    uint8_t *(*dynamic_text)(void *addr);
};

struct display_page_t {
    bool display_logo;
    struct display_menu_t *menu;
    struct display_item_t *items;
    int item_count;
    int index_esc;
    int index_up;
    int index_left;
    int index_right;
    int index_down;
    int index_enter;
};

extern struct display_page_t *display_pages;
extern int display_page_count;

#endif /* __app_display_screens_h_ */