#include "procedure.h"
#include "../Benchmark/XenoJetBench.h"

static rt_uint32_t addr = 0x00;
static size_t curr_read = 0;
static int16_t acc[3], gyro[3], temp;


static void write_mem_async()
{
      static uint8_t addr8[3];
      static rt_uint8_t acc8[6], gyro8[6], temp8[2];

      rt_mutex_take(raw_mutex, RT_WAITING_FOREVER);

      int i, j = 0;

      for (i = 0; i < 3; i++) {
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

      rt_mutex_release(raw_mutex);
}

static void read_mem_async()
{
  rt_mutex_take(raw_mutex, RT_WAITING_FOREVER);

  addr += 0x01;
  rt_thread_mdelay(1000);

  rt_uint8_t a[3] = {0x00, 0x00, 0x00};
  rt_uint8_t rd[28];
  w25q64_control(READ_DATA, a, 28, rd);

#if DEBUG
  for (i = 0; i < 28; i += 2) {
    rt_kprintf("%d\t", (int16_t)(rd[i]) + ((int16_t)(rd[i + 1]) >> 8));
  }
  rt_kprintf("\n");
#endif

  rt_mutex_release(raw_mutex);
}

void button_raw_async_handler()
{
  static uint8_t RWn =
      0; /* alternate between writing (first) and reading (after) */

  RWn ? read_mem_async() : write_mem_async(); /* perform asynchronous action */

#if DEBUG
  rt_kprintf("IRQ happened\n");
#endif

  RWn ^= 1; /* invert selection */
}

/*
static void write_bench_mem_async()
{
      static uint8_t addr8[3];
      static rt_uint8_t acc8[6], gyro8[6], temp8[2];

      rt_mutex_take(raw_mutex, RT_WAITING_FOREVER);

      int i, j = 0;

      for (i = 0; i < 3; i++) {
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

      rt_mutex_release(raw_mutex);
}

static void read_bench_mem_async()
{
  rt_mutex_take(raw_mutex, RT_WAITING_FOREVER);

  addr += 0x01;
  rt_thread_mdelay(1000);

  rt_uint8_t a[3] = {0x00, 0x00, 0x00};
  rt_uint8_t rd[28];
  w25q64_control(READ_DATA, a, 28, rd);

#if DEBUG
  for (i = 0; i < 28; i += 2) {
    rt_kprintf("%d\t", (int16_t)(rd[i]) + ((int16_t)(rd[i + 1]) >> 8));
  }
  rt_kprintf("\n");
#endif

  rt_mutex_release(raw_mutex);
}

void button_bench_async_handler()
{
  static uint8_t RWn =
      0; /* alternate between writing (first) and reading (after) *

  RWn ? read_bench_mem_async() : write_bench_mem_async(); /* perform asynchronous action *

#if DEBUG
  rt_kprintf("IRQ happened\n");
#endif

  RWn ^= 1; /* invert selection *
}
*/

void producer_entry(void *parameter)
{
  mpu6050_init("i2c1");
  mpu6050_reset();

#if DEBUG
  rt_kprintf("Init MPU6050 done\n");
#endif

  while (1) {

    for(curr_read = 0; curr_read < NUM_READINGS; curr_read++)
    {
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

    rt_thread_mdelay(3000);
  }
}

void consumer_entry(void *parameter)
{
  while (1) {
    rt_mutex_take(raw_mutex, RT_WAITING_FOREVER);

    init_xeno(acc_v, gyro_v, temp_v, NUM_READINGS);

    rt_mutex_release(raw_mutex);

    rt_mutex_take(bench_mutex, RT_WAITING_FOREVER);

    results[0] = u0d;
    results[1] = altd;
    results[2] = throtl;
    results[3] = fsmach;
    results[4] = psout;
    results[5] = tsout;
    results[6] = fnlb;
    results[7] = fglb;
    results[8] = drlb;
    results[9] = flflo;
    results[10] = sfc;
    results[11] = eair;
    results[12] = weight;
    results[13] = (fnlb / weight);

    rt_mutex_release(bench_mutex);
  }
}
