/*
 * Copyright (c) 2020 Gavin Hurlbut
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <zephyr.h>
#include <device.h>
#include <devicetree.h>
#include <drivers/gpio.h>
#include <drivers/adc.h>
#include <drivers/pwm.h>
#include <kernel.h>

#define IOEXP_COUNT 7
#define IOEXP_INST(x)   ioexp[x] = device_get_binding(DT_LABEL(DT_NODELABEL(ioexp##x)))

#define ADC_COUNT   6
#define ADC_INST(x)     adc[x] = device_get_binding(DT_LABEL(DT_NODELABEL(adc##x)))

const struct device *porta;
const struct device *ioexp[IOEXP_COUNT];
const struct device *adc[ADC_COUNT];
const struct device *display;
const struct device *pwm;


static void interrupt_handler(const struct device *port,
	        				  struct gpio_callback *cb,
					          gpio_port_pins_t pins);

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
                                                                            \
    x(nSD1, ioexp[0], 0, GPIO_OUTPUT_HIGH, true, 0, 0x00)                   \
    x(INT1, ioexp[0], 1, GPIO_INPUT, true, 1, GPIO_INT_EDGE_FALLING)        \
    x(POL1, ioexp[0], 2, GPIO_INPUT, false, 0, 0x00)                        \
    x(PWRGD1, ioexp[0], 3, GPIO_INPUT, false, 1, GPIO_INT_EDGE_BOTH)        \
    x(LED1ar, ioexp[0], 8, GPIO_OUTPUT_ACTIVE, 0, false, 0x00)              \
    x(LED1ag, ioexp[0], 9, GPIO_OUTPUT_ACTIVE, 0, false, 0x00)              \
    x(LED1br, ioexp[0], 10, GPIO_OUTPUT_ACTIVE, 0, false, 0x00)             \
    x(LED1bg, ioexp[0], 11, GPIO_OUTPUT_ACTIVE, 0, false, 0x00)             \
    x(BATSEL1a, ioexp[0], 12, GPIO_OUTPUT_ACTIVE, 0, false, 0x00)           \
    x(BATSEL1b, ioexp[0], 13, GPIO_OUTPUT_ACTIVE, 0, false, 0x00)           \
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

#define IO_ENTRY(label, dev, pin, io_flags, is_active_low, is_interrupt, interrupt_flags)       \
    {#label, false, (struct device **)(&dev), pin, io_flags, is_active_low, interrupt_flags, {},  \
     is_interrupt ? interrupt_handler : NULL, 0LL},  
#define IO_ENUM(label, ...) label,


FOR_ALL_IOS(enum io_names_t {, IO_ENUM, };)
FOR_ALL_IOS(static struct io_pins_t io_pins[] = {, IO_ENTRY, };)

#define NELEMENTS(x)    ((sizeof((x)) / sizeof((x)[0])))

uint16_t io_count = NELEMENTS(io_pins);

#define IS_INPUT(x)     ((io_pins[x].io_flags & GPIO_INPUT) == GPIO_INPUT)
#define IS_OUTPUT(x)    ((io_pins[x].io_flags & GPIO_OUTPUT) == GPIO_OUTPUT)

static void write_io_pin(enum io_names_t io_name, bool value) {
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

static int read_io_pin(enum io_names_t io_name, bool *outval) {
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

#define ADC_ENTRY(label, dev, channel, reference_mv)                        \
    {#label, 0x0000, 0x0000, reference_mv, (struct device **)(&dev),        \
        {ADC_GAIN_1, ADC_REF_INTERNAL, ADC_ACQ_TIME_DEFAULT, channel, 1,}},
#define ADC_ENUM(label, ...) label,


FOR_ALL_ADCS(enum adc_input_names_t {, ADC_ENUM, };)
FOR_ALL_ADCS(static struct adc_inputs_t adc_inputs[] = {, ADC_ENTRY, };)

uint16_t adc_input_count = NELEMENTS(adc_inputs);

struct adc_work_t {
    const struct device *dev;
    struct k_delayed_work worker;
    uint8_t channel_mask;
    enum adc_input_names_t inputs[4];
    struct adc_sequence sequence;
    uint16_t buffer[4];
};

struct adc_work_t adc_worker[ADC_COUNT];

static void adc_read_worker(struct k_work *work)
{
	struct adc_work_t * adc_worker = CONTAINER_OF(
		work, struct adc_work_t, worker);

    const struct device *dev = adc_worker->dev;
    struct adc_sequence *seq = &adc_worker->sequence;
    uint8_t channel_mask = adc_worker->channel_mask;
    uint16_t *buffer = adc_worker->buffer;
    size_t buflen = sizeof(adc_worker->buffer);
    enum adc_input_names_t input_name;
    struct adc_inputs_t *adc_input;
    uint32_t value;
    
    seq->options = NULL;
    seq->channels = channel_mask;
    seq->buffer = buffer;
    seq->buffer_size = buflen;
    seq->resolution = 16;
    seq->oversampling = 0;
    seq->calibrate = false;
    
    int ret = adc_read(dev, (const struct adc_sequence *)seq);
    if (ret == 0) {
        for(int i = 0; i < 4; i++) {
            if ((channel_mask & BIT(i)) != 0x00) {
                input_name = adc_worker->inputs[i];
                adc_input = &adc_inputs[input_name];
                value = buffer[i];
                adc_input->raw_value = value;
                ret = adc_raw_to_millivolts(adc_input->reference_mv,
                        adc_input->config.gain, 16, &value);
                if (ret != 0) {
                    value = 0x0000;
                }
                adc_input->value_mv = value;
            }
        }        
    }

    /* Schedule yourself for 1s out */
    k_delayed_work_submit((struct k_delayed_work *)work, K_MSEC(1000));
}


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


