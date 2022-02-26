/*
 * Copyright (c) 2006-2018, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2019-09-17     tyustli   first version
 */

#include "procedure.h"
#include <string.h>

/* defined the LED0 pin: PB7 */
#define LED_PIN GET_PIN(B, 14)
#define PUSHBUTTON_RAW GET_PIN(C, 13)
#define PUSHBUTTON_BENCH GET_PIN(C, 7)

int main(void) {
  rt_kprintf("Hello, RT-Thread!\n");

  if (w25q64_init()) {
    rt_kprintf("Error on spi device");
    return -RT_ERROR;
  }

  //w25q64_control(CHIP_ERASE, RT_NULL, RT_NULL, RT_NULL);

  rt_pin_mode(LED_PIN, PIN_MODE_OUTPUT);

#if USE_DEFERR

  strcpy(raw[0].name, "raw write");
  raw[0].thd_fun = write_raw_mem_async;
  raw[0].args = RT_NULL;

  strcpy(raw[1].name, "raw read");
  raw[1].thd_fun = read_raw_mem_async;
  raw[1].args = RT_NULL;

  strcpy(bench[0].name, "bench write");
  bench[0].thd_fun = write_bench_mem_async;
  bench[0].args = RT_NULL;

  strcpy(bench[1].name, "bench read");
  bench[1].thd_fun = read_bench_mem_async;
  bench[1].args = RT_NULL;

  rt_thread_deferrable_init(1000, 1000, THREAD_PRIORITY - 3);

#endif

  rt_pin_mode(PUSHBUTTON_RAW, PIN_MODE_INPUT);

  rt_pin_attach_irq(PUSHBUTTON_RAW, PIN_IRQ_MODE_RISING,
                    (void *)&button_raw_async_handler, RT_NULL);

  rt_pin_irq_enable(PUSHBUTTON_RAW, PIN_IRQ_ENABLE);

  rt_pin_mode(PUSHBUTTON_BENCH, PIN_MODE_INPUT);

  rt_pin_attach_irq(PUSHBUTTON_BENCH, PIN_IRQ_MODE_RISING,
                    (void *)&button_bench_async_handler, RT_NULL);

  rt_pin_irq_enable(PUSHBUTTON_BENCH, PIN_IRQ_ENABLE);

  raw_mutex = rt_mutex_create("raw_mutex", RT_IPC_FLAG_FIFO);
  if (raw_mutex == RT_NULL) {
    rt_kprintf("create raw mutex failed.\n");
    return -1;
  }

  bench_mutex = rt_mutex_create("bench_mutex", RT_IPC_FLAG_FIFO);
  if (bench_mutex == RT_NULL) {
    rt_kprintf("create bench mutex failed.\n");
    return -1;
  }

  rt_thread_init(&producer, "producer", producer_entry, RT_NULL,
                 &producer_stack[0], sizeof(producer_stack), THREAD_PRIORITY - 1,
                 THREAD_TIMESLICE);

  rt_thread_init(&consumer, "consumer", consumer_entry, RT_NULL,
                 &consumer_stack[0], sizeof(consumer_stack), THREAD_PRIORITY,
                 THREAD_TIMESLICE);

  rt_thread_startup(&producer);
  rt_thread_startup(&consumer);

  // while (1)
  //{}
}
