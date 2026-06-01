#include "gait_fsm.h"
#include "app_log.h"
#include <string.h>

void gait_fsm_init(gait_fsm_t *fsm)
{
  fsm->current_state       = GAIT_NORMAL;
  fsm->previous_state      = GAIT_NORMAL;
  fsm->consecutive_count   = 0;
  fsm->hysteresis_threshold = 3;
  fsm->confidence_threshold = 0.75f;
  fsm->state_changed        = false;
}

void gait_fsm_update(gait_fsm_t *fsm,
                     float p_normal,
                     float p_pre_freeze,
                     float p_freeze)
{
  fsm->state_changed = false;

  // Find winning class
  gait_state_t candidate = GAIT_NORMAL;
  float max_prob = p_normal;

  if (p_pre_freeze > max_prob) {
    candidate = GAIT_PRE_FREEZE;
    max_prob  = p_pre_freeze;
  }
  if (p_freeze > max_prob) {
    candidate = GAIT_FREEZE;
    max_prob  = p_freeze;
  }

  // Confidence gate
  if (max_prob < fsm->confidence_threshold) {
    app_log("[FSM] Low confidence (%.2f) — holding state\n", max_prob);
    return;
  }

  // Hysteresis — require N consecutive windows
  if (candidate == fsm->current_state) {
    fsm->consecutive_count = 0;
    return;
  }

  fsm->consecutive_count++;
  if (fsm->consecutive_count >= fsm->hysteresis_threshold) {
    fsm->previous_state    = fsm->current_state;
    fsm->current_state     = candidate;
    fsm->consecutive_count = 0;
    fsm->state_changed     = true;
    app_log("[FSM] State: %s → %s (conf=%.2f)\n",
            gait_state_to_string(fsm->previous_state),
            gait_state_to_string(fsm->current_state),
            max_prob);
  }
}

const char* gait_state_to_string(gait_state_t state)
{
  switch (state) {
    case GAIT_NORMAL:     return "NORMAL";
    case GAIT_PRE_FREEZE: return "PRE-FREEZE";
    case GAIT_FREEZE:     return "FREEZE";
    default:              return "UNKNOWN";
  }
}