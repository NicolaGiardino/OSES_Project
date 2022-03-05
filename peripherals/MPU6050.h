#ifndef MPU6050_H
#define MPU6050_H

#include "drv_soft_i2c.h"
#include <board.h>
#include <rtdevice.h>
#include <rtthread.h>

struct rt_i2c_bus_device *i2c_device;

void mpu6050_init(char *dev_name);

void mpu6050_reset();

void mpu6050_read_raw(int16_t accel[3], int16_t gyro[3], int16_t *temp);

#endif
