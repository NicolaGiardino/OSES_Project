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

/* defined the LED0 pin: PB7 */
#define LED_PIN    GET_PIN(B, 14)

#define W25Q_SPI_DEVICE_NAME "spi10"
#define W25Q_SPI_BUS_NAME "spi1"

struct rt_spi_device *spi_device;

static int rt_hw_spi_flash_init(void)
{
    rt_hw_spi_device_attach(W25Q_SPI_BUS_NAME, W25Q_SPI_DEVICE_NAME, GPIOA, GPIO_PIN_4);

    return RT_EOK;
}
INIT_COMPONENT_EXPORT(rt_hw_spi_flash_init);

static void mpu6050_reset()
{
    // Two byte reset. First byte register, second byte data
    // There are a load more options to set up the device in different ways that could be added here
    uint8_t buffer[2] = {0x6B, 0x00};
    struct rt_i2c_msg msgs;
    msgs.buf = buffer;
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
    buffer[0] = 0x3B;
    msgs.buf = buffer;
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
        accel[i] = (msgs.buf[i * 2] << 8 | msgs.buf[(i * 2) + 1]);
    }

    // Now gyro data from reg 0x43 for 6 bytes
    // The register is auto incrementing on each read
    buffer[0] = 0x43;
    msgs.buf = buffer;
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
        gyro[i] = (msgs.buf[i * 2] << 8 | msgs.buf[(i * 2) + 1]);
    }

    // Now temperature from reg 0x41 for 2 bytes
    // The register is auto incrementing on each read
    buffer[0] = 0x41;
    msgs.buf = buffer;
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

    *temp = msgs.buf[0] << 8 | msgs.buf[1];
}

