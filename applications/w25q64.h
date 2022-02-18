#ifndef W25Q64_H
#define W25Q64_H

#include <rtthread.h>
#include <rtdevice.h>
#include <board.h>
#include "dev_spi.h"

#define W25Q_SPI_DEVICE_NAME "w25q64"
#define W25Q_SPI_BUS_NAME "spi1"

#define WRITE_ENABLE    0x06
#define WRITE_DISABLE   0x04
#define READ_SR1        0x05
#define READ_SR2        0x35
#define WRITE_SR        0x01
#define PAGE_PROGRAM    0x02
#define SECTOR_ERASE    0x20
#define BLOCK_ERASE     0x52
#define DBLOCK_ERASE    0xD8
#define CHIP_ERASE      0x60
#define READ_DATA       0x03
#define JEDEC_ID        0x9F
#define RESET_ENABLE    0x66
#define RESET           0x99

struct rt_spi_device *spi_device;

static int rt_hw_spi_flash_init(void)
{
    rt_hw_spi_device_attach(W25Q_SPI_BUS_NAME, W25Q_SPI_DEVICE_NAME, GPIOA, GPIO_PIN_4);

    return RT_EOK;
}
INIT_COMPONENT_EXPORT(rt_hw_spi_flash_init);

int w25q64_init();
void w25q64_control(rt_uint8_t instr, rt_uint8_t addr[3], rt_uint8_t dlen, rt_uint8_t *data);

#endif