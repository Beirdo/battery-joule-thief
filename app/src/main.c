/*
 * Copyright (c) 2020 Gavin Hurlbut
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <zephyr.h>
#include <kernel.h>

#include "app-gpios.h"
#include "app-adcs.h"
#include "app-charge-counters.h"
#include "app-input-batteries.h"
#include "app-charger.h"
#include "app-display.h"


bool initialized;
bool cpu_led_on;
struct k_delayed_work led_pulse_worker;

void main_failed(void);
void led_pulse_start(void);
static void led_pulse(struct k_work *work);

void main(void)
{
    int ret;
    
    initialized = false;

    ret = gpios_init();
    if (ret !=0) {
        return;
    }

    /* Turn off the LEDs from the bootloader */
    write_io_pin(LEDBootG, false);
    write_io_pin(LEDBootR, false);
    
    /* Turn on the LED for the CPU */
    cpu_led_on = true;
    write_io_pin(LEDCPUg, true);
    write_io_pin(LEDCPUr, false);

    ret = adcs_init();
    if (ret != 0) {
        return main_failed();
    }
    
    ret = charge_counters_init();
    if (ret != 0) {
        return main_failed();
    }
    
    ret = input_batteries_init();
    if (ret != 0) {
        return main_failed();
    }
    
    ret = charger_init();
    if (ret != 0) {
        return main_failed();
    }

    ret = display_init();
    if (ret != 0) {
        return main_failed();
    }

    initialized = true;
    
    /* Start the CPU LED pulsing */
    led_pulse_start();

    /* Start ADC readings once a second (it reschedules itself) */
    adcs_start();
    
    /* Start the charge counters - once a second, reschedules itself */
    charge_counters_start();

    /* Start the display update work item (it gets scheduled by buttons or 
     * ADC complete) 
     */
    display_start();

    /* Start loop */
    while (1) {
        /* Check if batteries are depleted, disable those that are */
        
        /* If the enables have changed, reprogram the PWM phases */
        
        /* Sleep for 100ms */
        k_msleep(100);
    }
}

void main_failed(void)
{
    write_io_pin(LEDCPUg, false);
    write_io_pin(LEDCPUr, true);
}

static void led_pulse(struct k_work *work)
{
    if (initialized) {
        cpu_led_on = !cpu_led_on;
        write_io_pin(LEDCPUg, cpu_led_on);
        write_io_pin(LEDCPUr, false);
        k_delayed_work_submit(&led_pulse_worker, K_MSEC(500));
    }
}


void led_pulse_start(void)
{
	k_delayed_work_init(&led_pulse_worker, led_pulse);
    k_delayed_work_submit(&led_pulse_worker, K_MSEC(500));
}