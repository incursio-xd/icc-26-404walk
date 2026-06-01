#ifndef FEEDBACK_H
#define FEEDBACK_H

#include "gait_fsm.h"

typedef enum {
  HAPTIC_OFF            = 0,
  HAPTIC_GENTLE_RHYTHMIC = 1,
  HAPTIC_STRONG_URGENT   = 2
} haptic_pattern_t;

typedef enum {
  AUDIO_OFF            = 0,
  AUDIO_SOFT_METRONOME  = 1,
  AUDIO_URGENT_ALERT    = 2
} audio_pattern_t;

void feedback_init(void);
void feedback_trigger(gait_state_t state);
void feedback_stop_all(void);
void haptic_set(haptic_pattern_t pattern);
void audio_set(audio_pattern_t pattern);

#endif // FEEDBACK_H