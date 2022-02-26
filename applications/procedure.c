#include "procedure.h"
#include "../Benchmark/XenoJetBench.h"

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
  static rt_uint8_t acc8[4], gyro8[4], temp8[4];

  if(addr >= 0x7FFFF4)
  {
      w25q64_control(CHIP_ERASE, RT_NULL, RT_NULL, RT_NULL);
      addr = 0x00;
  }

  rt_mutex_take(raw_mutex, RT_WAITING_FOREVER);

  acc8[0] = (rt_uint8_t)acc_v[0];
  acc8[1] = (rt_uint8_t)((rt_uint32_t)acc_v[0] << 8);
  acc8[2] = (rt_uint8_t)((rt_uint32_t)acc_v[0] << 16);
  acc8[3] = (rt_uint8_t)((rt_uint32_t)acc_v[0] << 24);

  gyro8[0] = (rt_uint8_t)gyro_v[0];
  gyro8[1] = (rt_uint8_t)((rt_uint32_t)gyro_v[0] << 8);
  gyro8[2] = (rt_uint8_t)((rt_uint32_t)gyro_v[0] << 16);
  gyro8[3] = (rt_uint8_t)((rt_uint32_t)gyro_v[0] << 24);

  temp8[0] = (rt_uint8_t)temp_v[0];
  temp8[1] = (rt_uint8_t)((rt_uint32_t)temp_v[0] << 8);
  temp8[3] = (rt_uint8_t)((rt_uint32_t)temp_v[0] << 16);
  temp8[4] = (rt_uint8_t)((rt_uint32_t)temp_v[0] << 24);

  addr8[0] = (rt_uint8_t)addr;
  addr8[1] = (rt_uint8_t)(addr << 8);
  addr8[2] = (rt_uint8_t)(addr << 16);
  w25q64_control(PAGE_PROGRAM, addr8, 4, acc8);

#if DEBUG
  rt_kprintf("Wrote acc data on FLASH\n");
#endif

  addr += 0x04;
  addr8[0] = (rt_uint8_t)addr;
  addr8[1] = (rt_uint8_t)(addr << 8);
  addr8[2] = (rt_uint8_t)(addr << 16);
  w25q64_control(PAGE_PROGRAM, addr8, 4, gyro8);

#if DEBUG
  rt_kprintf("Wrote gyro data on FLASH\n");
#endif

  addr8[0] = (rt_uint8_t)addr;
  addr8[1] = (rt_uint8_t)(addr << 8);
  addr8[2] = (rt_uint8_t)(addr << 16);
  addr += 0x04;
  w25q64_control(PAGE_PROGRAM, addr8, 4, temp8);

  addr += 0x04;

#if DEBUG
  rt_kprintf("Wrote temp data on FLASH\n");
#endif

  rt_mutex_release(raw_mutex);
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
  rt_mutex_take(raw_mutex, RT_WAITING_FOREVER);

  rt_uint32_t addr32;
  rt_uint8_t addr8[3] = {0x00, 0x00, 0x00};
  rt_uint8_t rd[12];

  addr32 = addr - 0xC;

  addr8[0] = (rt_uint8_t)addr32;
  addr8[1] = (rt_uint8_t)(addr32 << 8);
  addr8[2] = (rt_uint8_t)(addr32 << 16);
  w25q64_control(READ_DATA, addr8, 12, rd);

#if DEBUG
  int i;

  for (i = 0; i < 28; i += 2) {
    rt_kprintf("%d\t", (int16_t)(rd[i]) + ((int16_t)(rd[i + 1]) >> 8));
  }
  rt_kprintf("\n");
#endif

  rt_mutex_release(raw_mutex);
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
  static uint8_t RWn = 0; /* alternate between writing (first) and reading (after) */

#if USE_DEFERR
  RWn ? rt_thread_deferrable_insert_task(&raw[1]) : rt_thread_deferrable_insert_task(&raw[0]);
#else
  RWn ? read_raw_mem_async() : write_raw_mem_async(); /* perform asynchronous action */
