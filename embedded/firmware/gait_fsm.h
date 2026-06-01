#ifndef GAIT_FSM_H
#define GAIT_FSM_H

#include <stdint.h>
#include <stdbool.h>

typedef enum {
  GAIT_NORMAL     = 0,
  GAIT_PRE_FREEZE = 1,
  GAIT_FREEZE     = 2
} gait_state_t;

typedef struct {
  gait_state_t current_state;
  gait_state_t previous_state;
  uint8_t      consecutive_count;
  uint8_t      hysteresis_threshold;  // default 3
  float        confidence_threshold;  // default 0.75
  bool         state_changed;
} gait_fsm_t;

void gait_fsm_init(gait_fsm_t *fsm);
void gait_fsm_update(gait_fsm_t *fsm,
                     float p_normal,
                     float p_pre_freeze,
                     float p_freeze);
const char* gait_state_to_string(gait_state_t state);

#endif // GAIT_FSM_H