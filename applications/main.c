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
#include "pico/stdlib.h"
#include "pico/binary_info.h"
#include "../Benchmark/XenoJetBench.h"

#define LED_PIN 25

static int addr = 0x68;

struct rt_i2c_bus_device *i2c_device;

static void mpu6050_reset()
{
    // Two byte reset. First byte register, second byte data
    // There are a load more options to set up the device in different ways that could be added here
    struct rt_i2c_msg msgs;
    msgs.buf[0] = 0x6B;
    msgs.buf[1] = 0x00;
    msgs.addr = addr;
    msgs.len = 2;
    msgs.flags = RT_I2C_WR;
    if (rt_i2c_transfer(i2c_device, &msgs, 1) != 1)
    {
        rt_kprintf("write fail!");
    }
}

static void mpu6050_read_raw(int16_t accel[3], int16_t gyro[3], int16_t *temp)
{
    // For this particular device, we send the device the register we want to read
    // first, then subsequently read from the device. The register is auto incrementing
    // so we don't need to keep sending the register we want, just the first.

    uint8_t buffer[6];

    // Start reading acceleration registers from register 0x3B for 6 bytes
    struct rt_i2c_msg msgs;
    msgs.buf[0] = 0x3B;
    msgs.addr = addr;
    msgs.len = 1;
    msgs.flags = RT_I2C_WR;
    if (rt_i2c_transfer(i2c_device, &msgs, 1) != 1)
    {
        rt_kprintf("write fail!");
    }
    msgs.addr = addr;
    msgs.len = 6;
    msgs.flags = RT_I2C_RD;
    if (rt_i2c_transfer(i2c_device, &msgs, 1) != 1)
    {
        rt_kprintf("read fail!");
    }

    for (int i = 0; i < 3; i++)
    {
        accel[i] = (buffer[i * 2] << 8 | buffer[(i * 2) + 1]);
    }

    // Now gyro data from reg 0x43 for 6 bytes
    // The register is auto incrementing on each read
    msgs.buf[0] = 0x43;
    msgs.addr = addr;
    msgs.len = 1;
    msgs.flags = RT_I2C_WR;
    if (rt_i2c_transfer(i2c_device, &msgs, 1) != 1)
    {
        rt_kprintf("write fail!");
    }
    msgs.addr = addr;
    msgs.len = 6;
    msgs.flags = RT_I2C_RD;
    if (rt_i2c_transfer(i2c_device, &msgs, 1) != 1)
    {
        rt_kprintf("read fail!");
    }

    for (int i = 0; i < 3; i++)
    {
        gyro[i] = (buffer[i * 2] << 8 | buffer[(i * 2) + 1]);
        ;
    }

    // Now temperature from reg 0x41 for 2 bytes
    // The register is auto incrementing on each read
    msgs.buf[0] = 0x41;
    msgs.addr = addr;
    msgs.len = 1;
    msgs.flags = RT_I2C_WR;
    if (rt_i2c_transfer(i2c_device, &msgs, 1) != 1)
    {
        rt_kprintf("write fail!");
    }
    msgs.addr = addr;
    msgs.len = 2;
    msgs.flags = RT_I2C_RD;
    if (rt_i2c_transfer(i2c_device, &msgs, 1) != 1)
    {
        rt_kprintf("read fail!");
    }

    *temp = buffer[0] << 8 | buffer[1];
}

int main(void)
{
    xeno_main();
    rt_kprintf("Hello, RT-Thread!\n");

    rt_pin_mode(LED_PIN, PIN_MODE_OUTPUT);

    stdio_init_all();
    printf("Hello, MPU6050! Reading raw data from registers...\n");

    i2c_device = rt_i2c_bus_device_find("i2c0");
    if (i2c_device == RT_NULL)
    {
        rt_kprintf("i2c bus device %s not found! ", "i2c0");
        return -RT_ENOSYS;
    }

    // This example will use I2C0 on the default SDA and SCL pins (4, 5 on a Pico)
    gpio_set_function(4, GPIO_FUNC_I2C);
    gpio_set_function(5, GPIO_FUNC_I2C);
    gpio_pull_up(4);
    gpio_pull_up(5);
    // Make the I2C pins available to picotool
    bi_decl(bi_2pins_with_func(4, 5, GPIO_FUNC_I2C));

    mpu6050_reset();

    int16_t acceleration[3], gyro[3], temp;

    while (1)
    {
        mpu6050_read_raw(acceleration, gyro, &temp);

        // These are the raw numbers from the chip, so will need tweaking to be really useful.
        // See the datasheet for more information
        rt_kprintf("Acc. X = %d, Y = %d, Z = %d\n", acceleration[0], acceleration[1], acceleration[2]);
        rt_kprintf("Gyro. X = %d, Y = %d, Z = %d\n", gyro[0], gyro[1], gyro[2]);
        // Temperature is simple so use the datasheet calculation to get deg C.
        // Note this is chip temperature.
        rt_kprintf("Temp. = %f\n", (temp / 340.0) + 36.53);

        rt_thread_mdelay(100);
    }
}
