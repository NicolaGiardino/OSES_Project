#include "w25q64.h"

/**
 * This function initializes the spi device of the FLASH
 * It is called by the kernel at init time
 *
 * @param NULL parameter passed to the function
 *
 * @return void
 */
static int rt_hw_spi_flash_init(void)
{
	rt_hw_spi_device_attach(W25Q_SPI_BUS_NAME, W25Q_SPI_DEVICE_NAME, GPIOA,
							GPIO_PIN_4);

	return RT_EOK;
}
INIT_COMPONENT_EXPORT(rt_hw_spi_flash_init);

/**
 * This function initializes the FLASH, getting the device
 * and configuring the SPI
 *
 * @param NULL parameter passed to the function
 *
 * @return void
 */
int w25q64_init()
{
	spi_device = (struct rt_spi_device *)rt_device_find("w25q64");
	if (!spi_device)
	{
		rt_kprintf("spi sample run failed! can't find %s device!\n", "w25q64");
		return -RT_ENOSYS;
	}
	else
	{
		struct rt_spi_configuration cfg;
		cfg.data_width = 8;
		cfg.mode = RT_SPI_MASTER | RT_SPI_MODE_0 | RT_SPI_MSB;
		cfg.max_hz = 1 * 1000 * 1000;
		rt_spi_configure(spi_device, &cfg);
	}

	return 0;
}

/**
 * This function is used to get the device ID
 *
 * @param the return value (the ID)
 *
 * @return void
 */
static void w25q64_getJEDEC(rt_uint8_t data[3])
{
	rt_uint8_t instr;
	struct rt_spi_message msg1, msg2;

	instr = JEDEC_ID;
	msg1.send_buf = &instr;
	msg1.recv_buf = RT_NULL;
	msg1.length = 1;
	msg1.cs_take = 1;
	msg1.cs_release = 0;
	msg1.next = &msg2;

	msg2.send_buf = RT_NULL;
	msg2.recv_buf = data;
	msg2.length = 3;
	msg2.cs_take = 0;
	msg2.cs_release = 1;
	msg2.next = RT_NULL;

	rt_spi_transfer_message(spi_device, &msg1);
}

/**
 * This function is used to get the Status Register
 *
 * @param The instruction to see which of the two SRs must be read
 * @param The returned SR
 *
 * @return void
 */
static void w25q64_getSR(rt_uint8_t instr, rt_uint8_t *data)
{

	struct rt_spi_message msg1, msg2;

	msg1.send_buf = &instr;
	msg1.recv_buf = RT_NULL;
	msg1.length = 1;
	msg1.cs_take = 1;
	msg1.cs_release = 0;
	msg1.next = &msg2;

	msg2.send_buf = RT_NULL;
	msg2.recv_buf = &data;
	msg2.length = 1;
	msg2.cs_take = 0;
	msg2.cs_release = 1;
	msg2.next = RT_NULL;

	rt_spi_transfer_message(spi_device, &msg1);
}

/**
 * This function is used to write the Status Register
 *
 * @param The read SRs
 *
 * @return void
 */
static void w25q64_writeSR(rt_uint8_t data[2])
{
	/* Write Enable */
	rt_uint8_t instr;
	struct rt_spi_message msg1, msg2;

	instr = WRITE_ENABLE;
	msg1.send_buf = &instr;
	msg1.recv_buf = RT_NULL;
	msg1.length = 1;
	msg1.cs_take = 1;
	msg1.cs_release = 1;
	msg1.next = RT_NULL;

	rt_spi_transfer_message(spi_device, &msg1);

	/* Write data to addr in msg2.buf, data is in msg3.buf */
	instr = WRITE_SR;
	msg1.send_buf = &instr;
	msg1.recv_buf = RT_NULL;
	msg1.length = 1;
	msg1.cs_take = 1;
	msg1.cs_release = 0;
	msg1.next = &msg2;

	msg2.send_buf = data;
	msg2.recv_buf = RT_NULL;
	msg2.length = 2;
	msg2.cs_take = 0;
	msg2.cs_release = 1;
	msg2.next = RT_NULL;

	rt_spi_transfer_message(spi_device, &msg1);

	/* Write disable */
	instr = WRITE_DISABLE;
	msg1.send_buf = &instr;
	msg1.recv_buf = RT_NULL;
	msg1.length = 1;
	msg1.cs_take = 1;
	msg1.cs_release = 1;
	msg1.next = RT_NULL;

	rt_spi_transfer_message(spi_device, &msg1);
}

/**
 * This function is used to erase the chip
 *
 * @param NULL parameter passed to the function
 *
 * @return void
 */