#endif

  rt_kprintf("IRQ happened\n");

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

  if(addr_bench >= 0xFFFFC8)
  {
      w25q64_control(CHIP_ERASE, RT_NULL, RT_NULL, RT_NULL);
      addr_bench = 0x800000;
  }

  rt_mutex_take(bench_mutex, RT_WAITING_FOREVER);

  int i, j = 0;

  for(i = 0; i < 56; i++)
  {
      res8[i] = (rt_uint8_t)results[j];
      res8[++i] = (rt_uint8_t)((rt_uint32_t)results[j] << 8);
      res8[++i] = (rt_uint8_t)((rt_uint32_t)results[j] << 16);
      res8[++i] = (rt_uint8_t)((rt_uint32_t)results[j] << 24);
      j++;
  }

  addr8[0] = (rt_uint8_t)addr_bench;
  addr8[1] = (rt_uint8_t)(addr_bench << 8);
  addr8[2] = (rt_uint8_t)(addr_bench << 16);
  w25q64_control(PAGE_PROGRAM, addr8, 56, res8);

  addr_bench += 0x38;

#if DEBUG
  rt_kprintf("Wrote res data on FLASH\n");
#endif

  rt_mutex_release(raw_mutex);
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
  rt_mutex_take(bench_mutex, RT_WAITING_FOREVER);

  rt_uint32_t addr32;
  rt_uint8_t addr8[3] = {0x00, 0x00, 0x00};
  rt_uint8_t rd[56];

  addr32 = addr_bench - 0x38;

  addr8[0] = (rt_uint8_t)addr32;
  addr8[1] = (rt_uint8_t)(addr32 << 8);
  addr8[2] = (rt_uint8_t)(addr32 << 16);
  w25q64_control(READ_DATA, addr8, 56, rd);

#if DEBUG
  int i;
  for (i = 0; i < 28; i += 2) {
    rt_kprintf("%d\t", (int16_t)(rd[i]) + ((int16_t)(rd[i + 1]) >> 8));
  }
  rt_kprintf("\n");
#endif

  rt_mutex_release(bench_mutex);
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
  static uint8_t RWn = 0; /* alternate between writing (first) and reading (after) */
#if USE_DEFERR
  RWn ? rt_thread_deferrable_insert_task(&bench[1]) : rt_thread_deferrable_insert_task(&bench[0]);
#else
  RWn ? read_bench_mem_async() : write_bench_mem_async(); /* perform asynchronous action */
#endif

  rt_kprintf("IRQ happened\n");

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
  mpu6050_init("i2c1");
  mpu6050_reset();

#if DEBUG
  rt_kprintf("Init MPU6050 done\n");
#endif

  while (1) {

    rt_kprintf("Start sampling\n");

    for (curr_read = 0; curr_read < NUM_READINGS; curr_read++) {
      rt_mutex_take(raw_mutex, RT_WAITING_FOREVER);

      mpu6050_read_raw(acc, gyro, &temp);
      acc_v[curr_read] =
          sqroot((float)(acc[0] * acc[0] + acc[1] * acc[1] + acc[2] * acc[2]));
      gyro_v[curr_read] = sqroot(
          (float)(gyro[0] * gyro[0] + gyro[1] * gyro[1] + gyro[2] * gyro[2]));
      temp_v[curr_read] = ((float)temp / 340.00) + 36.53;

#if DEBUG
      rt_kprintf("Acc. X = %d, Y = %d, Z = %d\n", acc[0], acc[1], acc[2]);
      rt_kprintf("Gyro. X = %d, Y = %d, Z = %d\n", gyro[0], gyro[1], gyro[2]);
      rt_kprintf("Temp. raw = %d\n", temp);
      rt_kprintf("Acc. mod = %d\n", (int)acc_v[curr_read]);
      rt_kprintf("Gyro. mod = %d\n", (int)gyro_v[curr_read]);
      rt_kprintf("Temp. actual = %d\n", (int)temp_v[curr_read]);
#endif

      rt_mutex_release(raw_mutex);
    }

    rt_kprintf("End sampling\n");
    rt_thread_mdelay(3000);
  }
}

/**
 * This function is the entry for the  consumer
 * it executes the benchmark
 *
 * @param NULL parameter passed to the function
 *
 * @return void
 */
void consumer_entry(void *parameter)
{
  while (1) {

    rt_kprintf("Start benchmark\n");
    init_xeno();
    rt_kprintf("End benchmark\n");


  }
}