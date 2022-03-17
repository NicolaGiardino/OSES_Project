#include "XenoJetBench.h"
#include "procedure.h"

static rt_uint32_t addr = 0x00;
static rt_uint32_t addr_bench = 0x800000;
static size_t curr_read = 0;
static int16_t acc[3], gyro[3], temp;

/**
 * This function is called to write mpu and bmp data on the FLASH,
 * it erases the chip if the last position was written
 *
 * @param NULL parameter passed to the function
 *
 * @return void
 */
void write_raw_mem_async()
{
	static uint8_t addr8[3];
	static rt_uint8_t data8[12];
	static int i = 0;
	rt_kprintf("Inside IRQ\n");

#if COUNT
	rt_tick_t init, end;
	rt_int32_t total;
	init = rt_tick_get_millisecond();
#endif

	if (addr >= 0x7FFFF4)
	{
		w25q64_control(CHIP_ERASE, RT_NULL, RT_NULL, RT_NULL);
		addr = 0x00;
	}

	rt_mutex_take(raw_mutex, RT_WAITING_FOREVER);

    data8[0] = (rt_uint8_t)acc_v[i];
    data8[1] = (rt_uint8_t)((rt_uint32_t)acc_v[i] >> 8);
    data8[2] = (rt_uint8_t)((rt_uint32_t)acc_v[i] >> 16);
    data8[3] = (rt_uint8_t)((rt_uint32_t)acc_v[i] >> 24);

    data8[4] = (rt_uint8_t)gyro_v[i];
    data8[5] = (rt_uint8_t)((rt_uint32_t)gyro_v[i] >> 8);
    data8[6] = (rt_uint8_t)((rt_uint32_t)gyro_v[i] >> 16);
    data8[7] = (rt_uint8_t)((rt_uint32_t)gyro_v[i] >> 24);

    data8[8] = (rt_uint8_t)baro_v[i];
    data8[9] = (rt_uint8_t)((rt_uint32_t)baro_v[i] >> 8);
    data8[10] = (rt_uint8_t)((rt_uint32_t)baro_v[i] >> 16);
    data8[11] = (rt_uint8_t)((rt_uint32_t)baro_v[i] >> 24);

	addr8[0] = (rt_uint8_t)addr;
	addr8[1] = (rt_uint8_t)(addr >> 8);
	addr8[2] = (rt_uint8_t)(addr >> 16);
	w25q64_control(PAGE_PROGRAM, addr8, 12, data8);

#if DEBUG
	rt_kprintf("Wrote raw data on FLASH\n");
#endif

	addr += 12;

	i = (i + 1) % NUM_READINGS;
	rt_mutex_release(raw_mutex);

#if COUNT
	end = rt_tick_get_millisecond();
	total = (int)(end - init);
	rt_kprintf("Total time IRQ: %d\n", total);
#endif
}

/**
 * This function is called to read mpu and bmp data from the FLASH
 * from the last written position
 *
 * @param NULL parameter passed to the function
 *
 * @return void
 */
void read_raw_mem_async()
{
#if COUNT
	rt_tick_t init, end;
	rt_int32_t total;
	init = rt_tick_get_millisecond();
#endif

	rt_mutex_take(raw_mutex, RT_WAITING_FOREVER);

	rt_uint32_t addr32;
	rt_uint8_t addr8[3] = {0x00, 0x00, 0x00};
	rt_uint8_t rd[12];

	addr32 = addr - 12;

	addr8[0] = (rt_uint8_t)addr32;
	addr8[1] = (rt_uint8_t)(addr32 >> 8);
	addr8[2] = (rt_uint8_t)(addr32 >> 16);
	w25q64_control(READ_DATA, addr8, 12, rd);

#if DEBUG
	int i;
	float read;

	for (i = 0; i < 12; i += 4)
	{
		read = (float)(rd[i]) + ((float)((int)(rd[i + 1]) << 8)) +
			   ((float)((int)(rd[i + 2]) << 16)) +
			   ((float)((int)(rd[i + 3]) << 24));
		rt_kprintf("%d\t", (int)read);
	}
	rt_kprintf("\n");
#endif

	rt_mutex_release(raw_mutex);
#if COUNT
	end = rt_tick_get_millisecond();
	total = (int)(end - init);
	rt_kprintf("Total time IRQ: %d\n", total);
#endif
}

/**
 * This function is the IRQ handler for the 1st button
 * if DefServ is active, it adds a function to its queue
 * otherwise calls the write/read function alternatively
 *
 * @param NULL parameter passed to the function
 *
 * @return void
 */
