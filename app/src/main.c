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


void main(void)
{
    int ret;

    ret = gpios_init();
    if (ret !=0) {
        return;
    }
    
    ret = adcs_init();
    if (ret != 0) {
        return;
    }
    
    ret = charge_counters_init();
    if (ret != 0) {
        return;
    }
    
    ret = input_batteries_init();
    if (ret != 0) {
        return;
    }
    
    ret = charger_init();
    if (ret != 0) {
        return;
    }

    ret = display_init();
    if (ret != 0) {
        return;
    }

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


