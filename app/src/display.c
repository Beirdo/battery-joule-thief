/*
 * Copyright (c) 2020 Gavin Hurlbut
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <zephyr.h>
#include <device.h>
#include <devicetree.h>
#include <kernel.h>
#include <adafruit-gfx-api.h>
#include <stdio.h>

#include "app-display.h"
#include "app-display-screens.h"
#include "app-gpios.h"
#include "app-input-batteries.h"
#include "app-charger.h"
#include "app-charge-counters.h"
#include "app-utils.h"


const struct device *display;

static void display_update_worker(struct k_work *work);

void battery_menu_display(int index);
void battery_menu_prev(int index);
void battery_menu_next(int index);
void battery_menu_select(int index);

uint8_t *battery_print_label(int index);
uint8_t *battery_print_enabled(int index);
uint8_t *battery_print_type(int index);
uint8_t *battery_print_voltage(int index);
uint8_t *battery_print_charge(int index);


/*
 * Logo
 * Display items:
 * - percentage bars for all 10 batteries + charger? (with active bar below each, selector arrow underneath, hit enter on one, it should go to battery/charger details page)
 * - For each battery (1-10)
 *   - selected battery designator
 *   - selected battery type
 *   - current voltage
 *   - mAh drained from battery (total)
 *   - enabled / disabled
 * - For charger
 *   - current voltage
 *   - mAh applied to battery (total)
 *   - active / charged
 * Hit enter on details page, should go to setup page for that battery/charger
 * Battery setup page should only allow for deactivate if currently active
 * Battery setup page should allow to select battery source, and battery type where applicable (i.e. JST connectors, 9V battery clip)
 * Clear mAh counter when activating new source
 * Charger setup page should only allow for deactivate if currently active
 * Charger setup page should allow to reactivate with a new battery connected, not allow if the charged battery is still connected
 * Clear mAh counter when activating charger for new battery
 */

#if 0
typedef uint8_t *(item_dynamic)(int);

struct display_item_t {
    int x;
    int y;
    int size;
    uint8_t *static_text;
    item_dynamic dynamic_text;
};
#endif

const struct display_item_t battery_item[] = {
    {0, 0, 1, NULL, battery_print_label},
    {10, 0, 1, NULL, battery_print_enabled},
    {0, 1, 1, NULL, battery_print_type},
    {0, 2, 1, "Voltage:", NULL},
    {10, 2, 1, NULL, battery_print_voltage},
    {0, 3, 1, "Charge:", NULL},
    {10, 3, 1, NULL, battery_print_charge},
};

const struct display_menu_t display_menu[] = {
    {
        .display_menu = battery_menu_display,
        .left = battery_menu_prev,
        .right = battery_menu_next,
        .enter = battery_menu_select,
    },    
};
const int menu_count = NELEMENTS(display_menu);

struct display_menu_ram_t display_menu_ram[NELEMENTS(display_menu)];

