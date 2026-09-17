# Changelog

## 0.2.0 — 2026-09-18

- `setFirmwareVersion()` names the version a sketch is. Nodrix compares it
  against the firmware assigned to the device, so it must match the string typed
  when the image was uploaded. Without one, over-the-air updates are now skipped
  with a message under `setDebug(true)` — previously the board downloaded an
  update the cloud could never see land, and reinstalled it after every reboot.
- The chip is reported on its own (`ESP.getChipModel()` on ESP32, `esp8266`
  otherwise), so a board names itself in the dashboard without `setChip()`.
- Dropped the `NODRIX_BUILD` build-id define, which only the browser build agent
  set.
- New `OtaUpdate` example: reports a version, updates itself, and can be asked
  to check from a dashboard button.

## 0.1.1 — 2026-09-16

- Debug output now tells connection failures apart: failed HTTP requests log the
  status code with a reason (token rejected, no access, wrong host, rate limited,
  server error, no connection), a refused WebSocket handshake is reported separately
  from a dropped link, and socket errors are no longer swallowed. All behind
  `setDebug(true)`.
- Install from the Arduino Library Manager, PlatformIO Registry, or ESP Component
  Registry; added ESP-IDF instructions.

## 0.1.0 — 2026-07-05

First release.

- `NODRIX_WRITE("var") { ... }` handlers for cloud control writes, with automatic
  acks and typed value coercion — `asBool()`, `asInt()`, `asLong()`, `asFloat()`,
  `asDouble()`, `asString()`, `isNull()`.
- `Nodrix.send()` telemetry with batching; overloads for `bool`, `int`, `long`,
  `float`, `double`, `const char*`, and `String`.
- Two transports: always-on WebSocket (`begin()` / `run()`) and HTTP polling for
  deep-sleep devices (`beginHTTP()` / `poll()`).
- Multiple WiFi networks with failover via `addAP()`.
- Automatic reconnect, heartbeat, and control-variable seeding.
- Optional TLS pinning — `setCACert()` (ESP32) and `setFingerprint()` (ESP8266
  HTTP); unvalidated `setInsecure()` by default.
- Server-side events via `event()`.
- Runtime debug logging via `setDebug()`.
- Examples: LedControl, HomeLights, MultiWiFi, SensorTelemetry, DeepSleepSensor.
- Supports ESP32 (including S2, S3, C3, C6, H2) and ESP8266.
