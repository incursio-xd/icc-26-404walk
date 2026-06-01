#include "sliding_window.h"
#include "app_log.h"
#include <string.h>
#include <math.h>

normalized_window_t inference_window;
volatile bool window_ready = false;

// Ring buffer for continuous sampling
static float ring_buffer[NUM_FEATURES * 2];  // double size for overlap
static uint32_t ring_head = 0;
static uint32_t samples_collected = 0;

void sliding_window_init(void)
{
  memset(&inference_window, 0, sizeof(inference_window));
  memset(ring_buffer, 0, sizeof(ring_buffer));
  ring_head = 0;
  samples_collected = 0;
  window_ready = false;
  app_log("[SW] Sliding window initialized (%d samples, %d stride)\n",
          WINDOW_SAMPLES, WINDOW_STRIDE);
}

void sliding_window_feed(imu_sample_t *samples, uint32_t count)
{
  for (uint32_t i = 0; i < count; i++) {
    uint32_t base = (ring_head % WINDOW_SAMPLES) * NUM_AXES;
    ring_buffer[base + 0] = (float)samples[i].accel_x;
    ring_buffer[base + 1] = (float)samples[i].accel_y;
    ring_buffer[base + 2] = (float)samples[i].accel_z;
    ring_buffer[base + 3] = (float)samples[i].gyro_x;
    ring_buffer[base + 4] = (float)samples[i].gyro_y;
    ring_buffer[base + 5] = (float)samples[i].gyro_z;
    ring_head++;
    samples_collected++;

    // Trigger inference every WINDOW_STRIDE new samples
    if (samples_collected >= WINDOW_SAMPLES &&
        (samples_collected - WINDOW_SAMPLES) % WINDOW_STRIDE == 0) {
      if (!window_ready) {
        // Copy current window into inference buffer
        for (uint32_t s = 0; s < WINDOW_SAMPLES; s++) {
          uint32_t idx = ((ring_head - WINDOW_SAMPLES + s) % WINDOW_SAMPLES) * NUM_AXES;
          for (uint32_t ax = 0; ax < NUM_AXES; ax++) {
            inference_window.data[s * NUM_AXES + ax] = ring_buffer[idx + ax];
          }
        }
        sliding_window_normalize();
        window_ready = true;
      }
    }
  }
}

void sliding_window_normalize(void)
{
  // Per-axis z-score normalization
  for (uint32_t ax = 0; ax < NUM_AXES; ax++) {
    // Calculate mean
    float mean = 0.0f;
    for (uint32_t s = 0; s < WINDOW_SAMPLES; s++) {
      mean += inference_window.data[s * NUM_AXES + ax];
    }
    mean /= WINDOW_SAMPLES;

    // Calculate std deviation
    float variance = 0.0f;
    for (uint32_t s = 0; s < WINDOW_SAMPLES; s++) {
      float diff = inference_window.data[s * NUM_AXES + ax] - mean;
      variance += diff * diff;
    }
    float std = sqrtf(variance / WINDOW_SAMPLES);
    if (std < 1e-6f) std = 1e-6f;  // prevent division by zero

    // Normalize
    for (uint32_t s = 0; s < WINDOW_SAMPLES; s++) {
      inference_window.data[s * NUM_AXES + ax] =
        (inference_window.data[s * NUM_AXES + ax] - mean) / std;
    }
  }
}