void button_raw_async_handler()
{
	static uint8_t RWn =
		0; /* alternate between writing (first) and reading (after) */

#if DEBUG
	rt_kprintf("IRQ happened\n");
#endif

#if USE_DEFERR
	RWn ? rt_thread_deferrable_insert_task(&raw[1])
		: rt_thread_deferrable_insert_task(&raw[0]);
#else
	if (RWn)
	{
		rt_thread_init(&read_mem_raw, "read raw", read_raw_mem_async, RT_NULL,
					   &mem_raw[1][0], sizeof(mem_raw[1]), THREAD_PRIORITY_DEF,
					   THREAD_TIMESLICE);
		rt_thread_startup(&read_mem_raw);
	}
	else
	{
		rt_thread_init(&write_mem_raw, "write raw", write_raw_mem_async, RT_NULL,
					   &mem_raw[0][0], sizeof(mem_raw[0]), THREAD_PRIORITY_DEF,
					   THREAD_TIMESLICE);
		rt_thread_startup(&write_mem_raw); /* perform asynchronous action */
	}
#endif

	RWn ^= 1; /* invert selection */
}

/**
 * This function is called to write bench data on the FLASH,
 * it erases the chip if the last position was written
 *
 * @param NULL parameter passed to the function
 *
 * @return void
 */
void write_bench_mem_async()
{
	static uint8_t addr8[3];
	static rt_uint8_t res8[56];

	rt_kprintf("Inside IRQ\n");

#if COUNT
	rt_tick_t init, end;
	rt_int32_t total;
	init = rt_tick_get_millisecond();
#endif

	if (addr_bench >= 0xFFFFC8)
	{
		w25q64_control(CHIP_ERASE, RT_NULL, RT_NULL, RT_NULL);
		addr_bench = 0x800000;
	}

	rt_mutex_take(bench_mutex, RT_WAITING_FOREVER);

	int i, j = 0;

	for (i = 0; i < 56; i++)
	{
		res8[i] = (rt_uint8_t)results[j];
		res8[++i] = (rt_uint8_t)((rt_uint32_t)results[j] >> 8);
		res8[++i] = (rt_uint8_t)((rt_uint32_t)results[j] >> 16);
		res8[++i] = (rt_uint8_t)((rt_uint32_t)results[j] >> 24);
		j++;
	}

	addr8[0] = (rt_uint8_t)addr_bench;
	addr8[1] = (rt_uint8_t)(addr_bench >> 8);
	addr8[2] = (rt_uint8_t)(addr_bench >> 16);
	w25q64_control(PAGE_PROGRAM, addr8, 56, res8);

	addr_bench += 0x38;

#if DEBUG
	rt_kprintf("Wrote res data on FLASH\n");
#endif

	rt_mutex_release(bench_mutex);

#if COUNT
	end = rt_tick_get_millisecond();
	total = (int)(end - init);
	rt_kprintf("Total time IRQ: %d\n", total);
#endif
}

/**
 * This function is called to read bench data from the FLASH
 * from the last written position
 *
 * @param NULL parameter passed to the function
 *
 * @return void
 */
void read_bench_mem_async()
{
#if COUNT
	rt_tick_t init, end;
	rt_int32_t total;
	init = rt_tick_get_millisecond();
#endif
	rt_mutex_take(bench_mutex, RT_WAITING_FOREVER);

	rt_uint32_t addr32;
	rt_uint8_t addr8[3] = {0x00, 0x00, 0x00};
	rt_uint8_t rd[56];

	addr32 = addr_bench - 0x38;

	addr8[0] = (rt_uint8_t)addr32;
	addr8[1] = (rt_uint8_t)(addr32 >> 8);
	addr8[2] = (rt_uint8_t)(addr32 >> 16);
	w25q64_control(READ_DATA, addr8, 56, rd);

#if DEBUG
	int i;
	float read;

	for (i = 0; i < 56; i += 4)
	{
		read = (float)(rd[i]) + ((float)((int)(rd[i + 1]) << 8)) +
			   ((float)((int)(rd[i + 2]) << 16)) +
			   ((float)((int)(rd[i + 3]) << 24));
		rt_kprintf("%d\t", (int)read);
	}
	rt_kprintf("\n");
#endif

	rt_mutex_release(bench_mutex);
#if COUNT
	end = rt_tick_get_millisecond();
	total = (int)(end - init);
	rt_kprintf("Total time IRQ: %d\n", total);
#endif
}

/**
 * This function is the IRQ handler for the 2nd  button
 * if DefServ is active, it adds a function to its queue
 * otherwise calls the write/read function alternatively
 *
 * @param NULL parameter passed to the function
 *
 * @return void
 */
