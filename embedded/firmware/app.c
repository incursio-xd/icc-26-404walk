#include "sl_bt_api.h"
#include "sl_main_init.h"
#include "app_assert.h"
#include "app_log.h"
#include "app.h"
#include "icm42688p.h"
#include "sl_i2cspm_instances.h"
#include "ldma_buffer.h"
#include "sliding_window.h"
#include "gait_fsm.h"
#include "feedback.h"

static uint8_t advertising_set_handle = 0xff;
static uint8_t ble_connection_handle  = 0xff;
static bool    imu_ready              = false;

static gait_fsm_t gait_fsm;

void app_init(void)
{
  app_log("\n=== FogGuard Starting ===\n");

  if (icm42688p_init(sl_i2cspm_imu) == SL_STATUS_OK) {
    imu_ready = true;
    app_log("[IMU] OK\n");
  } else {
    app_log("[IMU] Init failed - check wiring\n");
  }

  ldma_buffer_init();
  ldma_buffer_start();
  sliding_window_init();
  gait_fsm_init(&gait_fsm);
  feedback_init();
}

void app_process_action(void)
{
  if (!app_is_process_required() || !imu_ready) return;

  // Poll IMU and feed into sliding window
  icm42688p_raw_t raw;
  if (icm42688p_read_raw(sl_i2cspm_imu, &raw) == SL_STATUS_OK) {
    imu_sample_t sample = {
      .accel_x = raw.accel_x,
      .accel_y = raw.accel_y,
      .accel_z = raw.accel_z,
      .gyro_x  = raw.gyro_x,
      .gyro_y  = raw.gyro_y,
      .gyro_z  = raw.gyro_z
    };
    sliding_window_feed(&sample, 1);
  }

  // If a window is ready, run inference (stub for now)
  if (window_ready) {
    window_ready = false;

    // TODO Week 7-8: replace with real TFLite Micro inference
    // Stub: simulate normal gait
    float p_normal     = 1.0f;
    float p_pre_freeze = 0.0f;
    float p_freeze     = 0.0f;

    gait_fsm_update(&gait_fsm, p_normal, p_pre_freeze, p_freeze);

    if (gait_fsm.state_changed) {
      feedback_trigger(gait_fsm.current_state);
      app_log("[APP] State: %s\n",
              gait_state_to_string(gait_fsm.current_state));

      // TODO Week 3-4: send BLE notification with state + confidence
    }
  }
}

void sl_bt_on_event(sl_bt_msg_t *evt)
{
  sl_status_t sc;

  switch (SL_BT_MSG_ID(evt->header)) {

    case sl_bt_evt_system_boot_id:
      sc = sl_bt_advertiser_create_set(&advertising_set_handle);
      app_assert_status(sc);
      sc = sl_bt_legacy_advertiser_generate_data(advertising_set_handle,
             sl_bt_advertiser_general_discoverable);
      app_assert_status(sc);
      sc = sl_bt_advertiser_set_timing(advertising_set_handle,
             160, 160, 0, 0);
      app_assert_status(sc);
      sc = sl_bt_legacy_advertiser_start(advertising_set_handle,
             sl_bt_legacy_advertiser_connectable);
      app_assert_status(sc);
      app_log("[BLE] Advertising as FogGuard\n");
      break;

    case sl_bt_evt_connection_opened_id:
      ble_connection_handle = evt->data.evt_connection_opened.connection;
      app_log("[BLE] Connected (handle=%d)\n", ble_connection_handle);
      break;

    case sl_bt_evt_connection_closed_id:
      ble_connection_handle = 0xff;
      app_log("[BLE] Disconnected (reason=0x%X)\n",
              evt->data.evt_connection_closed.reason);
      sc = sl_bt_legacy_advertiser_generate_data(advertising_set_handle,
             sl_bt_advertiser_general_discoverable);
      app_assert_status(sc);
      sc = sl_bt_legacy_advertiser_start(advertising_set_handle,
             sl_bt_legacy_advertiser_connectable);
      app_assert_status(sc);
      break;

    case sl_bt_evt_gatt_server_characteristic_status_id:
      app_log("[BLE] GATT characteristic status changed\n");
      break;

    default:
      break;
  }
}