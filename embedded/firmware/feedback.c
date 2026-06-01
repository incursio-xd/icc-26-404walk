#include "feedback.h"
#include "app_log.h"
#include "sl_simple_led_instances.h"

// TODO Week 9-10: Replace with actual PWM/TIMER driver
// These are stubs until hardware is connected

static haptic_pattern_t current_haptic = HAPTIC_OFF;
static audio_pattern_t  current_audio  = AUDIO_OFF;

void feedback_init(void)
{
  current_haptic = HAPTIC_OFF;
  current_audio  = AUDIO_OFF;
  app_log("[FB] Feedback system initialized\n");
}

void haptic_set(haptic_pattern_t pattern)
{
  if (pattern == current_haptic) return;
  current_haptic = pattern;

  switch (pattern) {
    case HAPTIC_OFF:
      // TODO: Stop PWM on LRA pin
      app_log("[FB] Haptic OFF\n");
      break;
    case HAPTIC_GENTLE_RHYTHMIC:
      // TODO: PWM 150Hz, 30% duty, 1Hz on/off rhythm
      app_log("[FB] Haptic GENTLE RHYTHMIC\n");
      sl_led_turn_on(&sl_led_led0);  // LED as placeholder
      break;
    case HAPTIC_STRONG_URGENT:
      // TODO: PWM 175Hz, 80% duty, continuous
      app_log("[FB] Haptic STRONG URGENT\n");
      sl_led_turn_on(&sl_led_led0);
      break;
  }
}

void audio_set(audio_pattern_t pattern)
{
  if (pattern == current_audio) return;
  current_audio = pattern;

  switch (pattern) {
    case AUDIO_OFF:
      // TODO: GPIO low on buzzer pin
      app_log("[FB] Audio OFF\n");
      break;
    case AUDIO_SOFT_METRONOME:
      // TODO: 1Hz beep, 50ms pulse
      app_log("[FB] Audio SOFT METRONOME\n");
      break;
    case AUDIO_URGENT_ALERT:
      // TODO: 4Hz beep, 100ms pulse
      app_log("[FB] Audio URGENT ALERT\n");
      break;
  }
}

void feedback_trigger(gait_state_t state)
{
  switch (state) {
    case GAIT_NORMAL:
      feedback_stop_all();
      break;
    case GAIT_PRE_FREEZE:
      haptic_set(HAPTIC_GENTLE_RHYTHMIC);
      audio_set(AUDIO_SOFT_METRONOME);
      break;
    case GAIT_FREEZE:
      haptic_set(HAPTIC_STRONG_URGENT);
      audio_set(AUDIO_URGENT_ALERT);
      // TODO Week 3-4: trigger BLE caregiver alert
      break;
  }
}

void feedback_stop_all(void)
{
  haptic_set(HAPTIC_OFF);
  audio_set(AUDIO_OFF);
  sl_led_turn_off(&sl_led_led0);
}