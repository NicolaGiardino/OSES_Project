#include "w25q64.h"

int w25q64_init()
{
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
	}

	return 0;
}

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

	} while (buf[0] & 0x1);

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

	} while (buf[0] & 0x1);

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

static void w25q64_write(rt_uint8_t addr[3], rt_uint8_t dlen, rt_uint8_t *data)
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

static void w25q64_reset()
{
	/* Reset Enable */
	rt_uint8_t instr;
	struct rt_spi_message msg1, msg2;

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

void w25q64_control(rt_uint8_t instr, rt_uint8_t addr[3], rt_uint8_t dlen, rt_uint8_t *data)
{
	switch (instr)
	{
	case READ_SR1 || READ_SR2:
		w25q64_getSR(instr, data);
		break;
	case WRITE_SR:
		w25q64_writeSR(data);
		break;
	case PAGE_PROGRAM:
		w25q64_write(addr, dlen, data);
		break;

	case SECTOR_ERASE || BLOCK_ERASE || DBLOCK_ERASE:
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