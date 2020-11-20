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


struct battery_type_t {
    char *name;
    uint16_t max_voltage;
    uint16_t min_voltage;
};

struct battery_worker_t {
    char *name;
    enum adc_input_names_t signal;
    enum io_names_t select;
    enum io_names_t green;
    enum io_names_t red;
    enum io_names_t shutdown;
    uint8_t channel;
    bool power_good;
    bool enabled;
    struct k_work led_worker;
    struct k_work pwm_worker;
    struct battery_type_t battery_type;
    uint32_t battery_choice_bits;
};


#define FOR_ALL_BAT_TYPES(preamble, x, postamble)   \
preamble                                            \
    x(ZincCarbon_AA, 1500, 750)                     \
    x(Alkaline_AA, 1500, 850)                       \
    x(NiCd_AA, 1200, 900)                           \
    x(NiMH_AA, 1200, 900)                           \
    x(ZincCarbon_AAA, 1500, 750)                    \
    x(Alkaline_AAA, 1500, 850)                      \
    x(NiCd_AAA, 1200, 900)                          \
    x(NiMH_AAA, 1200, 900)                          \
    x(External_3V3, 3300, 850)                      \
    x(CR2032, 3000, 1600)                           \
    x(CR123A, 3000, 1600)                           \
    x(External_12V, 12000, 1500)                    \
    x(ZincCarbon_9V, 9000, 4500)                    \
    x(Alkaline_9V, 9000, 5100)                      \
    x(NiCd_9V, 7200, 5400)                          \
    x(NiMH_9V, 7200, 5400)                          \
    x(Lithium_9V, 9000, 4800)                       \
postamble



#define BAT_CHOICE_AA   (BIT(ZincCarbon_AA) | BIT(Alkaline_AA) | BIT(NiCd_AA) | BIT(NiMH_AA))
#define BAT_CHOICE_AAA  (BIT(ZincCarbon_AAA) | BIT(Alkaline_AAA) | BIT(NiCd_AAA) | BIT(NiMH_AAA))
#define BAT_CHOICE_9V   (BIT(ZincCarbon_9V) | BIT(Alkaline_9V) | BIT(NiCd_9V) | BIT(NiMH_9V) | BIT(Lithium_9V))


#define FOR_ALL_BATS(preamble, x, postamble)                            \
preamble                                                                \
    x(BATT1a, BATSEL1a, LED1ag, LED1ar, nSD1, 0, BAT_CHOICE_AA)         \
    x(BATT1b, BATSEL1b, LED1bg, LED1br, nSD1, 0, BAT_CHOICE_AAA)        \
    x(BATT2a, BATSEL2a, LED2ag, LED2ar, nSD2, 1, BAT_CHOICE_AA)         \
    x(BATT2b, BATSEL2b, LED2bg, LED2br, nSD2, 1, BIT(External_3V3))     \
    x(BATT3a, BATSEL3a, LED3ag, LED3ar, nSD3, 2, BAT_CHOICE_AA)         \
    x(BATT3b, BATSEL3b, LED3bg, LED3br, nSD3, 2, BIT(CR2032))           \
    x(BATT4a, BATSEL4a, LED4ag, LED4ar, nSD4, 3, BAT_CHOICE_AA)         \
    x(BATT4b, BATSEL4b, LED4bg, LED4br, nSD4, 3, BIT(CR123A))           \
    x(BATT5a, BATSEL5a, LED5ag, LED5ar, nSD5, 4, BIT(External_12V))     \
    x(BATT5b, BATSEL5b, LED5bg, LED5br, nSD5, 4, BAT_CHOICE_9V)         \
postamble

#define BAT_ENUM(label, ...) label,
FOR_ALL_BATS(enum battery_t {, BAT_ENUM, };)

FOR_ALL_BAT_TYPES(enum battery_name_t {, BAT_ENUM, };)

extern struct battery_worker_t battery_worker[];
extern size_t battery_count;
extern const struct battery_type_t battery_types[];
extern size_t battery_type_count;

int input_batteries_init(void);
uint8_t approximate_battery_level(struct battery_worker_t *battery);


#endif /* __app_input_batteries_h_ */