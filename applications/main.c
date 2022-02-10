/*
 * Copyright (c) 2006-2021, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author         Notes
 * 2021-01-28     flybreak       first version
 */

#include <rtthread.h>
#include <rtdevice.h>
#include "../Benchmark/XenoJetBench.c"

#define LED_PIN 25

int main(void)
{
    rt_kprintf("Hello, RT-Thread!\n");

    rt_pin_mode(LED_PIN, PIN_MODE_OUTPUT);

    rt_pin_mode(24, PIN_MODE_INPUT);

    int i = xeno_main();

    while (1)
    {
        if(rt_pin_read(24))
        {
            rt_pin_write(LED_PIN, 1);
            rt_thread_mdelay(1000);
        }
        else
        {
            rt_pin_write(LED_PIN, 0);
            rt_thread_mdelay(1000);
        }
    }
}

