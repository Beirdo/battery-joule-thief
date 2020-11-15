/*
 * Copyright (c) 2020 Gavin Hurlbut
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <zephyr.h>
#include <device.h>
#include <devicetree.h>
#include <drivers/pwm.h>
#include <kernel.h>

#include "app-utils.h"
#include "app-gpios.h"
#include "app-handlers.h"
#include "app-input-batteries.h"

const struct device *pwm;

#define BAT_ENTRY(label, select, green, red, shutdown, channel)             \
    {#label, select, green, red, shutdown, channel, 0, false, {}, {}},
    
FOR_ALL_BATS(struct battery_worker_t battery_worker[] = {, BAT_ENTRY, };)

uint16_t battery_count = NELEMENTS(battery_worker);


static void battery_led_worker(struct k_work *work)
{
	struct battery_worker_t * battery = CONTAINER_OF(
		work, struct battery_worker_t, led_worker);
	bool red = false;
	bool green = false;
		
	if (battery->enabled) {
	    if (battery->power_good) {
	        green = true;
	    } else {
	        red = true;
	    }
	}
	
	write_io_pin(battery->green, green);
	write_io_pin(battery->red, red);
}


#define PWM_DEADSPACE 2     /* ~1us deadspace between each pulse, half before, half after */

static uint16_t current_pwm_mask;

static void battery_pwm_worker(struct k_work *work)
{
	uint16_t pwm_mask = 0;
    uint32_t on_time;
    uint32_t off_time;
	int count = 0;
	uint32_t timeslot = 0;
	int i;
	int ret;
	
	for (i = 0; i < battery_count; i++) {
	    struct battery_worker_t * battery = &battery_worker[i];
	    
	    if (battery->power_good && battery->enabled) {
	        pwm_mask |= BIT(battery->channel);
	        count++;
	    } else if ((current_pwm_mask & BIT(battery->channel)) != 0) {
	        /* This was on, and is now off.  Disable it. */
	        battery->enabled = false;
	    }
	    
	    write_io_pin(battery->shutdown, !(battery->enabled));
	}
	
	if (current_pwm_mask == pwm_mask) {
	    return;
	}
	
	current_pwm_mask = pwm_mask;
	
	/* The mask has changed.  Let's go for safety and shut them all off first */
	ret = pwm_pin_set_cycles(pwm, 0xFF, 4096, 0, PWM_FLAG_START_DELAY);
    if (ret != 0) {
        return;
    }

	if (count) {
    	timeslot = 4096 / count;
	}
	
	for (i = 0; i < battery_count / 2; i++) {
	    if ((pwm_mask & BIT(i)) != 0) {
	        on_time = PWM_DEADSPACE + (i * timeslot);
	        off_time = ((i + 1) * timeslot) - (2 * PWM_DEADSPACE);
	    } else {
	        on_time = 4096;
	        off_time = 0;
	    }
	    
	    ret = pwm_pin_set_cycles(pwm, battery_worker[2 * i].channel, on_time,
	            off_time, PWM_FLAG_START_DELAY);
	    if (ret != 0) {
	        return;
	    }
	}
}


int input_batteries_init(void)
{
    pwm = device_get_binding(DT_LABEL(DT_NODELABEL(pwm)));

    /* Initialize battery workers */
    for (int i = 0; i < battery_count; i++) {
        struct battery_worker_t *worker = &battery_worker[i];
        k_work_init(&worker->led_worker, battery_led_worker);
        k_work_init(&worker->pwm_worker, battery_pwm_worker);
    }

    return 0;
}



static int get_active_battery(const struct device *dev, enum battery_t *battery)
{
    int i;
    struct battery_worker_t *worker;
    int ret;
    bool select;
    
    for (i = 0; i < battery_count; i++) {
        worker = &battery_worker[i];
        if (*io_pins[worker->select].pdev != dev) {
            continue;
        }
        
        ret = read_io_pin(worker->select, &select);
        if (ret != 0) {
            return ret;
        }
        
        if (select) {
            *battery = (enum battery_t)i;
            return 0;
        }
    }
    
    return -EINVAL;
}


void handler_power_good(enum io_names_t pin_name)
{
    struct io_pins_t * io_pin = &io_pins[pin_name];
    int ret;
    
    enum battery_t battery;
    
    ret = get_active_battery(*io_pin->pdev, &battery);
    if (ret != 0) {
        return;
    }
    
    struct battery_worker_t * worker = &battery_worker[battery];

    ret = read_io_pin(pin_name, &worker->power_good);
    if (ret != 0) {
        return;
    }

    k_work_submit(&worker->led_worker);
    k_work_submit(&worker->pwm_worker);
}