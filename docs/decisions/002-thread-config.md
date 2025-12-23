# ADR-002: Thread Networking Configuration

**Status:** Accepted
**Date:** 2024-12-23

## Context

Configure Matter networking for ESP32-C6 (development) and ESP32-H2 (production).

## Decision

### Thread-Only Networking

**Disable WiFi completely**, even on ESP32-C6 which supports it.

Rationale:
- Simpler network topology
- Lower power consumption
- Single protocol to debug
- H2 doesn't have WiFi anyway — unified codebase

### Thread Device Roles

| Chip | Role | Rationale |
|------|------|-----------|
| ESP32-C6 | FTD (Full Thread Device) | More RAM, can route |
| ESP32-H2 | MTD (Minimal Thread Device) | Memory constrained, sleepy end device |

### Border Router

User has existing Thread Border Router (Apple TV / HomePod).

No need to implement Border Router functionality — device is end-node only.

### Commissioning

BLE-based commissioning (standard Matter flow):
1. Device advertises via BLE
2. Commissioner (iPhone/Android) connects
3. Thread credentials provisioned
4. Device joins Thread network
5. BLE disabled, Thread-only operation

## Configuration

### sdkconfig.defaults.c6_thread

```
CONFIG_OPENTHREAD_ENABLED=y
CONFIG_OPENTHREAD_FTD=y
CONFIG_ENABLE_WIFI_STATION=n
CONFIG_ENABLE_WIFI_AP=n
```

### sdkconfig.defaults.h2_thread

```
CONFIG_OPENTHREAD_ENABLED=y
CONFIG_OPENTHREAD_MTD=y
```

## Thread Network Parameters

Using defaults for development:
- Channel: 15
- PAN ID: 0x1234
- Network name: "OpenThread"

Production devices will receive credentials during commissioning.

## Consequences

### Positive
- Unified codebase for C6 and H2
- Lower power consumption
- Simpler debugging (one protocol)
- Works with existing Apple/Google Thread infrastructure

### Negative
- Requires Thread Border Router in user's home
- Cannot fall back to WiFi if Thread unavailable
- Initial setup requires BLE-capable phone
