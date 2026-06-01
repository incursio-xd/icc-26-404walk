#ifndef LDMA_BUFFER_H
#define LDMA_BUFFER_H

#include <stdint.h>
#include <stdbool.h>
#include "em_ldma.h"

// Buffer configuration
#define SAMPLE_RATE_HZ       100
#define WINDOW_SIZE_SEC      3
#define WINDOW_SAMPLES       (SAMPLE_RATE_HZ * WINDOW_SIZE_SEC)  // 300
#define NUM_AXES             6
#define BUFFER_SIZE          (WINDOW_SAMPLES * NUM_AXES)          // 1800

// Raw IMU sample (6 x int16)
typedef struct {
  int16_t accel_x;
  int16_t accel_y;
  int16_t accel_z;
  int16_t gyro_x;
  int16_t gyro_y;
  int16_t gyro_z;
} imu_sample_t;

// Ping-pong buffers
extern imu_sample_t imu_buffer_A[WINDOW_SAMPLES];
extern imu_sample_t imu_buffer_B[WINDOW_SAMPLES];

// Flags set by DMA interrupt — read by main loop
extern volatile bool buffer_A_ready;
extern volatile bool buffer_B_ready;
extern volatile uint32_t buffer_overflow_count;

// API
void ldma_buffer_init(void);
void ldma_buffer_start(void);
void ldma_buffer_stop(void);
imu_sample_t* ldma_buffer_get_ready(void);
void ldma_buffer_release(imu_sample_t *buf);

#endif // LDMA_BUFFER_H