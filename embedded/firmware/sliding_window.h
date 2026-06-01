#ifndef SLIDING_WINDOW_H
#define SLIDING_WINDOW_H

#include <stdint.h>
#include <stdbool.h>
#include "ldma_buffer.h"

// Window configuration
#define WINDOW_STRIDE        150   // 50% overlap = 1.5s between inferences
#define NUM_FEATURES         (WINDOW_SAMPLES * NUM_AXES)  // 1800 floats

// Normalized window ready for inference
typedef struct {
  float data[NUM_FEATURES];   // z-score normalized, shape [300][6] flattened
  uint32_t timestamp_ms;
} normalized_window_t;

extern normalized_window_t inference_window;
extern volatile bool window_ready;

// API
void sliding_window_init(void);
void sliding_window_feed(imu_sample_t *samples, uint32_t count);
void sliding_window_normalize(void);

#endif // SLIDING_WINDOW_H