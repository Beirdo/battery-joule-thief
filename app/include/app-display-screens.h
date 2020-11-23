/*
 * Copyright (c) 2020 Gavin Hurlbut
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef __app_display_screens_h_
#define __app_display_screens_h_

typedef uint8_t *(*item_dynamic)(int);

struct display_menu_item_t {
    int x;
    int y;
    int w;
    int size;
    item_dynamic text;
};

typedef void (*menu_action)(int);

struct display_menu_t {
    menu_action display_menu;
    const struct display_menu_item_t *items;
    int item_count;
    menu_action up;
    menu_action left;
    menu_action right;
    menu_action down;
    menu_action enter;
};

struct display_menu_ram_t {
    int index_top;
    int index_current;
};

struct display_item_t {
    int x;
    int y;
    int size;
    uint8_t *static_text;
    item_dynamic dynamic_text;
};

struct display_page_t {
    bool display_logo;
    const struct display_menu_t *menu;
    const struct display_item_t *items;
    int item_count;
    int index_esc;
    int index_up;
    int index_left;
    int index_right;
    int index_down;
    int index_enter;
    int index_menu;
};

#endif /* __app_display_screens_h_ */