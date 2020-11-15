/*
 * Copyright (c) 2020 Gavin Hurlbut
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef __app_charge_counters_h_
#define __app_charge_counters_h_

#include <zephyr.h>

int charge_counters_init(void);
void charge_counters_start(void);


#endif /* __app_charge_counters_h_ */