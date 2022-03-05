#ifndef __BMP280_H
#define __BMP280_H

#include "drv_soft_i2c.h"
#include <board.h>
#include <rtdevice.h>
#include <rtthread.h>

struct rt_i2c_bus_device *i2c_device;
// Method for calculating compensated values
//   0: using 32-bit values - lowest precision, less code)
//   1: using 64-bit values - more precision, more code
//   2: using float calculations - best precision, lots of code on MCU without
//   FPU
#define BMP280_CALC_TYPE 2

#if (BMP280_CALC_TYPE < 0) || (BMP280_CALC_TYPE > 2)
#error "Please define correct value for BMP280_CALC_TYPE"
#endif

// Functions that operating with floats (both calculations and return value)
// Their names end with the 'f'
//   0: absent
//   1: present
#define BMP280_FLOAT_FUNCTIONS 1

// Possible I2C device address values
#define BMP280_ADDR_G (0x76)          // SDO pin tied to GND
#define BMP280_ADDR_V ((uint8_t)0x77) // SDO pin tied to VDDIO

// Device address, select one of BMP280_ADDR_XXX values above according to the
// chip wiring
#define BMP280_ADDR (BMP280_ADDR_G)

// BMP280 register bits

// Device state (status)
#define BMP280_STATUS_IM_UPDATE                                                \
  ((uint8_t)0x01) // [0] NVM data being copied to image registers
#define BMP280_STATUS_MEASURING ((uint8_t)0x08) // [3] conversion is running

// Temperature oversampling (ctrl_meas:osrs_t [7:5])
#define BMP280_OSRS_T_SKIP ((uint8_t)0x00) // Skipped
#define BMP280_OSRS_T_x1 ((uint8_t)0x20)   // x1
#define BMP280_OSRS_T_x2 ((uint8_t)0x40)   // x2
#define BMP280_OSRS_T_x4 ((uint8_t)0x60)   // x4
#define BMP280_OSRS_T_x8 ((uint8_t)0x80)   // x8
#define BMP280_OSRS_T_x16 ((uint8_t)0xA0)  // x16

// Pressure oversampling (ctrl_meas:osrs_p [4:2])
#define BMP280_OSRS_P_SKIP ((uint8_t)0x00) // Skipped
#define BMP280_OSRS_P_x1 ((uint8_t)0x04)   // x1
#define BMP280_OSRS_P_x2 ((uint8_t)0x08)   // x2
#define BMP280_OSRS_P_x4 ((uint8_t)0x0C)   // x4
#define BMP280_OSRS_P_x8 ((uint8_t)0x10)   // x8
#define BMP280_OSRS_P_x16 ((uint8_t)0x14)  // x16

// Power mode of the device (ctrl_meas:mode [1:0])
#define BMP280_MODE_SLEEP ((uint8_t)0x00)   // Sleep mode
#define BMP280_MODE_FORCED ((uint8_t)0x01)  // Forced mode
#define BMP280_MODE_FORCED2 ((uint8_t)0x02) // Forced mode
#define BMP280_MODE_NORMAL ((uint8_t)0x03)  // Normal mode

// Inactive duration in normal mode (config:t_sb [7:5])
#define BMP280_STBY_0p5ms ((uint8_t)0x00)  // 0.5ms
#define BMP280_STBY_62p5ms ((uint8_t)0x20) // 62.5ms
#define BMP280_STBY_125ms ((uint8_t)0x40)  // 125ms
#define BMP280_STBY_250ms ((uint8_t)0x60)  // 250ms
#define BMP280_STBY_500ms ((uint8_t)0x80)  // 500ms
#define BMP280_STBY_1s ((uint8_t)0xA0)     // 1s
#define BMP280_STBY_2s ((uint8_t)0xC0)     // 2s
#define BMP280_STBY_4s ((uint8_t)0xE0)     // 4s

// Time constant of the IIR filter (config:filter [4:2])
#define BMP280_FILTER_OFF ((uint8_t)0x00) // Off
#define BMP280_FILTER_2 ((uint8_t)0x04)   // 2
#define BMP280_FILTER_4 ((uint8_t)0x08)   // 4
#define BMP280_FILTER_8 ((uint8_t)0x0C)   // 8
#define BMP280_FILTER_16 ((uint8_t)0x10)  // 16

// Values indicating no readings is present
#define BMP280_NO_TEMPERATURE ((int32_t)0x80000)
#define BMP280_NO_PRESSURE ((int32_t)0x80000)

// BMP280 function result
typedef enum { BMP280_ERROR = 0, BMP280_SUCCESS = !BMP280_ERROR } BMP280_RESULT;

// Function prototypes
BMP280_RESULT BMP280_init(char *dev_name);

BMP280_RESULT BMP280_Check(void);
void BMP280_Reset(void);

rt_uint8_t BMP280_GetVersion(void);
rt_uint8_t BMP280_GetStatus(void);
rt_uint8_t BMP280_GetMode(void);

void BMP280_SetMode(rt_uint8_t mode);
void BMP280_SetFilter(rt_uint8_t filter);
void BMP280_SetStandby(rt_uint8_t tsb);
void BMP280_SetOSRST(rt_uint8_t osrs);
void BMP280_SetOSRSP(rt_uint8_t osrs);

BMP280_RESULT BMP280_Read_Calibration(void);
BMP280_RESULT BMP280_Read_UP(rt_int32_t *UP);
BMP280_RESULT BMP280_Read_UT(rt_int32_t *UT);
BMP280_RESULT BMP280_Read_UTP(rt_int32_t *UT, rt_int32_t *UP);

rt_int32_t BMP280_CalcT(rt_int32_t UT);
rt_uint32_t BMP280_CalcP(rt_int32_t UP);

rt_uint32_t BMP280_Pa_to_mmHg(rt_uint32_t p_pa);

#if (BMP280_FLOAT_FUNCTIONS)

float BMP280_CalcTf(rt_int32_t UT);
float BMP280_CalcPf(rt_uint32_t UP);
float BMP280_Pa_to_mmHgf(float p_pa);

#endif // BMP280_FLOAT_FUNCTIONS

#endif // __BMP280_H