const struct display_page_t display_pages[] = {
    {
        .display_logo = true,
        .index_esc = -1,
        .index_up = -1,
        .index_down = -1,
        .index_right = -1,
        .index_left = -1,
        .index_enter = 1,
    },
    {
        .menu = &display_menu[0],
        .index_esc = 0,
        .index_up = -1,
        .index_down = -1,
        .index_right = -1,
        .index_left = -1,
        .index_enter = -1,
    },
    {
        .items = battery_item,
        .item_count = NELEMENTS(battery_item),
        .index_esc = 0,
        .index_up = -1,
        .index_down = -1,
        .index_left = 12,
        .index_right = 3,
        .index_enter = 13,
        .index_menu = 0,
    },
    {
        .items = battery_item,
        .item_count = NELEMENTS(battery_item),
        .index_esc = 0,
        .index_up = -1,
        .index_down = -1,
        .index_left = 2,
        .index_right = 4,
        .index_enter = 13,
        .index_menu = 1,
    },
    {
        .items = battery_item,
        .item_count = NELEMENTS(battery_item),
        .index_esc = 0,
        .index_up = -1,
        .index_down = -1,
        .index_left = 3,
        .index_right = 5,
        .index_enter = 13,
        .index_menu = 2,
    },
    {
        .items = battery_item,
        .item_count = NELEMENTS(battery_item),
        .index_esc = 0,
        .index_up = -1,
        .index_down = -1,
        .index_left = 4,
        .index_right = 6,
        .index_enter = 13,
        .index_menu = 3,
    },
    {
        .items = battery_item,
        .item_count = NELEMENTS(battery_item),
        .index_esc = 0,
        .index_up = -1,
        .index_down = -1,
        .index_left = 5,
        .index_right = 7,
        .index_enter = 13,
        .index_menu = 4,
    },
    {
        .items = battery_item,
        .item_count = NELEMENTS(battery_item),
        .index_esc = 0,
        .index_up = -1,
        .index_down = -1,
        .index_left = 6,
        .index_right = 8,
        .index_enter = 13,
        .index_menu = 5,
    },
    {
        .items = battery_item,
        .item_count = NELEMENTS(battery_item),
        .index_esc = 0,
        .index_up = -1,
        .index_down = -1,
        .index_left = 7,
        .index_right = 9,
        .index_enter = 13,
        .index_menu = 6,
    },
    {
        .items = battery_item,
        .item_count = NELEMENTS(battery_item),
        .index_esc = 0,
        .index_up = -1,
        .index_down = -1,
        .index_left = 8,
        .index_right = 10,
        .index_enter = 13,
        .index_menu = 7,
    },
    {
        .items = battery_item,
        .item_count = NELEMENTS(battery_item),
        .index_esc = 0,
        .index_up = -1,
        .index_down = -1,
        .index_left = 9,
        .index_right = 11,
        .index_enter = 13,
        .index_menu = 8,
    },
    {
        .items = battery_item,
        .item_count = NELEMENTS(battery_item),
        .index_esc = 0,
        .index_up = -1,
        .index_down = -1,
        .index_left = 10,
        .index_right = 12,
        .index_enter = 13,
        .index_menu = 9,
    },
    {
        .items = battery_item,
        .item_count = NELEMENTS(battery_item),
        .index_esc = 0,
        .index_up = -1,
        .index_down = -1,
        .index_left = 11,
        .index_right = 2,
        .index_enter = 14,
        .index_menu = 10,
    },
};


struct display_page_t const *current_display_page = NULL;
int current_index_menu = 0;
uint8_t line_buffer[22];
struct k_delayed_work display_worker;


int display_init(void)
{
    display = device_get_binding(DT_LABEL(DT_NODELABEL(display)));

    /* Initialize screens */
    for (int i = 0; i < menu_count; i++) {
        display_menu_ram[i].index_top = 0;
        display_menu_ram[i].index_current = 0;
    }
    
    /* Initialize worker */
	k_delayed_work_init(&display_worker, display_update_worker);

    /* Initialize the Adafruit SSD module */
    return adafruit_gfx_initialize();
}

void display_start(void)
{
    /* start up the display worker */
    k_delayed_work_submit(&display_worker, K_MSEC(100));
}


static void display_update_worker(struct k_work *work)
{
    int i;
    int page_index = current_display_page - &display_pages[0];
    
    if (!current_display_page) {
        current_display_page = &display_pages[0];
    }

    adafruit_gfx_clearDisplay();

    if (current_display_page->display_logo) {
        adafruit_gfx_reset();
        current_display_page++;
    } else {
        const struct display_menu_t *menu = current_display_page->menu;
        if (menu) {
            int menu_index = menu - &display_menu[0];
            if (menu->display_menu) {
                menu->display_menu(menu_index);
            }

            const struct display_menu_item_t *menu_item = menu->items;
            for (i = 0; menu_item && i < menu->item_count; menu_item++, i++) {
                /* Display the menu item.  TBD */
            }            
        } 
        
        const struct display_item_t *item = current_display_page->items;
        for (i = 0; item && i < current_display_page->item_count; item++, i++) {
            uint8_t *str = item->static_text;
            
            if (item->dynamic_text) {
                str = item->dynamic_text(page_index);
            }

            if (!str) {
                continue;
            }
            
            /* Display the item. */
            adafruit_gfx_setCursor(item->x, item->y);
            adafruit_gfx_setTextSize(item->size);
            adafruit_gfx_setTextColor(WHITE, BLACK);

            for (; *str; str++) {
                adafruit_gfx_write(*str);
            }
        }
    }

    adafruit_gfx_display();

    k_delayed_work_submit((struct k_delayed_work *)work, K_MSEC(5000));
}


