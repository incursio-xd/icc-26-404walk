#include "ldma_buffer.h"
#include "em_ldma.h"
#include "em_cmu.h"
#include "app_log.h"

// Ping-pong buffers
imu_sample_t imu_buffer_A[WINDOW_SAMPLES];
imu_sample_t imu_buffer_B[WINDOW_SAMPLES];

volatile bool     buffer_A_ready       = false;
volatile bool     buffer_B_ready       = false;
volatile uint32_t buffer_overflow_count = 0;

static bool using_buffer_A = true;

// LDMA channel
#define IMU_LDMA_CHANNEL  0

void ldma_buffer_init(void)
{
  CMU_ClockEnable(cmuClock_LDMA, true);
  LDMA_Init_t init = LDMA_INIT_DEFAULT;
  LDMA_Init(&init);
  app_log("[LDMA] Initialized\n");
}

void ldma_buffer_start(void)
{
  // TODO Week 1-2: Wire to I2C DMA transfer
  // For now just mark as initialized
  using_buffer_A = true;
  buffer_A_ready = false;
  buffer_B_ready = false;
  app_log("[LDMA] Started - ping-pong buffering active\n");
}

void ldma_buffer_stop(void)
{
  LDMA_StopTransfer(IMU_LDMA_CHANNEL);
  app_log("[LDMA] Stopped\n");
}

// Called from DMA interrupt or polling — marks buffer ready
// and switches to the other buffer
void ldma_buffer_swap(void)
{
  if (using_buffer_A) {
    if (buffer_A_ready) {
      buffer_overflow_count++;
      app_log("[LDMA] Buffer A overflow!\n");
    }
    buffer_A_ready = true;
    using_buffer_A = false;
  } else {
    if (buffer_B_ready) {
      buffer_overflow_count++;
      app_log("[LDMA] Buffer B overflow!\n");
    }
    buffer_B_ready = true;
    using_buffer_A = true;
  }
}

// Returns pointer to whichever buffer is ready, or NULL
imu_sample_t* ldma_buffer_get_ready(void)
{
  if (buffer_A_ready) return imu_buffer_A;
  if (buffer_B_ready) return imu_buffer_B;
  return NULL;
}

// Release buffer after processing
void ldma_buffer_release(imu_sample_t *buf)
{
  if (buf == imu_buffer_A) buffer_A_ready = false;
  if (buf == imu_buffer_B) buffer_B_ready = false;
}