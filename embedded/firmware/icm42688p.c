#include "icm42688p.h"
#include "sl_i2cspm.h"
#include "sl_udelay.h"
#include "app_log.h"

// Write a single register
sl_status_t icm42688p_write_reg(sl_i2cspm_t *i2c,
                                 uint8_t reg, uint8_t val)
{
  I2C_TransferSeq_TypeDef seq;
  uint8_t buf[2] = {reg, val};

  seq.addr        = ICM42688P_I2C_ADDR << 1;
  seq.flags       = I2C_FLAG_WRITE;
  seq.buf[0].data = buf;
  seq.buf[0].len  = 2;

  if (I2CSPM_Transfer(i2c, &seq) != i2cTransferDone) {
    return SL_STATUS_TRANSMIT;
  }
  return SL_STATUS_OK;
}

// Read one or more registers
sl_status_t icm42688p_read_reg(sl_i2cspm_t *i2c, uint8_t reg,
                                uint8_t *data, uint8_t len)
{
  I2C_TransferSeq_TypeDef seq;

  seq.addr        = ICM42688P_I2C_ADDR << 1;
  seq.flags       = I2C_FLAG_WRITE_READ;
  seq.buf[0].data = &reg;
  seq.buf[0].len  = 1;
  seq.buf[1].data = data;
  seq.buf[1].len  = len;

  if (I2CSPM_Transfer(i2c, &seq) != i2cTransferDone) {
    return SL_STATUS_RECEIVE;
  }
  return SL_STATUS_OK;
}

// Verify WHO_AM_I register
sl_status_t icm42688p_verify(sl_i2cspm_t *i2c)
{
  uint8_t who_am_i = 0;
  sl_status_t sc = icm42688p_read_reg(i2c,
                     ICM42688P_REG_WHO_AM_I, &who_am_i, 1);
  if (sc != SL_STATUS_OK) return sc;

  if (who_am_i != ICM42688P_WHO_AM_I_VAL) {
    app_log("ICM42688P WHO_AM_I mismatch: 0x%02X\n", who_am_i);
    return SL_STATUS_FAIL;
  }
  app_log("ICM42688P detected OK (WHO_AM_I=0x%02X)\n", who_am_i);
  return SL_STATUS_OK;
}

// Initialize the IMU
sl_status_t icm42688p_init(sl_i2cspm_t *i2c)
{
  sl_status_t sc;

  // 1. Verify device identity
  sc = icm42688p_verify(i2c);
  if (sc != SL_STATUS_OK) return sc;

  // 2. Wake up — enable accel + gyro in low-noise mode
  sc = icm42688p_write_reg(i2c, ICM42688P_REG_PWR_MGMT0, 0x0F);
  if (sc != SL_STATUS_OK) return sc;
  sl_udelay_wait(1000); // 1ms startup time

  // 3. Set Gyro: ±500dps, 100Hz
  sc = icm42688p_write_reg(i2c, ICM42688P_REG_GYRO_CFG0,
                            ICM42688P_GYRO_FSR_500DPS |
                            ICM42688P_ODR_100HZ);
  if (sc != SL_STATUS_OK) return sc;

  // 4. Set Accel: ±4g, 100Hz
  sc = icm42688p_write_reg(i2c, ICM42688P_REG_ACCEL_CFG0,
                            ICM42688P_ACCEL_FSR_4G |
                            ICM42688P_ODR_100HZ);
  if (sc != SL_STATUS_OK) return sc;

  app_log("ICM42688P initialized: ±4g, ±500dps, 100Hz\n");
  return SL_STATUS_OK;
}

// Read raw 16-bit values (6 axes)
sl_status_t icm42688p_read_raw(sl_i2cspm_t *i2c,
                                icm42688p_raw_t *data)
{
  uint8_t buf[12];
  sl_status_t sc = icm42688p_read_reg(i2c,
                     ICM42688P_REG_ACCEL_X1, buf, 12);
  if (sc != SL_STATUS_OK) return sc;

  // Accel: bytes 0-5
  data->accel_x = (int16_t)((buf[0]  << 8) | buf[1]);
  data->accel_y = (int16_t)((buf[2]  << 8) | buf[3]);
  data->accel_z = (int16_t)((buf[4]  << 8) | buf[5]);
  // Gyro: bytes 6-11
  data->gyro_x  = (int16_t)((buf[6]  << 8) | buf[7]);
  data->gyro_y  = (int16_t)((buf[8]  << 8) | buf[9]);
  data->gyro_z  = (int16_t)((buf[10] << 8) | buf[11]);

  return SL_STATUS_OK;
}

// Read scaled values in g and dps
sl_status_t icm42688p_read_scaled(sl_i2cspm_t *i2c,
                                   icm42688p_scaled_t *data)
{
  icm42688p_raw_t raw;
  sl_status_t sc = icm42688p_read_raw(i2c, &raw);
  if (sc != SL_STATUS_OK) return sc;

  data->accel_x = raw.accel_x * ICM42688P_ACCEL_SCALE;
  data->accel_y = raw.accel_y * ICM42688P_ACCEL_SCALE;
  data->accel_z = raw.accel_z * ICM42688P_ACCEL_SCALE;
  data->gyro_x  = raw.gyro_x  * ICM42688P_GYRO_SCALE;
  data->gyro_y  = raw.gyro_y  * ICM42688P_GYRO_SCALE;
  data->gyro_z  = raw.gyro_z  * ICM42688P_GYRO_SCALE;

  return SL_STATUS_OK;
}