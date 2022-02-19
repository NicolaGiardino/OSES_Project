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
#define PUSHBUTTON GET_PIN(C, 6)

#define THREAD_PRIORITY     20
#define THREAD_TIMESLICE    10

rt_mutex_t mutex;
static int16_t acc[3], gyro[3], temp;

static struct rt_thread producer;
static char producer_stack[1024];
static void producer_entry(void* parameter)
{
    mpu6050_init("i2c1");
    mpu6050_reset();

    while(1)
    {
        rt_mutex_take(mutex, RT_WAITING_FOREVER);

        mpu6050_read_raw(acc, gyro, &temp);

        rt_mutex_release(mutex);

        rt_thread_mdelay(1000);

    }

}



static struct rt_thread consumer;
static char consumer_stack[1024];
static void consumer_entry(void* parameter)
{

    static rt_uint32_t addr = 0x00;
    static uint8_t addr8[3];

    static rt_uint8_t acc8[6], gyro8[6], temp8[2];

    int i;
    i = w25q64_init();
    if(i)
    {
        rt_kprintf("Error on i2c device");
        rt_thread_suspend(&consumer);
    }

    w25q64_control(CHIP_ERASE, RT_NULL, RT_NULL, RT_NULL);

    while(1)
    {
        rt_mutex_take(mutex, RT_WAITING_FOREVER);

        int j = 0;

        for(i = 0; i < 3; i++)
        {
            acc8[j] = (rt_uint8_t)acc[i];
            gyro8[j] = (rt_uint8_t)gyro[i];
            j++;
            acc8[j] = (rt_uint8_t)(acc[i] << 8);
            gyro8[j] = (rt_uint8_t)(gyro[i] << 8);
            j++;
        }

        temp8[0] = (rt_uint8_t)temp;
        temp8[1] = (rt_uint8_t)(temp << 8);

        addr8[0] = (rt_uint8_t)addr;
        addr8[1] = (rt_uint8_t)(addr << 8);
        addr8[2] = (rt_uint8_t)(addr << 16);
        w25q64_control(PAGE_PROGRAM, addr8, 3, acc8);

        addr += 0x03;
        addr8[0] = (rt_uint8_t)addr;
        addr8[1] = (rt_uint8_t)(addr << 8);
        addr8[2] = (rt_uint8_t)(addr << 16);
        w25q64_control(PAGE_PROGRAM, addr8, 3, gyro8);

        addr8[0] = (rt_uint8_t)addr;
        addr8[1] = (rt_uint8_t)(addr << 8);
        addr8[2] = (rt_uint8_t)(addr << 16);
        addr += 0x03;
        w25q64_control(PAGE_PROGRAM, addr8, 1, temp8);

        addr += 0x01;

        rt_mutex_release(mutex);
    }

}

int main(void)
{
    rt_kprintf("Hello, RT-Thread!\n");

    rt_pin_mode(LED_PIN, PIN_MODE_OUTPUT);

    rt_pin_mode(PUSHBUTTON, PIN_MODE_INPUT);

    mutex = rt_mutex_create("mutex", RT_IPC_FLAG_FIFO);
    if (mutex == RT_NULL)
    {
        rt_kprintf("create mutex failed.\n");
        return -1;
    }

    rt_thread_init(&producer, "producer", producer_entry, RT_NULL, &producer_stack[0], sizeof(producer_stack), THREAD_PRIORITY, THREAD_TIMESLICE);

    rt_thread_init(&consumer, "consumer", consumer_entry, RT_NULL, &consumer_stack, sizeof(consumer_stack), THREAD_PRIORITY, THREAD_TIMESLICE);


    rt_thread_startup(&producer);
    rt_thread_startup(&consumer);

    while (1)
    {

        // These are the raw numbers from the chip, so will need tweaking to be really useful.
        // See the datasheet for more information
        //rt_kprintf("Acc. X = %d, Y = %d, Z = %d\n", acceleration[0], acceleration[1], acceleration[2]);
        //rt_kprintf("Gyro. X = %d, Y = %d, Z = %d\n", gyro[0], gyro[1], gyro[2]);
        // Temperature is simple so use the datasheet calculation to get deg C.
        // Note this is chip temperature.
        //rt_kprintf("Temp. = %d\n", (temp / 340.0) + 36.53);

    }
}