static void w25q64_erase()
{
	rt_uint8_t instr, buf;
	struct rt_spi_message msg1, msg2;

	/* Write enable */
	instr = WRITE_ENABLE;
	msg1.send_buf = &instr;
	msg1.recv_buf = RT_NULL;
	msg1.length = 1;
	msg1.cs_take = 1;
	msg1.cs_release = 1;
	msg1.next = RT_NULL;

	rt_spi_transfer_message(spi_device, &msg1);

	/* Chip Erase */
	instr = CHIP_ERASE;
	msg2.send_buf = &instr;
	msg2.recv_buf = RT_NULL;
	msg2.length = 1;
	msg2.cs_take = 1;
	msg2.cs_release = 1;
	msg2.next = RT_NULL;

	rt_spi_transfer_message(spi_device, &msg2);

	/* Check finish erase */
	do
	{
		instr = READ_SR1;
		msg1.send_buf = &instr;
		msg1.recv_buf = RT_NULL;
		msg1.length = 1;
		msg1.cs_take = 1;
		msg1.cs_release = 0;
		msg1.next = &msg2;

		msg2.send_buf = RT_NULL;
		msg2.recv_buf = &buf;
		msg2.length = 1;
		msg2.cs_take = 0;
		msg2.cs_release = 1;
		msg2.next = RT_NULL;

		rt_spi_transfer_message(spi_device, &msg1);

	} while (buf & 0x1);

	/* Write disable */
	instr = WRITE_DISABLE;
	msg1.send_buf = &instr;
	msg1.recv_buf = RT_NULL;
	msg1.length = 1;
	msg1.cs_take = 1;
	msg1.cs_release = 1;
	msg1.next = RT_NULL;

	rt_spi_transfer_message(spi_device, &msg1);
}

/**
 * This function is used to erase a block or a sector of the chip
 *
 * @param The instruction that defines how much memory to erase
 * @param The address from which to start erasing
 *
 * @return void
 */
static void w25q64_erase_part(rt_uint8_t instr, rt_uint8_t addr[3])
{

	rt_uint8_t instrc, buf;
	struct rt_spi_message msg1, msg2;

	/* Write enable */
	instrc = WRITE_ENABLE;
	msg1.send_buf = &instrc;
	msg1.recv_buf = RT_NULL;
	msg1.length = 1;
	msg1.cs_take = 1;
	msg1.cs_release = 1;
	msg1.next = RT_NULL;

	rt_spi_transfer_message(spi_device, &msg1);

	/* Block/Sector Erase */
	msg1.send_buf = &instr;
	msg1.recv_buf = RT_NULL;
	msg1.length = 1;
	msg1.cs_take = 1;
	msg1.cs_release = 0;
	msg1.next = &msg2;

	msg2.send_buf = addr;
	msg2.recv_buf = RT_NULL;
	msg2.length = 3;
	msg2.cs_take = 0;
	msg2.cs_release = 1;
	msg2.next = RT_NULL;

	rt_spi_transfer_message(spi_device, &msg2);

	/* Check finish erase */
	do
	{
		instrc = READ_SR1;
		msg1.send_buf = &instrc;
		msg1.recv_buf = RT_NULL;
		msg1.length = 1;
		msg1.cs_take = 1;
		msg1.cs_release = 0;
		msg1.next = &msg2;

		msg2.send_buf = RT_NULL;
		msg2.recv_buf = &buf;
		msg2.length = 1;
		msg2.cs_take = 0;
		msg2.cs_release = 1;
		msg2.next = RT_NULL;

		rt_spi_transfer_message(spi_device, &msg1);

	} while (buf & 0x1);

	/* Write disable */
	instrc = WRITE_DISABLE;
	msg1.send_buf = &instrc;
	msg1.recv_buf = RT_NULL;
	msg1.length = 1;
	msg1.cs_take = 1;
	msg1.cs_release = 1;
	msg1.next = RT_NULL;

	rt_spi_transfer_message(spi_device, &msg1);
}

/**
 * This function is used to write on the FLASH on a given address
 *
 * @param The address to write to
 * @param The data length
 * @param The data to be written
 *
 * @return void
 */
