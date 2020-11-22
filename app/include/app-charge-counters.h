/*
 * Copyright (c) 2020 Gavin Hurlbut
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef __app_charge_counters_h_
#define __app_charge_counters_h_

#include <zephyr.h>
#include <kernel.h>
#include "app-gpios.h"
#include "app-devices.h"

struct charge_counter_t {
    int32_t raw_count;
    int32_t mAh;
    uint64_t start_time;
    enum io_names_t polarity;
    enum io_names_t interrupt;
    enum io_names_t shutdown;
    struct k_delayed_work worker;
};


int charge_counters_init(void);
void charge_counters_start(void);

extern struct charge_counter_t charge_counter[CHARGE_COUNTER_COUNT];


#endif /* __app_charge_counters_h_ */