struct battery_worker_t {
    char *name;
    enum io_names_t select;
    enum io_names_t green;
    enum io_names_t red;
    enum io_names_t shutdown;
    uint8_t channel;
    bool power_good;
    bool enabled;
    struct k_work led_worker;
    struct k_work pwm_worker;
};


#define FOR_ALL_BATS(preamble, x, postamble)        \
preamble                                            \
    x(BATT1a, BATSEL1a, LED1ag, LED1ar, nSD1, 0)    \
    x(BATT1b, BATSEL1b, LED1bg, LED1br, nSD1, 0)    \
    x(BATT2a, BATSEL2a, LED2ag, LED2ar, nSD2, 1)    \
    x(BATT2b, BATSEL2b, LED2bg, LED2br, nSD2, 1)    \
    x(BATT3a, BATSEL3a, LED3ag, LED3ar, nSD3, 2)    \
    x(BATT3b, BATSEL3b, LED3bg, LED3br, nSD3, 2)    \
    x(BATT4a, BATSEL4a, LED4ag, LED4ar, nSD4, 3)    \
    x(BATT4b, BATSEL4b, LED4bg, LED4br, nSD4, 3)    \
    x(BATT5a, BATSEL5a, LED5ag, LED5ar, nSD5, 4)    \
    x(BATT5b, BATSEL5b, LED5bg, LED5br, nSD5, 4)    \
postamble

#define BAT_ENTRY(label, select, green, red, shutdown, channel)             \
    {#label, select, green, red, shutdown, channel, 0, false, {}, {}},
#define BAT_ENUM(label, ...) label,


FOR_ALL_BATS(enum battery_t {, BAT_ENUM, };)
FOR_ALL_BATS(static struct battery_worker_t battery_worker[] = {, BAT_ENTRY, };)

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

uint16_t current_pwm_mask;

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


void main(void)
{
    int i;
    int ret;

    /* Initialize all device mappings */
    porta = device_get_binding(DT_LABEL(DT_NODELABEL(porta)));
    display = device_get_binding(DT_LABEL(DT_NODELABEL(display)));
    pwm = device_get_binding(DT_LABEL(DT_NODELABEL(pwm)));
    
    FOR_EACH(IOEXP_INST, (;), 0, 1, 2, 3, 4, 5, 6);
    FOR_EACH(ADC_INST, (;), 0, 1, 2, 3, 4, 5);
    
    /* Initialize all IOs */
    for (i = 0; i < io_count; i++) {
        struct io_pins_t *io_pin = &io_pins[i];
        
        ret = gpio_pin_configure(*io_pin->pdev, io_pin->pin, io_pin->io_flags);
        if (ret != 0) {
            return;
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
    
    /* Initialize the ADC workers and charge counter workers */
    for (i = 0; i < ADC_COUNT; i++) {
        struct adc_work_t *worker = &adc_worker[i];
        struct charge_counter_t *counter = &charge_counter[i];
        
        worker->dev = adc[i];
    	k_delayed_work_init(&worker->worker, adc_read_worker);
    	worker->channel_mask = 0;
    	k_delayed_work_init(&counter->worker, charge_update_worker);
    }
    
    /* Initialize all ADC channels */
    for (i = 0; i < adc_input_count; i++) {
        struct adc_inputs_t *adc_input = &adc_inputs[i];
        int index = *adc_input->pdev - adc[0];
        int channel = adc_input->config.channel_id;
        
        ret = adc_channel_setup(*adc_input->pdev, &adc_input->config);
        if (ret != 0) {
            return;
        }
        
        adc_worker[index].channel_mask |= BIT(channel);
        adc_worker[index].inputs[channel] = (enum adc_input_names_t)i;
    }

    /* Initialize battery workers */
    for (i = 0; i < battery_count; i++) {
        struct battery_worker_t *worker = &battery_worker[i];
        k_work_init(&worker->led_worker, battery_led_worker);
        k_work_init(&worker->pwm_worker, battery_pwm_worker);
    }


    /* Initialize Display pages */

    /* Start ADC readings once a second (it reschedules itself) */
    for (i = 0; i < ADC_COUNT; i++) {
        k_delayed_work_submit(&adc_worker[i].worker, K_MSEC(1000));
    }
    
    /* Start the display update work item (it gets scheduled by buttons or 
     * ADC complete) 
     */
     
    /* Start the charge counters - once a second, reschedules itself */
    for (i = 0; i < ADC_COUNT; i++) {
        k_delayed_work_submit(&charge_counter[i].worker, K_MSEC(1000));
    }

    /* Start loop */
    while (1) {
        /* Check if batteries are depleted, disable those that are */
        
        /* If the enables have changed, reprogram the PWM phases */
        
        /* Sleep for 100ms */
        k_msleep(100);
    }
}


static void handler_charge_counter(enum io_names_t pin_name)
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


static void handler_power_good(enum io_names_t pin_name)
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


static void handler_charger(enum io_names_t pin_name)
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


static void handler_button(enum io_names_t pin_name)
{
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