void handler_button(enum io_names_t pin_name)
{
    const struct display_menu_t *menu = current_display_page->menu;
    int index;
    int menu_index;
    
    if (menu) {
        menu_index = menu - &display_menu[0];
    }
    
    switch(pin_name) {
        case UP:
            index = current_display_page->index_up;
            if (index != -1) {
                break;
            }
            if (menu && menu->up) {
                menu->up(menu_index);
            }
            break;
        case LEFT:
            index = current_display_page->index_left;
            if (index != -1) {
                break;
            }
            if (menu && menu->left) {
                menu->left(menu_index);
            }
            break;
        case RIGHT:
            index = current_display_page->index_right;
            if (index != -1) {
                break;
            }
            if (menu && menu->right) {
                menu->right(menu_index);
            }
            break;
        case DOWN:
            index = current_display_page->index_down;
            if (index != -1) {
                break;
            }
            if (menu && menu->down) {
                menu->down(menu_index);
            }
            break;
        case ENTER:
            index = current_display_page->index_enter;
            if (index != -1) {
                break;
            }
            if (menu && menu->enter) {
                current_index_menu = current_display_page->index_menu;
                menu->enter(menu_index);
            }
            break;
        case ESC:
            index = current_display_page->index_esc;
            if (index != -1) {
                break;
            }
            index = 0;
            break;
        default:
            break;
    }
    
    if (index != -1) {
        current_display_page = &display_pages[index];
    }

    k_delayed_work_submit(&display_worker, K_MSEC(100));
}


void draw_battery_level(int battery_index)
{
    int x0 = battery_index * 10;
    int y0 = 16;
    uint8_t percentage;
    
    if (battery_index == 12) {
        /* This is the output battery */
        percentage = approximate_output_battery_level();
    } else if (battery_index >= 0 && battery_index < battery_count) {
        percentage = approximate_battery_level(battery_index);
    } else {
        /* WTF?  nope. */
        return;
    }

    int filled = ((percentage * 34) + 17) / 100;
    
    adafruit_gfx_drawRect(x0, y0 + 2, 8, 36, WHITE);
    adafruit_gfx_drawRect(x0 + 2, y0, 4, 2, WHITE);
    adafruit_gfx_drawRect(x0 + 1, y0 + 3 + (34 - filled), 6, filled, WHITE);
}

void draw_battery_selector(int battery_index)
{
    int x0 = battery_index * 10;
    int y0 = 63;
    
    adafruit_gfx_fillTriangle(x0, y0, x0 + 7, y0, x0 + 3, y0 - 4, WHITE);
}

void draw_battery_enabled(int battery_index)
{
    int x0 = battery_index * 10;
    int y0 = 56;
    bool enabled;
        
    if (battery_index == 12) {
        /* This is the output battery */
        enabled = charger_enabled();
    } else if (battery_index >= 0 && battery_index < battery_count) {
        enabled = battery_enabled(battery_index);
    } else {
        /* WTF?  nope. */
        return;
    }

    if (enabled) {
        adafruit_gfx_fillRect(x0, y0, 8, 2, WHITE);
    }
}

void draw_big_arrow(void)
{
    int x0 = 104;
    int y0 = 36;
    
    adafruit_gfx_fillRect(x0, y0 - 2, 6, 5, WHITE);
    adafruit_gfx_fillTriangle(x0 + 6, y0 - 4, x0 + 6, y0 + 4, x0 + 10, y0, WHITE);
}

