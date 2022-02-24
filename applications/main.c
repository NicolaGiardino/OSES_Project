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

/* defined the LED0 pin: PB7 */
#define LED_PIN GET_PIN(B, 14)
#define PUSHBUTTON_RAW GET_PIN(C, 13)
#define PUSHBUTTON_BENCH GET_PIN(C, 7)

int main(void) {
  rt_kprintf("Hello, RT-Thread!\n");


  int i = w25q64_init();
  if(i)
  {
      rt_kprintf("Error on spi device");
      return -RT_ERROR;
  }

  w25q64_control(CHIP_ERASE, RT_NULL, RT_NULL, RT_NULL);


  rt_pin_mode(LED_PIN, PIN_MODE_OUTPUT);

  rt_pin_mode(PUSHBUTTON_RAW, PIN_MODE_INPUT);

  rt_pin_attach_irq(PUSHBUTTON_RAW, PIN_IRQ_MODE_RISING,
                    (void *)&button_raw_async_handler, RT_NULL);

  rt_pin_irq_enable(PUSHBUTTON_RAW, PIN_IRQ_ENABLE);

  /*
  rt_pin_mode(PUSHBUTTON_BENCH, PIN_MODE_INPUT);

  rt_pin_attach_irq(PUSHBUTTON_BENCH, PIN_IRQ_MODE_RISING,
                    (void *)&button_bench_async_handler, RT_NULL);

  rt_pin_irq_enable(PUSHBUTTON_BENCH, PIN_IRQ_ENABLE);
  */

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
                 &producer_stack[0], sizeof(producer_stack), THREAD_PRIORITY,
                 THREAD_TIMESLICE);

  rt_thread_init(&consumer, "consumer", consumer_entry, RT_NULL,
                 &consumer_stack[0], sizeof(consumer_stack), THREAD_PRIORITY,
                 THREAD_TIMESLICE);

  rt_thread_startup(&producer);
  rt_thread_startup(&consumer);

  // while (1)
  //{

  // These are the raw numbers from the chip, so will need tweaking to be really
  // useful. See the datasheet for more information
  // rt_kprintf("Acc. X = %d, Y = %d, Z = %d\n", acceleration[0],
  // acceleration[1], acceleration[2]); rt_kprintf("Gyro. X = %d, Y = %d, Z =
  // %d\n", gyro[0], gyro[1], gyro[2]);
  // Temperature is simple so use the datasheet calculation to get deg C.
  // Note this is chip temperature.
  // rt_kprintf("Temp. = %d\n", (temp / 340.0) + 36.53);

  //}
}
