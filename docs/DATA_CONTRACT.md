# FogGuard Data Contract

## BLE → Raspberry Pi → Dashboard JSON Format

### Real-time Inference Payload (every ~1.5s)

```json
{
  "schema_version": 1,
  "device_id": "fogguard-001",
  "timestamp": 1748689200000,
  "gait_state": "pre-freeze",
  "gait_state_code": 1,
  "confidence": 0.87,
  "imu": {
    "accel": [0.12, -0.05, 9.81],
    "gyro": [1.2, -0.3, 0.8]
  },
  "intervention_fired": true,
  "battery_pct": 82,
  "consecutive_count": 2
}
```

### Freeze Episode Log (stored in MongoDB)

```json
{
  "schema_version": 1,
  "event_id": "evt_1748689200000",
  "device_id": "fogguard-001",
  "event_type": "freeze",
  "start_timestamp": 1748689200000,
  "end_timestamp": 1748689215000,
  "duration_ms": 15000,
  "peak_confidence": 0.94,
  "intervention_fired": true,
  "caregiver_alerted": true
}
```

---

## Field Reference

| Field | Type | Description |
|---|---|---|
| `schema_version` | int | Contract version — increment on breaking changes |
| `device_id` | string | Unique device identifier |
| `timestamp` | int64 | Unix timestamp in milliseconds (UTC) |
| `gait_state` | string | Human-readable state for display |
| `gait_state_code` | int | Integer code for logic/chart coloring (0/1/2) |
| `confidence` | float | Winning class probability (0.0–1.0) |
| `imu.accel` | float[3] | Accelerometer [x, y, z] in g |
| `imu.gyro` | float[3] | Gyroscope [x, y, z] in dps |
| `intervention_fired` | bool | True if haptic/audio was triggered this cycle |
| `battery_pct` | int | Battery percentage (0–100) |
| `consecutive_count` | int | Consecutive windows classified as current state |

---

## Gait States

| Code | String | On-Device Action | BLE Action |
|---|---|---|---|
| 0 | `normal` | Nothing | No alert |
| 1 | `pre-freeze` | Gentle vibration + soft audio | No alert |
| 2 | `freeze` | Strong vibration + urgent audio | Caregiver alert |

> **Note:** `intervention_fired` can be false even when `gait_state` is `pre-freeze` or `freeze` if the confidence was below the on-device threshold (0.75) or hysteresis (3 consecutive windows) was not met.

---

## BLE GATT Characteristics

| Characteristic | Size | Properties | Update Rate |
|---|---|---|---|
| Gait State | 1 byte (uint8) | Notify | Every state change |
| Confidence Score | 4 bytes (float32) | Notify | Every inference (~1.5s) |
| IMU Raw Data | 12 bytes (6 × int16) | Notify | Every inference (~1.5s) |
| Device Status | 1 byte (uint8) | Read | On request |

---

## Socket.io Events (Dashboard)

```js
// Real-time inference result
socket.emit('gait_update', payload)

// Freeze episode started
socket.emit('freeze_start', { device_id, timestamp, confidence })

// Freeze episode ended
socket.emit('freeze_end', { device_id, timestamp, duration_ms })

// Device online/offline
socket.emit('device_status', { device_id, online: true, battery_pct: 82 })
```

---

## MongoDB Collections

| Collection | Description | Key fields |
|---|---|---|
| `gait_events` | One document per inference cycle | device_id, timestamp, gait_state_code, confidence |
| `freeze_episodes` | One document per complete freeze event | start_timestamp, end_timestamp, duration_ms |
| `devices` | One document per device | device_id, last_seen, battery_pct, online |

---

## IMU Axis Order

Axes are always in this fixed order — firmware and model both depend on this:

```
Index 0: accel_x
Index 1: accel_y
Index 2: accel_z
Index 3: gyro_x
Index 4: gyro_y
Index 5: gyro_z
```

---

## On-Device Inference Parameters (for ML team reference)

| Parameter | Value |
|---|---|
| Sampling rate | 100 Hz |
| Window size | 300 samples (3 seconds) |
| Window stride | 150 samples (1.5 seconds) |
| Input tensor shape | [300, 6] flattened to 1800 floats |
| Normalization | Per-axis z-score (applied on-device before inference) |
| Confidence threshold | 0.75 (results below this are ignored) |
| Hysteresis | 3 consecutive windows required for state transition |