int main(void)
{
    rt_kprintf("Hello, RT-Thread!\n");

    rt_pin_mode(LED_PIN, PIN_MODE_OUTPUT);

    rt_kprintf("Hello, MPU6050! Reading raw data from registers...\n");

    rt_uint8_t w25x_read_id = 0x90;
    rt_uint8_t id[5] = {0};
    rt_uint8_t id2[5] = {0};
    struct rt_spi_message msg1, msg2, msg3;
    spi_device = (struct rt_spi_device *)rt_device_find("spi10");
    if (!spi_device)
    {
        rt_kprintf("spi sample run failed! can't find %s device!\n", "spi10");
        return -RT_ENOSYS;
    }
    else
    {

        struct rt_spi_configuration cfg;
        cfg.data_width = 8;
        cfg.mode = RT_SPI_MASTER | RT_SPI_MODE_0 | RT_SPI_MSB;
        cfg.max_hz = 1 * 1000 * 1000;
        rt_spi_configure(spi_device, &cfg);

        /* Method 1: Send the command to read the ID using rt_spi_send_then_recv() */
        //rt_spi_send_then_recv(spi_device, &w25x_read_id, 1, id, 5);
        //rt_kprintf("use rt_spi_send_then_recv() read w25q ID is:%x%x\n", id[3], id[4]);

        /* Method 2: Send the command to read the ID using rt_spi_transfer_message() */


        msg1.send_buf   = &w25x_read_id;
        msg1.recv_buf   = RT_NULL;
        msg1.length     = 1;
        msg1.cs_take    = 1;
        msg1.cs_release = 0;
        msg1.next       = &msg2;

        msg2.send_buf   = RT_NULL;
        msg2.recv_buf   = id;
        msg2.length     = 5;
        msg2.cs_take    = 0;
        msg2.cs_release = 1;
        msg2.next       = RT_NULL;

        rt_spi_transfer_message(spi_device, &msg1);
        rt_kprintf("use rt_spi_transfer_message() read w25q ID is:%x%x\n", id[3], id[4]);
        /* Chip Erase
        w25x_read_id = 0x06;
        msg1.send_buf   = &w25x_read_id;
        msg1.recv_buf   = RT_NULL;
        msg1.length     = 1;
        msg1.cs_take    = 1;
        msg1.cs_release = 1;
        msg1.next       = RT_NULL;

        rt_spi_transfer_message(spi_device, &msg1);

        w25x_read_id = 0x60;
        msg2.send_buf   = &w25x_read_id;
        msg2.recv_buf   = RT_NULL;
        msg2.length     = 1;
        msg2.cs_take    = 1;
        msg2.cs_release = 1;
        msg2.next       = RT_NULL;

        rt_spi_transfer_message(spi_device, &msg2);
        do
        {
            w25x_read_id = 0x05;
            msg1.send_buf   = &w25x_read_id;
            msg1.recv_buf   = RT_NULL;
            msg1.length     = 1;
            msg1.cs_take    = 1;
            msg1.cs_release = 0;
            msg1.next       = &msg2;

            msg2.send_buf   = RT_NULL;
            msg2.recv_buf   = id;
            msg2.length     = 1;
            msg2.cs_take    = 0;
            msg2.cs_release = 1;
            msg2.next       = RT_NULL;

            rt_spi_transfer_message(spi_device, &msg1);

            rt_kprintf("Received Status Register is %d\n", id[0]);

        }while(id[0] & 0x1);

        */
   }
    rt_thread_mdelay(2000);
    /* Write Enable */

    w25x_read_id = 0x06;
    msg1.send_buf   = &w25x_read_id;
    msg1.recv_buf   = RT_NULL;
    msg1.length     = 1;
    msg1.cs_take    = 1;
    msg1.cs_release = 1;
    msg1.next       = RT_NULL;

    rt_spi_transfer_message(spi_device, &msg1);

    w25x_read_id = 0x05;
    msg1.send_buf   = &w25x_read_id;
    msg1.recv_buf   = RT_NULL;
    msg1.length     = 1;
    msg1.cs_take    = 1;
    msg1.cs_release = 0;
    msg1.next       = &msg2;

    msg2.send_buf   = RT_NULL;
    msg2.recv_buf   = id;
    msg2.length     = 1;
    msg2.cs_take    = 0;
    msg2.cs_release = 1;
    msg2.next       = RT_NULL;

    rt_spi_transfer_message(spi_device, &msg1);

    rt_kprintf("Received Status Register is %d\n", id[0]);

    /* Write data to addr in msg2.buf, data is in msg3.buf */
    w25x_read_id = 0x02;
    msg1.send_buf   = &w25x_read_id;
    msg1.recv_buf   = RT_NULL;
    msg1.length     = 1;
    msg1.cs_take    = 1;
    msg1.cs_release = 0;
    msg1.next       = &msg2;

    id2[0] = 0x00;
    id2[1] = 0x00;
    id2[2] = 0x00;
    msg2.send_buf   = id2;
    msg2.recv_buf   = RT_NULL;
    msg2.length     = 3;
    msg2.cs_take    = 0;
    msg2.cs_release = 0;
    msg2.next       = &msg3;

    id[0] = 0x12;
    id[1] = 0x13;
    id[2] = 0x1f;
    msg3.send_buf   = id;
    msg3.recv_buf   = RT_NULL;
    msg3.length     = 3;
    msg3.cs_take    = 0;
    msg3.cs_release = 1;
    msg3.next       = RT_NULL;

    rt_spi_transfer_message(spi_device, &msg1);

    w25x_read_id = 0x04;
    msg1.send_buf   = &w25x_read_id;
    msg1.recv_buf   = RT_NULL;
    msg1.length     = 1;
    msg1.cs_take    = 1;
    msg1.cs_release = 1;
    msg1.next       = RT_NULL;

    rt_spi_transfer_message(spi_device, &msg1);

    rt_kprintf("Data sent, waiting\n");
    rt_thread_mdelay(3000);

    /* Read data from addr in msg2.buf, receive on msg3.buf */
    w25x_read_id = 0x03;
    msg1.send_buf   = &w25x_read_id;
    msg1.recv_buf   = RT_NULL;
    msg1.length     = 1;
    msg1.cs_take    = 1;
    msg1.cs_release = 0;
    msg1.next       = &msg2;

    id2[0] = 0x00;
    id2[1] = 0x00;
    id2[2] = 0x00;
    msg2.send_buf   = id2;
    msg2.recv_buf   = RT_NULL;
    msg2.length     = 3;
    msg2.cs_take    = 0;
    msg2.cs_release = 0;
    msg2.next       = &msg3;

    msg3.send_buf   = RT_NULL;
    msg3.recv_buf   = id;
    msg3.length     = 3;
    msg3.cs_take    = 0;
    msg3.cs_release = 1;
    msg3.next       = RT_NULL;

    rt_spi_transfer_message(spi_device, &msg1);

    rt_kprintf("Data received: %d %d %d\n", id[0], id[1], id[2]);

    i2c_device = rt_i2c_bus_device_find("i2c1");
    if (i2c_device == RT_NULL)
    {
        rt_kprintf("i2c bus device %s not found! ", "i2c1");
        return -RT_ENOSYS;
    }
    //mpu6050_reset();

    int16_t acceleration[3], gyro[3], temp;

    while (1)
    {
        /*mpu6050_read_raw(acceleration, gyro, &temp);
        w25x_read_id = 0x06;
        msg1.send_buf   = &w25x_read_id;
        msg1.recv_buf   = RT_NULL;
        msg1.length     = 1;
        msg1.cs_take    = 1;
        msg1.cs_release = 0;
        msg1.next       = &msg2;

        msg2.send_buf   = RT_NULL;
        msg2.recv_buf   = id;
        msg2.length     = 5;
        msg2.cs_take    = 0;
        msg2.cs_release = 1;
        msg2.next       = RT_NULL;
*/
        //rt_spi_transfer_message(spi_device, &msg1);
        //rt_kprintf("use rt_spi_transfer_message() read w25q ID is:%x%x\n", id[3], id[4]);


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
