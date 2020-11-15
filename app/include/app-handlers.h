/*
 * Copyright (c) 2020 Gavin Hurlbut
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef __app_handlers_h_
#define __app_handlers_h_

#include <zephyr.h>
#include "app-gpios.h"


void handler_charge_counter(enum io_names_t pin_name);
void handler_power_good(enum io_names_t pin_name);
void handler_charger(enum io_names_t pin_name);
void handler_button(enum io_names_t pin_name);


#endif /* __app_handlers_h_ */