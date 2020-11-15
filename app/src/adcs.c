/*
 * Copyright (c) 2020 Gavin Hurlbut
 *
 * SPDX-License-Identifier: Apache-2.0
 */
 
#include <zephyr.h>
#include <device.h>
#include <devicetree.h>
#include <drivers/adc.h>
#include <kernel.h>

#include "app-devices.h"
#include "app-utils.h"
#include "app-adcs.h"

#define ADC_INST(x)     adc[x] = device_get_binding(DT_LABEL(DT_NODELABEL(adc##x)))

const struct device *adc[ADC_COUNT];


#define ADC_ENTRY(label, dev, channel, reference_mv)                        \
    {#label, 0x0000, 0x0000, reference_mv, (struct device **)(&dev),        \
        {ADC_GAIN_1, ADC_REF_INTERNAL, ADC_ACQ_TIME_DEFAULT, channel, 1,}},


FOR_ALL_ADCS(struct adc_inputs_t adc_inputs[] = {, ADC_ENTRY, };)


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


int adcs_init(void)
{
    FOR_EACH(ADC_INST, (;), 0, 1, 2, 3, 4, 5);
 
    /* Initialize the ADC workers */
    for (int i = 0; i < ADC_COUNT; i++) {
        struct adc_work_t *worker = &adc_worker[i];

        worker->dev = adc[i];
    	k_delayed_work_init(&worker->worker, adc_read_worker);
    	worker->channel_mask = 0;
    }
    
    /* Initialize all ADC channels */
    for (int i = 0; i < adc_input_count; i++) {
        struct adc_inputs_t *adc_input = &adc_inputs[i];
        int index = *adc_input->pdev - adc[0];
        int channel = adc_input->config.channel_id;
        
        int ret = adc_channel_setup(*adc_input->pdev, &adc_input->config);
        if (ret != 0) {
            return ret;
        }
        
        adc_worker[index].channel_mask |= BIT(channel);
        adc_worker[index].inputs[channel] = (enum adc_input_names_t)i;
    }

    return 0;
}

void adcs_start(void)
{
    for (int i = 0; i < ADC_COUNT; i++) {
        k_delayed_work_submit(&adc_worker[i].worker, K_MSEC(1000));
    }
}