static void w25q64_write(rt_uint8_t addr[3], rt_uint8_t dlen,
						 rt_uint8_t *data)
{
	/* Write Enable */
	rt_uint8_t instr;
	struct rt_spi_message msg1, msg2, msg3;

	instr = WRITE_ENABLE;
	msg1.send_buf = &instr;
	msg1.recv_buf = RT_NULL;
	msg1.length = 1;
	msg1.cs_take = 1;
	msg1.cs_release = 1;
	msg1.next = RT_NULL;

	rt_spi_transfer_message(spi_device, &msg1);

	/* Write data to addr in msg2.buf, data is in msg3.buf */
	instr = PAGE_PROGRAM;
	msg1.send_buf = &instr;
	msg1.recv_buf = RT_NULL;
	msg1.length = 1;
	msg1.cs_take = 1;
	msg1.cs_release = 0;
	msg1.next = &msg2;

	msg2.send_buf = addr;
	msg2.recv_buf = RT_NULL;
	msg2.length = 3;
	msg2.cs_take = 0;
	msg2.cs_release = 0;
	msg2.next = &msg3;

	msg3.send_buf = data;
	msg3.recv_buf = RT_NULL;
	msg3.length = dlen;
	msg3.cs_take = 0;
	msg3.cs_release = 1;
	msg3.next = RT_NULL;

	rt_spi_transfer_message(spi_device, &msg1);

	/* Write disable */
	instr = WRITE_DISABLE;
	msg1.send_buf = &instr;
	msg1.recv_buf = RT_NULL;
	msg1.length = 1;
	msg1.cs_take = 1;
	msg1.cs_release = 1;
	msg1.next = RT_NULL;

	rt_spi_transfer_message(spi_device, &msg1);
}

/**
 * This function is used to read from the FLASH from a given address
 *
 * @param The address from which to read
 * @param How many bytes to read
 * @param The returned read data
 *
 * @return void
 */
static void w25q64_read(rt_uint8_t addr[3], rt_uint8_t dlen, rt_uint8_t *data)
{
	rt_uint8_t instr;
	struct rt_spi_message msg1, msg2, msg3;

	/* Read data from addr in msg2.buf, receive on msg3.buf */
	instr = READ_DATA;
	msg1.send_buf = &instr;
	msg1.recv_buf = RT_NULL;
	msg1.length = 1;
	msg1.cs_take = 1;
	msg1.cs_release = 0;
	msg1.next = &msg2;

	msg2.send_buf = addr;
	msg2.recv_buf = RT_NULL;
	msg2.length = 3;
	msg2.cs_take = 0;
	msg2.cs_release = 0;
	msg2.next = &msg3;

	msg3.send_buf = RT_NULL;
	msg3.recv_buf = data;
	msg3.length = dlen;
	msg3.cs_take = 0;
	msg3.cs_release = 1;
	msg3.next = RT_NULL;

	rt_spi_transfer_message(spi_device, &msg1);
}

/**
 * This function is used to reset the chip
 *
 * @param NULL parameter passed to the function
 *
 * @return void
 */
static void w25q64_reset()
{
	/* Reset Enable */
	rt_uint8_t instr;
	struct rt_spi_message msg1;

	instr = RESET_ENABLE;
	msg1.send_buf = &instr;
	msg1.recv_buf = RT_NULL;
	msg1.length = 1;
	msg1.cs_take = 1;
	msg1.cs_release = 1;
	msg1.next = RT_NULL;

	rt_spi_transfer_message(spi_device, &msg1);

	/* Reset */
	instr = RESET;
	msg1.send_buf = &instr;
	msg1.recv_buf = RT_NULL;
	msg1.length = 1;
	msg1.cs_take = 1;
	msg1.cs_release = 1;
	msg1.next = RT_NULL;

	rt_spi_transfer_message(spi_device, &msg1);
}

/**
 * This function is the only public function
 * it is used to control the chip by using the defined instruction
 * This then calls the relative function based on the input command
 *
 * @param The instruction to switch to the right command
 * @param The address, if needed, to use
 * @param The data lenght, if needed
 * @param The data to pass/receive, if needed
 *
 * @return void
 */
void w25q64_control(rt_uint8_t instr, rt_uint8_t addr[3], rt_uint8_t dlen,
					rt_uint8_t *data)
{
	switch (instr)
	{
	case READ_SR1:
	case READ_SR2:
		w25q64_getSR(instr, data);
		break;
	case WRITE_SR:
		w25q64_writeSR(data);
		break;
	case PAGE_PROGRAM:
		w25q64_write(addr, dlen, data);
		break;

	case SECTOR_ERASE:
	case BLOCK_ERASE:
	case DBLOCK_ERASE:
		w25q64_erase_part(instr, addr);
		break;

	case CHIP_ERASE:
		w25q64_erase();
		break;

	case READ_DATA:
		w25q64_read(addr, dlen, data);
		break;

	case JEDEC_ID:
		w25q64_getJEDEC(data);
		break;

	case RESET:
		w25q64_reset();
		break;

	default:
		break;
	}
}
