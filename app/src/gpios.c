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
#include "app-handlers.h"
#include "app-devices.h"
#include "app-utils.h"

#define IOEXP_INST(x)   ioexp[x] = device_get_binding(DT_LABEL(DT_NODELABEL(ioexp##x)))

const struct device *porta;
const struct device *ioexp[IOEXP_COUNT];


static void interrupt_handler(const struct device *port,
	        				  struct gpio_callback *cb,
					          gpio_port_pins_t pins);


#define IO_ENTRY(label, dev, pin, io_flags, is_active_low, is_interrupt, interrupt_flags)       \
    {#label, false, (struct device **)(&dev), pin, io_flags, is_active_low, interrupt_flags, {},  \
     is_interrupt ? interrupt_handler : NULL, 0LL},  


FOR_ALL_IOS(struct io_pins_t io_pins[] = {, IO_ENTRY, };)

uint16_t io_count = NELEMENTS(io_pins);


void write_io_pin(enum io_names_t io_name, bool value) {
    struct io_pins_t *io_pin = &io_pins[io_name];
    
    if (!IS_OUTPUT(io_name)) {
        /* Not going to try to write an input, are you insane? */
        return;
    }

    /* value is always stored active high */
    io_pin->value = value;    
    
    if (io_pin->is_active_low) {
        value = !value;
    }
    gpio_pin_set(*io_pin->pdev, io_pin->pin, (int)value);
    io_pin->expiry = z_timeout_end_calc(K_MSEC(100));
}

int read_io_pin(enum io_names_t io_name, bool *outval) {
    struct io_pins_t *io_pin = &io_pins[io_name];
    const struct device *dev = *io_pins->pdev;
    uint32_t portval = 0;
    int ret;
    bool value;
    
    /* If this IO has been accessed in the last 100ms, we will not read the port */
    if (io_pin->expiry < k_uptime_ticks()) {
        /* We read the entire port in one shot */
        ret = gpio_port_get_raw(dev, &portval);
        if (ret != 0) {
            return ret;
        }
    
        uint64_t expiry = z_timeout_end_calc(K_MSEC(100));
    
        /* Find the first item in this device */
        int curr;
        for (curr = (int)io_name; curr != 0 && *io_pins[curr].pdev == dev; curr--);
        
        if (*io_pins[curr].pdev != dev) {
            curr++;
        }
        
        /* Now assign the values for every IO in this device from the port reading */
        for ( ; curr != io_count && *io_pins[curr].pdev == dev; curr++) {
            io_pin = &io_pins[curr];
            value = ((portval & BIT(io_pin->pin)) != 0);
            if (io_pin->is_active_low) {
                /* We store our values active high */
                value = !value;
            }
            io_pin->value = value;
            io_pin->expiry = expiry;
        }
    }
    
    /* And return the requested one */
    *outval = io_pins[io_name].value;
    return 0;
}

int gpios_init(void)
{
    porta = device_get_binding(DT_LABEL(DT_NODELABEL(porta)));
    FOR_EACH(IOEXP_INST, (;), 0, 1, 2, 3, 4, 5, 6);
    
    /* Initialize all IOs */
    for (int i = 0; i < io_count; i++) {
        struct io_pins_t *io_pin = &io_pins[i];
        
        int ret = gpio_pin_configure(*io_pin->pdev, io_pin->pin, io_pin->io_flags);
        if (ret != 0) {
            return ret;
        }
        
        if (IS_OUTPUT(i)) {
            /* Set all outputs to inactive logic level */
            write_io_pin(i, false);
        }
        
        if (IS_INPUT(i) && io_pin->handler) {
        	/* Prepare GPIO callback for interrupt pin */
        	gpio_init_callback(&io_pin->callback,
			                   io_pin->handler, BIT(io_pin->pin));
        	gpio_add_callback(*io_pin->pdev, &io_pin->callback);
        }
    }
    
    return 0;
}


static void interrupt_handler(const struct device *port,
	        				  struct gpio_callback *cb,
					          gpio_port_pins_t pins)
{
	struct io_pins_t * io_pin = CONTAINER_OF(
		cb, struct io_pins_t, callback);
    ARG_UNUSED(pins);
    
    if (*io_pin->pdev != port) {
        /* something's boogered */
        return;
    }

    int index = io_pin - &io_pins[0];
    enum io_names_t pin_name = (enum io_names_t)index;
    
    switch(pin_name) {
        case INT1:
        case INT2:
        case INT3:
        case INT4:
        case INT5:
        case INTO:
            handler_charge_counter(pin_name);
            break;
            
        case PWRGD1:
        case PWRGD2:
        case PWRGD3:
        case PWRGD4:
        case PWRGD5:
            handler_power_good(pin_name);
            break;
            
        case nSTANDBY:
        case nCHARGE:
            handler_charger(pin_name);
            break;

        case UP:
        case RIGHT:
        case LEFT:
        case DOWN:
        case ENTER:
        case ESC:
            handler_button(pin_name);
            break;
            
        default:
            break;
    }
}
