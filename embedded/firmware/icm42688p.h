#ifndef ICM42688P_H
#define ICM42688P_H

#include <stdint.h>
#include "sl_i2cspm.h"
#include "sl_status.h"

// ICM-42688-P I2C Address (AD0 pin low = 0x68, high = 0x69)
#define ICM42688P_I2C_ADDR        0x68

// Register Map
#define ICM42688P_REG_WHO_AM_I    0x75  // Should return 0x47
#define ICM42688P_REG_PWR_MGMT0   0x4E
#define ICM42688P_REG_GYRO_CFG0   0x4F
#define ICM42688P_REG_ACCEL_CFG0  0x50
#define ICM42688P_REG_INT_CONFIG  0x14
#define ICM42688P_REG_ACCEL_X1    0x1F  // Start of 12-byte data burst
#define ICM42688P_REG_TEMP_DATA1  0x1D

// WHO_AM_I expected value
#define ICM42688P_WHO_AM_I_VAL    0x47

// Accel FSR: ±4g
#define ICM42688P_ACCEL_FSR_4G    0x60
// Gyro FSR: ±500dps
#define ICM42688P_GYRO_FSR_500DPS 0x40
// ODR: 100Hz
#define ICM42688P_ODR_100HZ       0x08

// Conversion factors
#define ICM42688P_ACCEL_SCALE     (4.0f / 32768.0f)   // ±4g range
#define ICM42688P_GYRO_SCALE      (500.0f / 32768.0f) // ±500dps range

// Raw IMU data struct
typedef struct {
  int16_t accel_x;
  int16_t accel_y;
  int16_t accel_z;
  int16_t gyro_x;
  int16_t gyro_y;
  int16_t gyro_z;
} icm42688p_raw_t;

// Scaled IMU data struct
typedef struct {
  float accel_x;  // g
  float accel_y;
  float accel_z;
  float gyro_x;   // dps
  float gyro_y;
  float gyro_z;
} icm42688p_scaled_t;

// Function prototypes
sl_status_t icm42688p_init(sl_i2cspm_t *i2c);
sl_status_t icm42688p_verify(sl_i2cspm_t *i2c);
sl_status_t icm42688p_read_raw(sl_i2cspm_t *i2c, icm42688p_raw_t *data);
sl_status_t icm42688p_read_scaled(sl_i2cspm_t *i2c, icm42688p_scaled_t *data);
sl_status_t icm42688p_write_reg(sl_i2cspm_t *i2c, uint8_t reg, uint8_t val);
sl_status_t icm42688p_read_reg(sl_i2cspm_t *i2c, uint8_t reg,
                                uint8_t *data, uint8_t len);

#endif // ICM42688P_H