void button_bench_async_handler()
{
	static uint8_t RWn =
		0; /* alternate between writing (first) and reading (after) */
#if USE_DEFERR
	RWn ? rt_thread_deferrable_insert_task(&bench[1])
		: rt_thread_deferrable_insert_task(&bench[0]);
#else
	if (RWn)
	{
		rt_thread_init(&read_mem_bench, "read bench", read_bench_mem_async, RT_NULL,
					   &mem_bench[1][0], sizeof(mem_bench[1]), THREAD_PRIORITY_DEF,
					   THREAD_TIMESLICE);

		rt_thread_startup(&read_mem_bench);
	}
	else
	{
		rt_thread_init(&write_mem_bench, "write bench", write_bench_mem_async,
					   RT_NULL, &mem_bench[0][0], sizeof(mem_bench[0]),
					   THREAD_PRIORITY_DEF, THREAD_TIMESLICE);
		rt_thread_startup(&write_mem_bench); /* perform asynchronous action */
	}
#endif

#if DEBUG
	rt_kprintf("IRQ happened\n");
#endif

	RWn ^= 1; /* invert selection */
}

/**
 * This function is the entry for the producer
 * it reads NUM_READINGS time from the accelerometer and barometer
 * taking hold of a MUTEX on each reading
 *
 * @param NULL parameter passed to the function
 *
 * @return void
 */
void producer_entry(void *parameter)
{

	rt_uint32_t i;

	// RAW temperature and pressure values
	rt_int32_t UT, UP;

	// Human-readable temperature and pressure
	rt_uint32_t pressure;

#if DEBUG
	rt_kprintf("Init MPU6050 done\n");
#endif

#if COUNT
	rt_uint32_t init_time, end_time;
	rt_int32_t time_taken;
#endif
	while (1)
	{

		rt_kprintf("Start sampling\n");

#if COUNT
		init_time = rt_tick_get_millisecond();
#endif

		for (curr_read = 0; curr_read < NUM_READINGS; curr_read++)
		{
			rt_mutex_take(raw_mutex, RT_WAITING_FOREVER);

			mpu6050_read_raw(acc, gyro, &temp);
			acc_v[curr_read] =
				sqroot((float)(acc[0] * acc[0] + acc[1] * acc[1] + acc[2] * acc[2]));
			gyro_v[curr_read] = sqroot(
				(float)(gyro[0] * gyro[0] + gyro[1] * gyro[1] + gyro[2] * gyro[2]));

#if DEBUG
			rt_kprintf("Acc. X = %d, Y = %d, Z = %d\n", acc[0], acc[1], acc[2]);
			rt_kprintf("Gyro. X = %d, Y = %d, Z = %d\n", gyro[0], gyro[1], gyro[2]);
			rt_kprintf("Acc. mod = %d\n", (int)acc_v[curr_read]);
			rt_kprintf("Gyro. mod = %d\n", (int)gyro_v[curr_read]);
#endif
			// Get raw readings from the chip
			i = BMP280_Read_UTP(&UT, &UP);
#if DEBUG
			rt_kprintf("Raw: T=0x%05X P=0x%05X [R=%s]\r\n", UT, UP,
					   i ? "OK" : "ERROR");
#endif

			if (UT == 0x80000)
			{
#if DEBUG
				// Either temperature measurement is configured as 'skip' or first
				// conversion is not completed yet
				rt_kprintf("Temperature: no data\r\n");
				// There is no sense to calculate pressure without temperature readings
				rt_kprintf("Pressure: no temperature readings\r\n");
#endif
			}
			else
			{
				if (UP == 0x80000)
				{
#if DEBUG
					// Either pressure measurement is configured as 'skip' or first
					// conversion is not completed yet
					rt_kprintf("Pressure: no data\r\n");
#endif
				}
				else
				{
					// Pressure
					baro_v[curr_read] = BMP280_CalcPf(UP);
#if DEBUG
					pressure = (uint32_t)(baro_v[curr_read]);
					rt_kprintf("Pressure: %.3uPa [%.3uhPa]\r\n", pressure,
							   pressure / 100);
#endif
				}
			}
#if DEBUG
			rt_kprintf("------------------------\r\n");
#endif
			rt_thread_mdelay(10);
			rt_mutex_release(raw_mutex);
		}

#if COUNT
		end_time = rt_tick_get_millisecond();
		time_taken = (rt_int32_t)(end_time - init_time);

		rt_kprintf("End sampling, time taken : %d\n", time_taken);
#endif

#if USE_DEFERR
		rt_thread_mdelay(201000);
#else
		rt_thread_mdelay(240000);
#endif
	}
}

/**
 * This function is the entry for the consumer
 * it executes the benchmark
 *
 * @param NULL parameter passed to the function
 *
 * @return void
 */
void consumer_entry(void *parameter)
{
	rt_thread_mdelay(5000);
	while (1)
	{

		rt_kprintf("Start benchmark\n");

#if COUNT
		rt_tick_t init, end;
		rt_int32_t time_taken;
		init = rt_tick_get_millisecond();
#endif
		init_xeno();
#if COUNT
		end = rt_tick_get_millisecond();
		time_taken = end - init;
		rt_kprintf("End benchmark, time taken : %d\n", time_taken);
#endif

#if USE_DEFERR
		rt_thread_mdelay(6000);
#else
		rt_thread_mdelay(45000);
#endif
	}
}
