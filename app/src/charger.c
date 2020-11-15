/*
 * Copyright (c) 2020 Gavin Hurlbut
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <zephyr.h>
#include <kernel.h>

#include "app-gpios.h"


int charger_init(void) {
    write_io_pin(nSDO, true);
    return 0;
}


void handler_charger(enum io_names_t pin_name)
{
    int ret;
    bool standby;
    bool charge;
    bool shutdown;
    
    ret = read_io_pin(nSTANDBY, &standby);
    if (ret != 0) {
        return;
    }
    
    ret = read_io_pin(nCHARGE, &charge);
    if (ret != 0) {
        return;
    }
    
    shutdown = standby || !charge;

    write_io_pin(nSDO, shutdown);
    write_io_pin(LEDOg, charge);
    write_io_pin(LEDOr, standby);
    write_io_pin(LEDActive, !shutdown);
}

