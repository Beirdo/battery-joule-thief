/*
 * Copyright (c) 2020 Gavin Hurlbut
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef __app_devices_h_
#define __app_devices_h_

#include <zephyr.h>
#include <device.h>

#define IOEXP_COUNT 7
#define ADC_COUNT   6
#define CHARGE_COUNTER_COUNT    6

extern const struct device *porta;
extern const struct device *ioexp[IOEXP_COUNT];
extern const struct device *adc[ADC_COUNT];
extern const struct device *display;
extern const struct device *pwm;

#endif /* __app_devices_h_ */