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


const struct device *display;


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

