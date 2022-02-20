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

#define DEBUG 0

/* defined the LED0 pin: PB7 */
#define LED_PIN    GET_PIN(B, 14)
#define PUSHBUTTON GET_PIN(C, 13)

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

#if DEBUG
    rt_kprintf("Init MPU6050 done\n");
#endif

    while(1)
    {
        rt_mutex_take(mutex, RT_WAITING_FOREVER);

        mpu6050_read_raw(acc, gyro, &temp);

#if DEBUG
        rt_kprintf("Acc. X = %d, Y = %d, Z = %d\n", acc[0], acc[1], acc[2]);
        rt_kprintf("Gyro. X = %d, Y = %d, Z = %d\n", gyro[0], gyro[1], gyro[2]);
        rt_kprintf("Temp. = %d\n", (rt_int16_t)((temp / 340.0) + 36.53));
#endif

        rt_mutex_release(mutex);

        rt_thread_mdelay(3000);

    }

}



static struct rt_thread consumer;
static char consumer_stack[1024];
static void consumer_entry(void* parameter)
{

    while(1){}

}

void write_mem_async()
{
    static rt_uint32_t addr = 0x00;
    static uint8_t addr8[3];

    static rt_uint8_t acc8[6], gyro8[6], temp8[2];

    int i;

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
    w25q64_control(PAGE_PROGRAM, addr8, 6, acc8);
#if DEBUG
    rt_kprintf("Wrote acc data on FLASH\n");
#endif

    addr += 0x03;
    addr8[0] = (rt_uint8_t)addr;
    addr8[1] = (rt_uint8_t)(addr << 8);
    addr8[2] = (rt_uint8_t)(addr << 16);
    w25q64_control(PAGE_PROGRAM, addr8, 6, gyro8);
#if DEBUG
    rt_kprintf("Wrote gyro data on FLASH\n");
#endif

    addr8[0] = (rt_uint8_t)addr;
    addr8[1] = (rt_uint8_t)(addr << 8);
    addr8[2] = (rt_uint8_t)(addr << 16);
    addr += 0x03;
    w25q64_control(PAGE_PROGRAM, addr8, 2, temp8);
#if DEBUG
    rt_kprintf("Wrote temp data on FLASH\n");
#endif

    addr += 0x01;

    rt_thread_mdelay(1000);

    rt_uint8_t a[3] = {0x00, 0x00, 0x00};
    rt_uint8_t rd[28];
    w25q64_control(READ_DATA, a, 28, rd);

#if DEBUG
    for(i = 0; i < 28; i+=2)
    {
        rt_kprintf("%d\t", (int16_t)(rd[i]) + ((int16_t)(rd[i + 1]) >> 8));
    }
    rt_kprintf("\n");
#endif

    rt_mutex_release(mutex);
}

void turn_on_led_irq()
{
    static int led = 0;

#if DEBUG
    rt_kprintf("IRQ\n");
#endif

    rt_pin_write(LED_PIN, led);

    led = ! led;
}

int main(void)
{
    rt_kprintf("Hello, RT-Thread!\n");

    /*int i = w25q64_init();
    if(i)
    {
        rt_kprintf("Error on spi device");
        return -RT_ERROR;
    }

    w25q64_control(CHIP_ERASE, RT_NULL, RT_NULL, RT_NULL);
*/
    rt_pin_mode(LED_PIN, PIN_MODE_OUTPUT);

    rt_pin_mode(PUSHBUTTON, PIN_MODE_INPUT);

    rt_pin_attach_irq(PUSHBUTTON, PIN_IRQ_MODE_RISING, (void*)&turn_on_led_irq, RT_NULL);

    rt_pin_irq_enable(PUSHBUTTON, PIN_IRQ_ENABLE);

    mutex = rt_mutex_create("mutex", RT_IPC_FLAG_FIFO);
    if (mutex == RT_NULL)
    {
        rt_kprintf("create mutex failed.\n");
        return -1;
    }

    rt_thread_init(&producer, "producer", producer_entry, RT_NULL, &producer_stack[0], sizeof(producer_stack), THREAD_PRIORITY, THREAD_TIMESLICE);

    rt_thread_init(&consumer, "consumer", consumer_entry, RT_NULL, &consumer_stack[0], sizeof(consumer_stack), THREAD_PRIORITY, THREAD_TIMESLICE);


    rt_thread_startup(&producer);
    rt_thread_startup(&consumer);

    //while (1)
    //{

        // These are the raw numbers from the chip, so will need tweaking to be really useful.
        // See the datasheet for more information
        //rt_kprintf("Acc. X = %d, Y = %d, Z = %d\n", acceleration[0], acceleration[1], acceleration[2]);
        //rt_kprintf("Gyro. X = %d, Y = %d, Z = %d\n", gyro[0], gyro[1], gyro[2]);
        // Temperature is simple so use the datasheet calculation to get deg C.
        // Note this is chip temperature.
        //rt_kprintf("Temp. = %d\n", (temp / 340.0) + 36.53);

    //}
}
