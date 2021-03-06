#ifndef PROCEDURE_H
#define PROCEDURE_H

#define USE_DEFERR 1

#include "../peripherals/BMP280.h"
#include "../peripherals/MPU6050.h"
#include "../peripherals/w25q64.h"
#if USE_DEFERR
#include "deferrable.h"
#endif
#include <board.h>
#include <rtdevice.h>
#include <rtthread.h>

#define DEBUG 1
#define COUNT 0

#define THREAD_PRIORITY RT_THREAD_PRIORITY_MAX - 15
#define THREAD_TIMESLICE 10
#define THREAD_PRIORITY_DEF THREAD_PRIORITY

#define NUM_READINGS 125

float acc_v[NUM_READINGS], gyro_v[NUM_READINGS], baro_v[NUM_READINGS];
float results[14];

#if USE_DEFERR
rt_aperiodic_task_t raw[2], bench[2];
#else
char mem_bench[2][1024];
struct rt_thread write_mem_bench, read_mem_bench;
char mem_raw[2][1024];
struct rt_thread write_mem_raw, read_mem_raw;
#endif

rt_mutex_t raw_mutex, bench_mutex;

void write_raw_mem_async();
void read_raw_mem_async();
void button_raw_async_handler();

void write_bench_mem_async();
void read_bench_mem_async();
void button_bench_async_handler();

struct rt_thread producer;
char producer_stack[2048];
void producer_entry(void *parameter);

struct rt_thread consumer;
char consumer_stack[20480];
void consumer_entry(void *parameter);

#endif
