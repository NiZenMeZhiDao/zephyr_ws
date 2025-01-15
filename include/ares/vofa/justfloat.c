#include <stddef.h>
#include <stdint.h>
#include <zephyr/drivers/uart.h>
#include <zephyr/kernel.h>
#include "justfloat.h"
#include "zephyr/kernel/thread.h"
#include <string.h>
#include <stdlib.h>

// LOG_MODULE_REGISTER(vofa, LOG_LEVEL_DBG);

static void jf_send_float(struct JFData *data)
{
	const struct device *uart_dev = data->uart_dev;

	for (int i = 0; i < data->channel; i++) {
		if (data->fdata[i] == *(float *)tail) {
			data->fdata[i] = 1e+6;
		}
	}

	if (data->fdata[data->channel] != *(float *)tail) {
		memcpy(&(data->fdata[data->channel]), tail, 4 * sizeof(uint8_t));
	}

	uart_tx(uart_dev, (const uint8_t *)data->fdata, data->channel * 4 + 4, SYS_FOREVER_US);
}

/* JustFloat Feedback to console*/
static K_THREAD_STACK_DEFINE(jf_stack_area, 768); // 定义线程栈
static struct k_thread jf_thread_data;

static void jf_feedback(void *arg1, void *arg2, void *arg3)
{
	int delay = (int)arg1;
	struct JFData *data = (struct JFData *)arg2;

	while (1) {
		k_msleep(delay);
		if (data->channel == 0) {
			continue;
		}
		for (int i = 0; i < data->channel; i++) {
			switch (data->types[i]) {
			case PTR_INT:
				data->fdata[i] = *(int *)data->data_ptr[i];
				break;
			case PTR_FLOAT:
				data->fdata[i] = *(float *)data->data_ptr[i];
				break;
			case PTR_DOUBLE:
				data->fdata[i] = *(double *)data->data_ptr[i];
				break;
			case PTR_INT8:
				data->fdata[i] = *(int8_t *)data->data_ptr[i];
				break;
			case PTR_INT16:
				data->fdata[i] = *(int16_t *)data->data_ptr[i];
				break;
			default:
				break;
			}
		}
		jf_send_float(data);
	}
}

void jf_channel_add(struct JFData *data, void *value, enum JF_Types type)
{
	if (data->channel < 24) {
		if (type == RAW) {
			data->fdata[data->channel] = *(float *)value;
		} else {
			data->fdata[data->channel] = *(float *)value;
			data->data_ptr[data->channel] = value;
		}
	}
	data->types[data->channel] = type;
	data->channel++;
	data->types[data->channel] = RAW;
	memcpy(&(data->fdata[data->channel]), tail, 4 * sizeof(uint8_t));
}

struct JFData *jf_send_init(const struct device *uart_dev, int delay)
{
	int channel = 24;
	aresPlotData.channel = 0;
	if (!device_is_ready(uart_dev)) {
		return NULL;
	}
	aresPlotData.uart_dev = (struct device *)uart_dev;
	memcpy(&(aresPlotData.fdata[aresPlotData.channel]), tail, 4 * sizeof(uint8_t));
	aresPlotData.types[aresPlotData.channel] = RAW;
	/* Start JustFloat thread*/
	k_thread_create(&jf_thread_data, jf_stack_area, K_THREAD_STACK_SIZEOF(jf_stack_area),
			jf_feedback, (void *)delay, &aresPlotData, NULL, -1, 0, K_NO_WAIT);
	return &aresPlotData;
}