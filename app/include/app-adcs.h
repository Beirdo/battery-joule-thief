/*
 * Copyright (c) 2020 Gavin Hurlbut
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef __app_adcs_h_
#define __app_adcs_h_

#include <zephyr.h>

#include <zephyr.h>
#include <device.h>
#include <devicetree.h>
#include <drivers/adc.h>
#include <kernel.h>


struct adc_inputs_t {
    char *name;
    uint16_t raw_value;
    uint16_t value_mv;
    uint16_t reference_mv;
    struct device **pdev;
    struct adc_channel_cfg config;
};

#define FOR_ALL_ADCS(preamble, x, postamble)    \
preamble                                        \
    x(VBATT1a, adc[0], 0, 2048)                 \
    x(VBATT1b, adc[0], 1, 2048)                 \
    x(VOUT1, adc[0], 2, 5059)                   \
                                                \
    x(VBATT2a, adc[1], 0, 2048)                 \
    x(VBATT2b, adc[1], 1, 3480)                 \
    x(VOUT2, adc[1], 2, 5059)                   \
                                                \
    x(VBATT3a, adc[2], 0, 2048)                 \
    x(VBATT3b, adc[2], 1, 3480)                 \
    x(VOUT3, adc[2], 2, 5059)                   \
                                                \
    x(VBATT4a, adc[3], 0, 2048)                 \
    x(VBATT4b, adc[3], 1, 3480)                 \
    x(VOUT4, adc[3], 2, 5059)                   \
                                                \
    x(VBATT5a, adc[4], 0, 12268)                \
    x(VBATT5b, adc[4], 1, 12268)                \
    x(VOUT5, adc[4], 2, 5059)                   \
                                                \
    x(VOUT, adc[5], 0, 5059)                    \
postamble


#define ADC_ENUM(label, ...) label,

FOR_ALL_ADCS(enum adc_input_names_t {, ADC_ENUM, };)


extern struct adc_inputs_t adc_inputs[];
extern uint16_t adc_input_count;

int adcs_init(void);
void adcs_start(void);


#endif /* __app_adcs_h_ */