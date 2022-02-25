#ifndef PROCEDURE_H
#define PROCEDURE_H

#include "MPU6050.h"
#include "w25q64.h"
#include "deferrable.h"
#include <board.h>
#include <rtdevice.h>
#include <rtthread.h>

#define DEBUG 0

#define USE_DEFERR 0

#define THREAD_PRIORITY RT_THREAD_PRIORITY_MAX - 15
#define THREAD_TIMESLICE 10

#define NUM_READINGS 125

float acc_v[NUM_READINGS], gyro_v[NUM_READINGS], temp_v[NUM_READINGS];
float results[14];

#if USE_DEFERR
  rt_aperiodic_task_t raw[2], bench[2];
#endif

rt_mutex_t raw_mutex, bench_mutex;

void write_raw_mem_async();
void read_raw_mem_async();
void button_raw_async_handler();

void write_bench_mem_async();
void read_bench_mem_async();
void button_bench_async_handler();

struct rt_thread producer;
char producer_stack[1024];
void producer_entry(void *parameter);

struct rt_thread consumer;
char consumer_stack[1024];
void consumer_entry(void *parameter);

#endif