void battery_menu_display(int index)
{
    struct display_menu_ram_t *menu_ram = &display_menu_ram[index];
    int i = 0;
    
    draw_big_arrow();
    
    for (i = 0; i < 12; i++) {
        if (i == 11) {
            i = 12;
        }
        draw_battery_level(i);
        draw_battery_enabled(i);
        if (i == menu_ram->index_current) {
            draw_battery_selector(i);
        }
    }
}


void battery_menu_prev(int index)
{
    struct display_menu_ram_t *menu_ram = &display_menu_ram[index];

    int i = menu_ram->index_current;
    
    switch(i) {
        case 0:
            i = 12;
            break;
        case 12:
            i = 9;
            break;
            
        case 2:
        case 3:
        case 4:
        case 5:
        case 6:
        case 7:
        case 8:
        case 9:
            i--;
            break;

        case 1:
        default:
            i = 0;
            break;
    }
    
    menu_ram->index_current = i;
}

void battery_menu_next(int index)
{
    struct display_menu_ram_t *menu_ram = &display_menu_ram[index];
    
    int i = menu_ram->index_current;
    
    switch(i) {
        case 0:
        case 1:
        case 2:
        case 3:
        case 4:
        case 5:
        case 6:
        case 7:
        case 8:
            i++;
            break;

        case 9:
            i = 12;
            break;

        case 12:
        default:
            i = 0;
            break;
    }
    
    menu_ram->index_current = i;
}

void battery_menu_select(int index)
{
    const struct display_menu_t *menu = &display_menu[index];
    struct display_menu_ram_t *menu_ram = &display_menu_ram[index];
    ARG_UNUSED(menu);
    ARG_UNUSED(menu_ram);
}


uint8_t *battery_print_label(int index)
{
    const struct display_page_t *page = &display_pages[index];
    int battery_index = page->index_menu;

    if (battery_index == 10) {
        /* This is the charger */
        return "Charger";
    }
    
    struct battery_worker_t *battery = &battery_worker[battery_index];
    return (uint8_t *)battery->name;
}

uint8_t *battery_print_enabled(int index)
{
    const struct display_page_t *page = &display_pages[index];
    int battery_index = page->index_menu;
    bool enabled;
    
    if (battery_index == 10) {
        enabled = charger_enabled();
    } else {
        enabled = battery_enabled(battery_index);
    }
    
    if (enabled) {
        return "Enabled";
    } else {
        return "Disabled";
    }
}

uint8_t *battery_print_type(int index)
{
    const struct display_page_t *page = &display_pages[index];
    int battery_index = page->index_menu;
    struct battery_type_t *battery_type;

    if (battery_index == 10) {
        battery_type = &battery_type[LiIon_18650];
    } else if (!battery_enabled(battery_index)) {
        return NULL;
    } else {
        struct battery_worker_t *battery = &battery_worker[battery_index];
        battery_type = &battery->battery_type;
    }
    
    return (uint8_t *)battery_type->name;
}

uint8_t *battery_print_voltage(int index)
{
    const struct display_page_t *page = &display_pages[index];
    int battery_index = page->index_menu;
    enum adc_input_names_t input;
    int mv;
    int v;
    
    if (battery_index == 10) {
        input = VOUT;
    } else {
        struct battery_worker_t *battery = &battery_worker[battery_index];
        input = battery->signal;
    }
    
    mv = adc_inputs[input].value_mv;
    v = mv / 1000;
    mv = mv % 1000;
    
    snprintf(line_buffer, 22, "%01d.%03d V", v, mv);
    return line_buffer;
}

uint8_t *battery_print_charge(int index)
{
    const struct display_page_t *page = &display_pages[index];
    int battery_index = page->index_menu;
    int counter_index = battery_index / 2;
    struct charge_counter_t *counter = &charge_counter[counter_index];
    int mAh = counter->mAh;
    
    snprintf(line_buffer, 22, "%d mAH", mAh);
    return line_buffer;
}
