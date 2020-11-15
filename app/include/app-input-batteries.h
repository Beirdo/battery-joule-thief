/*
 * Copyright (c) 2020 Gavin Hurlbut
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef __app_input_batteries_h_
#define __app_input_batteries_h_

#include <zephyr.h>
#include <kernel.h>
#include "app-gpios.h"


struct battery_worker_t {
    char *name;
    enum io_names_t select;
    enum io_names_t green;
    enum io_names_t red;
    enum io_names_t shutdown;
    uint8_t channel;
    bool power_good;
    bool enabled;
    struct k_work led_worker;
    struct k_work pwm_worker;
};


#define FOR_ALL_BATS(preamble, x, postamble)        \
preamble                                            \
    x(BATT1a, BATSEL1a, LED1ag, LED1ar, nSD1, 0)    \
    x(BATT1b, BATSEL1b, LED1bg, LED1br, nSD1, 0)    \
    x(BATT2a, BATSEL2a, LED2ag, LED2ar, nSD2, 1)    \
    x(BATT2b, BATSEL2b, LED2bg, LED2br, nSD2, 1)    \
    x(BATT3a, BATSEL3a, LED3ag, LED3ar, nSD3, 2)    \
    x(BATT3b, BATSEL3b, LED3bg, LED3br, nSD3, 2)    \
    x(BATT4a, BATSEL4a, LED4ag, LED4ar, nSD4, 3)    \
    x(BATT4b, BATSEL4b, LED4bg, LED4br, nSD4, 3)    \
    x(BATT5a, BATSEL5a, LED5ag, LED5ar, nSD5, 4)    \
    x(BATT5b, BATSEL5b, LED5bg, LED5br, nSD5, 4)    \
postamble

#define BAT_ENUM(label, ...) label,
FOR_ALL_BATS(enum battery_t {, BAT_ENUM, };)

extern struct battery_worker_t battery_worker[];
extern uint16_t battery_count;


int input_batteries_init(void);


#endif /* __app_input_batteries_h_ */