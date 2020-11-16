/*
 * Copyright (c) 2020 Gavin Hurlbut
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef __app_gpios_h_
#define __app_gpios_h_

#include <zephyr.h>
#include <device.h>
#include <devicetree.h>
#include <drivers/gpio.h>
#include <kernel.h>


struct io_pins_t {
    char *name;
    bool value;
    struct device **pdev;
    int pin;
    uint32_t io_flags;
    bool is_active_low;
    uint32_t interrupt_flags;
    struct gpio_callback callback;
    gpio_callback_handler_t handler;
    uint64_t expiry;
};



#define FOR_ALL_IOS(preamble, x, postamble)                                 \
preamble                                                                    \
    x(EXT_nINT, porta, 0, GPIO_INPUT, true, 0, GPIO_INT_EDGE_FALLING)       \
    x(nOE, porta, 1, GPIO_OUTPUT_HIGH, true, 0, 0x00)                       \
    x(LEDBootR, porta, 22, GPIO_OUTPUT_ACTIVE, false, 0, 0x00)              \
    x(LEDBootG, porta, 23, GPIO_OUTPUT_ACTIVE, false, 0, 0x00)              \
    x(LEDCPUg, porta, 24, GPIO_OUTPUT_ACTIVE, false, 0, 0x00)               \
    x(LEDCPUr, porta, 25, GPIO_OUTPUT_ACTIVE, false, 0, 0x00)               \
                                                                            \
    x(nSD1, ioexp[0], 0, GPIO_OUTPUT_HIGH, true, 0, 0x00)                   \
    x(INT1, ioexp[0], 1, GPIO_INPUT, true, 1, GPIO_INT_EDGE_FALLING)        \
    x(POL1, ioexp[0], 2, GPIO_INPUT, false, 0, 0x00)                        \
    x(PWRGD1, ioexp[0], 3, GPIO_INPUT, false, 1, GPIO_INT_EDGE_BOTH)        \
    x(LED1ar, ioexp[0], 8, GPIO_OUTPUT_ACTIVE, false, 0, 0x00)              \
    x(LED1ag, ioexp[0], 9, GPIO_OUTPUT_ACTIVE, false, 0, 0x00)              \
    x(LED1br, ioexp[0], 10, GPIO_OUTPUT_ACTIVE, false, 0, 0x00)             \
    x(LED1bg, ioexp[0], 11, GPIO_OUTPUT_ACTIVE, false, 0, 0x00)             \
    x(BATSEL1a, ioexp[0], 12, GPIO_OUTPUT_ACTIVE, false, 0, 0x00)           \
    x(BATSEL1b, ioexp[0], 13, GPIO_OUTPUT_ACTIVE, false, 0, 0x00)           \
                                                                            \
    x(nSD2, ioexp[1], 0, GPIO_OUTPUT_HIGH, true, 0, 0x00)                   \
    x(INT2, ioexp[1], 1, GPIO_INPUT, true, 1, GPIO_INT_EDGE_FALLING)        \
    x(POL2, ioexp[1], 2, GPIO_INPUT, false, 0, 0x00)                        \
    x(PWRGD2, ioexp[1], 3, GPIO_INPUT, false, 1, GPIO_INT_EDGE_BOTH)        \
    x(LED2ar, ioexp[1], 8, GPIO_OUTPUT_ACTIVE, false, 0, 0x00)              \
    x(LED2ag, ioexp[1], 9, GPIO_OUTPUT_ACTIVE, false, 0, 0x00)              \
    x(LED2br, ioexp[1], 10, GPIO_OUTPUT_ACTIVE, false, 0, 0x00)             \
    x(LED2bg, ioexp[1], 11, GPIO_OUTPUT_ACTIVE, false, 0, 0x00)             \
    x(BATSEL2a, ioexp[1], 12, GPIO_OUTPUT_ACTIVE, false, 0, 0x00)           \
    x(BATSEL2b, ioexp[1], 13, GPIO_OUTPUT_ACTIVE, false, 0, 0x00)           \
                                                                            \
    x(nSD3, ioexp[2], 0, GPIO_OUTPUT_HIGH, true, 0, 0x00)                   \
    x(INT3, ioexp[2], 1, GPIO_INPUT, true, 1, GPIO_INT_EDGE_FALLING)        \
    x(POL3, ioexp[2], 2, GPIO_INPUT, false, 0, 0x00)                        \
    x(PWRGD3, ioexp[2], 3, GPIO_INPUT, false, 1, GPIO_INT_EDGE_BOTH)        \
    x(LED3ar, ioexp[2], 8, GPIO_OUTPUT_ACTIVE, false, 0, 0x00)              \
    x(LED3ag, ioexp[2], 9, GPIO_OUTPUT_ACTIVE, false, 0, 0x00)              \
    x(LED3br, ioexp[2], 10, GPIO_OUTPUT_ACTIVE, false, 0, 0x00)             \
    x(LED3bg, ioexp[2], 11, GPIO_OUTPUT_ACTIVE, false, 0, 0x00)             \
    x(BATSEL3a, ioexp[2], 12, GPIO_OUTPUT_ACTIVE, false, 0, 0x00)           \
    x(BATSEL3b, ioexp[2], 13, GPIO_OUTPUT_ACTIVE, false, 0, 0x00)           \
                                                                            \
    x(nSD4, ioexp[3], 0, GPIO_OUTPUT_HIGH, true, 0, 0x00)                   \
    x(INT4, ioexp[3], 1, GPIO_INPUT, true, 1, GPIO_INT_EDGE_FALLING)        \
    x(POL4, ioexp[3], 2, GPIO_INPUT, false, 0, 0x00)                        \
    x(PWRGD4, ioexp[3], 3, GPIO_INPUT, false, 1, GPIO_INT_EDGE_BOTH)        \
    x(LED4ar, ioexp[3], 8, GPIO_OUTPUT_ACTIVE, false, 0, 0x00)              \
    x(LED4ag, ioexp[3], 9, GPIO_OUTPUT_ACTIVE, false, 0, 0x00)              \
    x(LED4br, ioexp[3], 10, GPIO_OUTPUT_ACTIVE, false, 0, 0x00)             \
    x(LED4bg, ioexp[3], 11, GPIO_OUTPUT_ACTIVE, false, 0, 0x00)             \
    x(BATSEL4a, ioexp[3], 12, GPIO_OUTPUT_ACTIVE, false, 0, 0x00)           \
    x(BATSEL4b, ioexp[3], 13, GPIO_OUTPUT_ACTIVE, false, 0, 0x00)           \
                                                                            \
    x(nSD5, ioexp[4], 0, GPIO_OUTPUT_HIGH, true, 0, 0x00)                   \
    x(INT5, ioexp[4], 1, GPIO_INPUT, true, 1, GPIO_INT_EDGE_FALLING)        \
    x(POL5, ioexp[4], 2, GPIO_INPUT, false, 0, 0x00)                        \
    x(PWRGD5, ioexp[4], 3, GPIO_INPUT, false, 1, GPIO_INT_EDGE_BOTH)        \
    x(LED5ar, ioexp[4], 8, GPIO_OUTPUT_ACTIVE, false, 0, 0x00)              \
    x(LED5ag, ioexp[4], 9, GPIO_OUTPUT_ACTIVE, false, 0, 0x00)              \
    x(LED5br, ioexp[4], 10, GPIO_OUTPUT_ACTIVE, false, 0, 0x00)             \
    x(LED5bg, ioexp[4], 11, GPIO_OUTPUT_ACTIVE, false, 0, 0x00)             \
    x(BATSEL5a, ioexp[4], 12, GPIO_OUTPUT_ACTIVE, false, 0, 0x00)           \
    x(BATSEL5b, ioexp[4], 13, GPIO_OUTPUT_ACTIVE, false, 0, 0x00)           \
                                                                            \
    x(BATT1_INT, ioexp[5], 0, GPIO_INPUT, true, 0, GPIO_INT_EDGE_FALLING)   \
    x(BATT2_INT, ioexp[5], 1, GPIO_INPUT, true, 0, GPIO_INT_EDGE_FALLING)   \
    x(BATT3_INT, ioexp[5], 2, GPIO_INPUT, true, 0, GPIO_INT_EDGE_FALLING)   \
    x(BATT4_INT, ioexp[5], 3, GPIO_INPUT, true, 0, GPIO_INT_EDGE_FALLING)   \
    x(BATT5_INT, ioexp[5], 4, GPIO_INPUT, true, 0, GPIO_INT_EDGE_FALLING)   \
    x(OUT_INT, ioexp[5], 5, GPIO_INPUT, true, 0, GPIO_INT_EDGE_FALLING)     \
    x(UP, ioexp[5], 8, GPIO_INPUT, true, 1, GPIO_INT_EDGE_FALLING)          \
    x(LEFT, ioexp[5], 9, GPIO_INPUT, true, 1, GPIO_INT_EDGE_FALLING)        \
    x(RIGHT, ioexp[5], 10, GPIO_INPUT, true, 1, GPIO_INT_EDGE_FALLING)      \
    x(DOWN, ioexp[5], 11, GPIO_INPUT, true, 1, GPIO_INT_EDGE_FALLING)       \
    x(ENTER, ioexp[5], 12, GPIO_INPUT, true, 1, GPIO_INT_EDGE_FALLING)      \
    x(ESC, ioexp[5], 13, GPIO_INPUT, true, 1, GPIO_INT_EDGE_FALLING)        \
                                                                            \
    x(nSDO, ioexp[6], 0, GPIO_OUTPUT_HIGH, true, 0, 0x00)                   \
    x(INTO, ioexp[6], 1, GPIO_INPUT, true, 1, GPIO_INT_EDGE_FALLING)        \
    x(POLO, ioexp[6], 2, GPIO_INPUT, false, 0, 0x00)                        \
    x(LEDActive, ioexp[6], 3, GPIO_OUTPUT_ACTIVE, false, 0, 0x00)           \
    x(nSTANDBY, ioexp[6], 8, GPIO_INPUT, true, 1, GPIO_INT_EDGE_BOTH)       \
    x(nCHARGE, ioexp[6], 9, GPIO_INPUT, true, 1, GPIO_INT_EDGE_BOTH)        \
    x(LEDOr, ioexp[6], 10, GPIO_OUTPUT_ACTIVE, false, 0, 0x00)              \
    x(LEDOg, ioexp[6], 11, GPIO_OUTPUT_ACTIVE, false, 0, 0x00)              \
postamble

#define IO_ENUM(label, ...) label,

FOR_ALL_IOS(enum io_names_t {, IO_ENUM, };)


extern struct io_pins_t io_pins[];
extern uint16_t io_count;

#define IS_INPUT(x)     ((io_pins[x].io_flags & GPIO_INPUT) == GPIO_INPUT)
#define IS_OUTPUT(x)    ((io_pins[x].io_flags & GPIO_OUTPUT) == GPIO_OUTPUT)

void write_io_pin(enum io_names_t io_name, bool value);
int read_io_pin(enum io_names_t io_name, bool *outval);
int gpios_init(void);

#endif /* __app_gpios_h_ */
