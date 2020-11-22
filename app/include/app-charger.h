/*
 * Copyright (c) 2020 Gavin Hurlbut
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef __app_charger_h_
#define __app_charger_h_

#include <zephyr.h>

int charger_init(void);
uint8_t approximate_output_battery_level(void);


#endif /* __app_charger_h_ */