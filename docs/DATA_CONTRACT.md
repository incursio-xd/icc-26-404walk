# FogGuard Data Contract

## BLE → Raspberry Pi → Dashboard JSON Format

```json
{
  "device_id": "fogguard-001",
  "timestamp": 1716900000000,
  "gait_state": "pre-freeze",
  "confidence": 0.87,
  "imu": {
    "accel": [0.12, -0.05, 9.81],
    "gyro":  [1.2, -0.3, 0.8]
  },
  "intervention_fired": true,
  "battery_pct": 82
}
```

## Gait States
- `normal` → No action
- `pre-freeze` → Gentle vibration + soft audio
- `freeze` → Strong vibration + urgent audio + BLE alert

## BLE GATT Characteristics
| Characteristic | UUID | Size | Direction |
|---|---|---|---|
| Gait State | auto-generated | 1 byte | Notify |
| Confidence Score | auto-generated | 4 bytes | Notify |
| IMU Raw Data | auto-generated | 12 bytes | Notify |
| Device Status | auto-generated | 1 byte | Read |