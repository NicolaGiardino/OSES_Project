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

int main(void)
{
    rt_uint32_t i;
    rt_kprintf("Hello, RT-Thread!\n");

    mpu6050_init("i2c1");
    mpu6050_reset();

    if (w25q64_init())
    {
        rt_kprintf("Error on spi device");
        return -RT_ERROR;
    }

    w25q64_control(CHIP_ERASE, RT_NULL, RT_NULL, RT_NULL);

    if(!BMP280_init("i2c1"))
    {
        rt_kprintf("Err on init\n");
        return 1;
    }

    if (!BMP280_Check())
    {
        rt_kprintf("NONE\r\nHALTING\r\n");
    }
    i = BMP280_GetMode();
    rt_kprintf("Mode: [%02X] ", i);
    switch (i)
    {
        case BMP280_MODE_SLEEP:
            rt_kprintf("SLEEP\r\n");
            break;
        case BMP280_MODE_FORCED:
        case BMP280_MODE_FORCED2:
            rt_kprintf("FORCED\r\n");
            break;
        default:
            rt_kprintf("NORMAL\r\n");
            break;
    }

    // Reset BMP280 chip
    rt_kprintf("BMP280 chip reset: ");
    BMP280_Reset();

    rt_thread_mdelay(1000);

    // Read calibration values
    rt_kprintf("Calibration: ");
    i = BMP280_Read_Calibration();
    rt_kprintf("[%02X] %s\r\n", i, (i == BMP280_SUCCESS) ? "OK" : "FAIL");

    // Set normal mode inactive duration (standby time)
    BMP280_SetStandby(BMP280_STBY_0p5ms);

    // Set IIR filter constant
    BMP280_SetFilter(BMP280_FILTER_OFF);

    // Set oversampling for temperature
    BMP280_SetOSRST(BMP280_OSRS_T_x1);

    // Set oversampling for pressure
    BMP280_SetOSRSP(BMP280_OSRS_P_x2);

    // Set normal mode (perpetual periodic conversion)
    BMP280_SetMode(BMP280_MODE_NORMAL);

    // Chip working mode
    i = BMP280_GetMode();
    rt_kprintf("Mode: [%02X] -> ", i);
    switch (i)
    {
        case BMP280_MODE_SLEEP:
            rt_kprintf("SLEEP\r\n");
            break;
        case BMP280_MODE_FORCED:
        case BMP280_MODE_FORCED2:
            rt_kprintf("FORCED\r\n");
            break;
        default:
            rt_kprintf("NORMAL\r\n");
            break;
    }


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

    rt_thread_deferrable_init(rt_tick_from_millisecond(1000), rt_tick_from_millisecond(205000), THREAD_PRIORITY_DEF);
#endif

    rt_pin_mode(LED_PIN, PIN_MODE_OUTPUT);

    rt_pin_mode(PUSHBUTTON_RAW, PIN_MODE_INPUT);

    rt_pin_attach_irq(PUSHBUTTON_RAW, PIN_IRQ_MODE_RISING,
                    (void *)&button_raw_async_handler, RT_NULL);

    rt_pin_irq_enable(PUSHBUTTON_RAW, PIN_IRQ_ENABLE);

    rt_pin_mode(PUSHBUTTON_BENCH, PIN_MODE_INPUT);

    rt_pin_attach_irq(PUSHBUTTON_BENCH, PIN_IRQ_MODE_RISING,
                    (void *)&button_bench_async_handler, RT_NULL);

    rt_pin_irq_enable(PUSHBUTTON_BENCH, PIN_IRQ_ENABLE);

    raw_mutex = rt_mutex_create("raw_mutex", RT_IPC_FLAG_FIFO);
    if (raw_mutex == RT_NULL)
    {
        rt_kprintf("create raw mutex failed.\n");
        return -1;
    }

    bench_mutex = rt_mutex_create("bench_mutex", RT_IPC_FLAG_FIFO);
    if (bench_mutex == RT_NULL)
    {
        rt_kprintf("create bench mutex failed.\n");
        return -1;
    }

    rt_thread_init(&producer, "producer", producer_entry, RT_NULL,
                 &producer_stack[0], sizeof(producer_stack), THREAD_PRIORITY - 10,
                 THREAD_TIMESLICE);

    rt_thread_init(&consumer, "consumer", consumer_entry, RT_NULL,
                 &consumer_stack[0], sizeof(consumer_stack), THREAD_PRIORITY,
                 THREAD_TIMESLICE);

    rt_thread_startup(&producer);
    rt_thread_startup(&consumer);



  //while (1)
  //{}

}
