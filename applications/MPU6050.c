#include "MPU6050.h"

static int addr = 0x68;

/**
 * This function initializes the I2C device for the accelerometer
 *
 * @param NULL parameter passed to the function
 *
 * @return void
 */
void mpu6050_init(char *dev_name)
{
    i2c_device = rt_i2c_bus_device_find(dev_name);
    if (i2c_device == RT_NULL)
    {
        rt_kprintf("i2c bus device %s not found! ", dev_name);
    }
}

/**
 * This function is used to reset the mpu6050, must be called at startup
 *
 * @param NULL parameter passed to the function
 *
 * @return void
 */
void mpu6050_reset()
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

/**
 * This function is used to read raw data from the accelerometer via I2C
 *
 * @param accelerometer raw data
 * @param gyroscope raw data
 * @param temperature raw data
 *
 * @return void
 */
void mpu6050_read_raw(int16_t accel[3], int16_t gyro[3], int16_t *temp)
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
