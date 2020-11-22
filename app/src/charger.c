/*
 * Copyright (c) 2020 Gavin Hurlbut
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <zephyr.h>
#include <kernel.h>

#include "app-gpios.h"
#include "app-utils.h"
#include "app-adcs.h"


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

uint8_t approximate_output_battery_level(void)
{
	uint16_t min_voltage = 3000;
	uint16_t max_voltage = 4200;
	uint16_t current_voltage;
	uint16_t temp;
	
	current_voltage = clamp(adc_inputs[VOUT].value_mv, min_voltage, max_voltage);

	temp = (current_voltage - min_voltage) * 100;
	temp /= (max_voltage - min_voltage);
	
	return temp;
}
