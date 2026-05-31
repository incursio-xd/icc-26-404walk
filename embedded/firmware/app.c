#include "sl_bt_api.h"
#include "sl_main_init.h"
#include "app_assert.h"
#include "app_log.h"
#include "app.h"
#include "icm42688p.h"
#include "sl_i2cspm_instances.h"

static uint8_t advertising_set_handle = 0xff;
static uint8_t ble_connection_handle  = 0xff;
static bool    imu_ready              = false;

void app_init(void)
{
  app_log("\n=== FogGuard Starting ===\n");

  if (icm42688p_init(sl_i2cspm_imu) == SL_STATUS_OK) {
    imu_ready = true;
    app_log("[IMU] OK\n");
  } else {
    app_log("[IMU] Init failed - check wiring\n");
  }
}

void app_process_action(void)
{
  if (!app_is_process_required() || !imu_ready) return;

  icm42688p_scaled_t imu;
  if (icm42688p_read_scaled(sl_i2cspm_imu, &imu) == SL_STATUS_OK) {
    app_log("[IMU] AX:%6.3f AY:%6.3f AZ:%6.3f | GX:%7.3f GY:%7.3f GZ:%7.3f\n",
            imu.accel_x, imu.accel_y, imu.accel_z,
            imu.gyro_x,  imu.gyro_y,  imu.gyro_z);
  } else {
    app_log("[IMU] Read failed\n");
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