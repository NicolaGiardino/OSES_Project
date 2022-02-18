/*
 * Copyright (c) 2006-2018, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2019-09-17     tyustli   first version
 */

#include <rtthread.h>
#include <rtdevice.h>
#include <board.h>
#include "MPU6050.h"
#include "w25q64.h"

/* defined the LED0 pin: PB7 */
#define LED_PIN    GET_PIN(B, 14)

int main(void)
{
    rt_kprintf("Hello, RT-Thread!\n");

    rt_pin_mode(LED_PIN, PIN_MODE_OUTPUT);


    //mpu6050_reset();

    int16_t acceleration[3], gyro[3], temp;

    while (1)
    {

        // These are the raw numbers from the chip, so will need tweaking to be really useful.
        // See the datasheet for more information
        //rt_kprintf("Acc. X = %d, Y = %d, Z = %d\n", acceleration[0], acceleration[1], acceleration[2]);
        //rt_kprintf("Gyro. X = %d, Y = %d, Z = %d\n", gyro[0], gyro[1], gyro[2]);
        // Temperature is simple so use the datasheet calculation to get deg C.
        // Note this is chip temperature.
        //rt_kprintf("Temp. = %d\n", (temp / 340.0) + 36.53);

        rt_thread_mdelay(100);
    }
}
