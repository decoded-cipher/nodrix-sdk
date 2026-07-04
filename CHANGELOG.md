# Changelog

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
