/*
 * Copyright (c) 2020 Gavin Hurlbut
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <zephyr.h>
#include <device.h>
#include <devicetree.h>
#include <drivers/gpio.h>
#include <kernel.h>

#include "app-gpios.h"
#include "app-devices.h"
#include "app-handlers.h"


struct charge_counter_t {
    int32_t raw_count;
    int32_t mAh;
    uint64_t start_time;
    enum io_names_t polarity;
    enum io_names_t interrupt;
    enum io_names_t shutdown;
    struct k_delayed_work worker;
};


struct charge_counter_t charge_counter[ADC_COUNT] = {
    {0, 0, 0, POL1, INT1, nSD1, {}},
    {0, 0, 0, POL2, INT2, nSD2, {}},
    {0, 0, 0, POL3, INT3, nSD3, {}},
    {0, 0, 0, POL4, INT4, nSD4, {}},
    {0, 0, 0, POL5, INT5, nSD5, {}},
    {0, 0, 0, POLO, INTO, nSDO, {}},
};


static void charge_update_worker(struct k_work *work)
{
	struct charge_counter_t * counter = CONTAINER_OF(
		work, struct charge_counter_t, worker);
	bool shutdown;
	int32_t raw_count;
	int32_t mAh;

    int ret = read_io_pin(counter->shutdown, &shutdown);
    if (ret != 0) {
        goto done;
    }
    
    if (shutdown) {
        goto done;
    }
    
    raw_count = counter->raw_count;
    
    /*
     * 1 interrupt = 1/(Gvh * Rsense) Coulombs
     *   Gvh = 32.55 (typ)
     *   Rsense = 0.1
     *
     * so.  coulombs = count / 3.255
     * 1 Ah = 3600 coulombs
     * 1 mAh = 3.6 coulombs
     *
     * so mAh = coulombs / 3.6
     *        = count / 11.718
     */
     
    mAh = (raw_count * 1000) / 11718;
    counter->mAh = mAh;

done:
    /* Schedule yourself for 1s out */
    k_delayed_work_submit((struct k_delayed_work *)work, K_MSEC(1000));
}


int charge_counters_init(void)
{
        
    /* Initialize the charge counter workers */
    for (int i = 0; i < CHARGE_COUNTER_COUNT; i++) {
        struct charge_counter_t *counter = &charge_counter[i];
        
    	k_delayed_work_init(&counter->worker, charge_update_worker);
    }
}


void charge_counters_start(void)
{
    for (int i = 0; i < CHARGE_COUNTER_COUNT; i++) {
        k_delayed_work_submit(&charge_counter[i].worker, K_MSEC(1000));
    }
}


void handler_charge_counter(enum io_names_t pin_name)
{
    struct io_pins_t * io_pin = &io_pins[pin_name];
    int index = *io_pin->pdev - ioexp[0];
    if (index == 6) {
        index = 5;
    }
    struct charge_counter_t *counter = &charge_counter[index];
    bool polarity;
    
    int ret = read_io_pin(counter->polarity, &polarity);
    if (ret != 0) {
        return;
    }
    
    counter->raw_count += (polarity ? 1 : -1);
}
