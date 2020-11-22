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

#include "app-display.h"
#include "app-display-screens.h"
#include "app-gpios.h"
#include "app-input-batteries.h"
#include "app-charger.h"


const struct device *display;

#if 0
struct display_page_t {
    bool display_logo;
    struct display_menu_t *menu;
    struct display_item_t **items;
    int item_count;
    int index_esc;
    int index_up;
    int index_left;
    int index_right;
    int index_down;
    int index_enter;
};
#endif

/*
 * Logo
 * Display items:
 * - percentage bars for all 10 batteries + charger? (with active "LED" above each, selector arrow underneath, hit enter on one, it should go to battery/charger details page)
 * - For each battery (1-5)
 *   - selected battery designator
 *   - selected battery type
 *   - current voltage
 *   - mAh drained from battery (total)
 *   - active / drained
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


struct display_page_t display_pages[] = {
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
        .menu = NULL, //&display_menu[0],
        .index_esc = 0,
        .index_up = -1,
        .index_down = -1,
        .index_right = -1,
        .index_left = -1,
        .index_enter = -1,
    }
};


int display_init(void)
{
    display = device_get_binding(DT_LABEL(DT_NODELABEL(display)));

    /* Initialize screens */
    
    /* Initialize the Adafruit SSD module */
    return adafruit_gfx_initialize();
}

void display_start(void)
{
    /* start up the display worker */
}




void handler_button(enum io_names_t pin_name)
{
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
    
    adafruit_gfx_fillRect(x0, y0, 8, 2, WHITE);
}

void draw_big_arrow(void)
{
    int x0 = 104;
    int y0 = 36;
    
    adafruit_gfx_fillRect(x0, y0 - 2, 6, 5, WHITE);
    adafruit_gfx_fillTriangle(x0 + 6, y0 - 4, x0 + 6, y0 + 4, x0 + 10, y0, WHITE);
}