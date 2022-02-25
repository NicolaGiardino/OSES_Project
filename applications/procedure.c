#include "procedure.h"
#include "../Benchmark/XenoJetBench.h"

static rt_uint32_t addr = 0x00;
static rt_uint32_t addr_bench = 0x800000;
static size_t curr_read = 0;
static int16_t acc[3], gyro[3], temp;

<<<<<<< Updated upstream
static void write_mem_async() {
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
=======
<<<<<<< HEAD
static void write_raw_mem_async()
{
  static uint8_t addr8[3];
  static rt_uint8_t acc8[4], gyro8[4], temp8[4];

  if(addr >= 0x7FFFF4)
  {
      w25q64_control(CHIP_ERASE, RT_NULL, RT_NULL, RT_NULL);
  }

  rt_mutex_take(raw_mutex, RT_WAITING_FOREVER);

  int i, j = 0;

  acc8[0] = (rt_uint8_t)acc;
  acc8[1] = (rt_uint8_t)(acc << 8);
  acc8[2] = (rt_uint8_t)(acc << 16);
  acc8[3] = (rt_uint8_t)(acc << 24);

  gyro8[0] = (rt_uint8_t)gyro;
  gyro8[1] = (rt_uint8_t)(gyro << 8);
  gyro8[2] = (rt_uint8_t)(gyro << 16);
  gyro8[3] = (rt_uint8_t)(gyro << 24);

  temp8[0] = (rt_uint8_t)temp;
  temp8[1] = (rt_uint8_t)(temp << 8);
  temp8[3] = (rt_uint8_t)(temp << 16);
  temp8[4] = (rt_uint8_t)(temp << 24);

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
>>>>>>> Stashed changes

  temp8[0] = (rt_uint8_t)temp;
  temp8[1] = (rt_uint8_t)(temp << 8);

<<<<<<< Updated upstream
=======
static void read_raw_mem_async()
{
  rt_mutex_take(raw_mutex, RT_WAITING_FOREVER);

  rt_uint32_t addr32;
  rt_uint8_t a[3] = {0x00, 0x00, 0x00};
  rt_uint8_t rd[12];

  addr32 = addr - 0xC;

  addr8[0] = (rt_uint8_t)addr32;
  addr8[1] = (rt_uint8_t)(addr32 << 8);
  addr8[2] = (rt_uint8_t)(addr32 << 16);
  w25q64_control(READ_DATA, a, 12, rd);
=======
static void write_mem_async() {
  static uint8_t addr8[3];
  static rt_uint8_t acc8[6], gyro8[6], temp8[2];

  rt_mutex_take(raw_mutex, RT_WAITING_FOREVER);

  int i, j = 0;
>>>>>>> cdf77fb5f751a65f01587b73b8bbe6f9db3b5e62

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

<<<<<<< HEAD
void button_raw_async_handler()
{
  static uint8_t RWn =
      0; /* alternate between writing (first) and reading (after) */

  RWn ? read_raw_mem_async() : write_raw_mem_async(); /* perform asynchronous action */
=======
>>>>>>> Stashed changes
  addr8[0] = (rt_uint8_t)addr;
  addr8[1] = (rt_uint8_t)(addr << 8);
  addr8[2] = (rt_uint8_t)(addr << 16);
  w25q64_control(PAGE_PROGRAM, addr8, 6, acc8);
<<<<<<< Updated upstream
=======
>>>>>>> cdf77fb5f751a65f01587b73b8bbe6f9db3b5e62
>>>>>>> Stashed changes

#if DEBUG
  rt_kprintf("Wrote acc data on FLASH\n");
#endif

  addr += 0x03;
  addr8[0] = (rt_uint8_t)addr;
  addr8[1] = (rt_uint8_t)(addr << 8);
  addr8[2] = (rt_uint8_t)(addr << 16);
  w25q64_control(PAGE_PROGRAM, addr8, 6, gyro8);

<<<<<<< Updated upstream
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

=======
<<<<<<< HEAD
static void write_bench_mem_async()
{
  static uint8_t addr8[3];
  static rt_uint8_t res8[56];

  if(addr >= 0xFFFFF4)
  {
      w25q64_control(CHIP_ERASE, RT_NULL, RT_NULL, RT_NULL);
  }

  rt_mutex_take(bench_mutex, RT_WAITING_FOREVER);

  int i, j = 0;

  for(i = 0; i < 56; i++)
  {
      res8[i] = (rt_uint8_t)results[j];
      res8[++i] = (rt_uint8_t)(results[j] << 8);
      res8[++i] = (rt_uint8_t)(results[j] << 16);
      res8[++i] = (rt_uint8_t)(results[j] << 24);
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

static void read_raw_mem_async()
{
  rt_mutex_take(bench_mutex, RT_WAITING_FOREVER);
=======
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

>>>>>>> Stashed changes
static void read_mem_async() {
  rt_mutex_take(raw_mutex, RT_WAITING_FOREVER);

  addr += 0x01;
  rt_thread_mdelay(1000);
>>>>>>> cdf77fb5f751a65f01587b73b8bbe6f9db3b5e62

  rt_uint32_t addr32;
  rt_uint8_t a[3] = {0x00, 0x00, 0x00};
  rt_uint8_t rd[56];

  addr32 = addr - 0x38;

  addr8[0] = (rt_uint8_t)addr32;
  addr8[1] = (rt_uint8_t)(addr32 << 8);
  addr8[2] = (rt_uint8_t)(addr32 << 16);
  w25q64_control(READ_DATA, a, 56, rd);

#if DEBUG
  for (i = 0; i < 28; i += 2) {
    rt_kprintf("%d\t", (int16_t)(rd[i]) + ((int16_t)(rd[i + 1]) >> 8));
  }
  rt_kprintf("\n");
#endif

  rt_mutex_release(bench_mutex);
}

<<<<<<< Updated upstream
void button_raw_async_handler() {
  static uint8_t RWn =
      0; /* alternate between writing (first) and reading (after) */

  RWn ? read_mem_async() : write_mem_async(); /* perform asynchronous action */
=======
<<<<<<< HEAD
void button_raw_async_handler()
{
  static uint8_t RWn =
      0; /* alternate between writing (first) and reading (after) */

  RWn ? read_bench_mem_async() : write_bench_mem_async(); /* perform asynchronous action */
=======
void button_raw_async_handler() {
  static uint8_t RWn =
      0; /* alternate between writing (first) and reading (after) */

  RWn ? read_mem_async() : write_mem_async(); /* perform asynchronous action */
>>>>>>> cdf77fb5f751a65f01587b73b8bbe6f9db3b5e62
>>>>>>> Stashed changes

#if DEBUG
  rt_kprintf("IRQ happened\n");
#endif

  RWn ^= 1; /* invert selection */
}

void producer_entry(void *parameter) {
  mpu6050_init("i2c1");
  mpu6050_reset();

#if DEBUG
  rt_kprintf("Init MPU6050 done\n");
#endif

  while (1) {

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

    rt_thread_mdelay(3000);
  }
}

void consumer_entry(void *parameter) {
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
