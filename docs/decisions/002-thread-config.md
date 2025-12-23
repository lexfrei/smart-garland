# ADR-002: Thread Networking Configuration

**Status:** Accepted (Updated)
**Date:** 2024-12-23
**Updated:** 2024-12-23

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

### Thread Device Role: MTD Only

| Chip | Role | Rationale |
|------|------|-----------|
| ESP32-C6 | MTD (Minimal Thread Device) | End device, doesn't need routing |
| ESP32-H2 | MTD (Minimal Thread Device) | End device, lower power |

**Updated**: Originally planned FTD for C6, but:
- Garland is end device, doesn't route traffic
- MTD is sufficient and simpler
- esp-openthread has better MTD support
- Unified behavior across chips

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

## Configuration (Rust)

Thread configuration via esp-openthread in Rust code:

```rust
// Cargo.toml
[dependencies]
esp-openthread = { git = "https://github.com/esp-rs/esp-openthread", features = ["mtd"] }

// main.rs
let config = ThreadConfig {
    role: ThreadRole::MinimalEndDevice,
    // Network credentials from commissioning
};
```

No sdkconfig files needed — pure Rust configuration.

## Thread Network Parameters

Network credentials received during Matter commissioning:
- Channel, PAN ID, Network Key — from Border Router
- No hardcoded defaults needed

## Consequences

### Positive
- Unified codebase for C6 and H2 (both MTD)
- Lower power consumption
- Simpler debugging (one protocol, one role)
- Works with existing Apple/Google Thread infrastructure
- Pure Rust configuration

### Negative
- Requires Thread Border Router in user's home
- Cannot fall back to WiFi if Thread unavailable
- Initial setup requires BLE-capable phone
