# Rust for Matter/Thread on ESP32

Research date: 2025-12-23 (updated)

## Summary

Rust support for Matter/Thread on ESP32 is **close but not quite there**. The key blocker is the **missing glue between rs-matter and esp-openthread** — both pieces exist, but nobody has connected them yet.

**Key insight**: If using MTD (Minimal Thread Device) instead of FTD, the esp-openthread limitation is not a blocker — MTD is sufficient for a garland/light device.

## Technology Stack Overview

```
┌─────────────────────────────────────┐
│  rs-matter (pure Rust Matter)       │  ← project-chip/rs-matter
├─────────────────────────────────────┤
│  rs-matter-stack (integrations)     │
├──────────────────┬──────────────────┤
│ esp-idf-matter   │ rs-matter-embassy│  ← two runtime options
│ (ESP-IDF + std)  │ (Embassy + no_std)│
├──────────────────┴──────────────────┤
│        esp-openthread (Thread)       │  ← esp-rs/esp-openthread
├─────────────────────────────────────┤
│        esp-ieee802154 (radio)        │  ← 802.15.4 driver
└─────────────────────────────────────┘
         ↑↑↑ MISSING GLUE ↑↑↑
```

## MTD vs FTD Clarification

| Type | Full Name             | Role                     | Power          | Use Case                   |
|------|-----------------------|--------------------------|----------------|----------------------------|
| MTD  | Minimal Thread Device | End device only          | Low, can sleep | Sensors, lights, switches  |
| FTD  | Full Thread Device    | Can become Router/Leader | High, always on| Hubs, plugs, border routers|

**For a garland**: MTD is sufficient. The device doesn't need to route traffic for other devices.

## Key Projects

### rs-matter

- **Repository**: <https://github.com/project-chip/rs-matter>
- **Status**: Experimental, but functional with major controllers
- **Features**:
  - Pure Rust, `no_std`, no-alloc, async-first
  - Works with Google Home, Apple HomeKit, Alexa, Home Assistant
  - Scales from 1MB flash / 256KB RAM to Linux
  - Modular: transport layer can be swapped
- **Hardware**: ESP32, Nordic NRF52840, RP2040
- **Limitation**: APIs still changing, backwards-incompatible changes expected

### esp-idf-matter

- **Repository**: <https://github.com/sysgrok/esp-idf-matter>
- **Purpose**: Run rs-matter on ESP-IDF (with Rust std library)
- **Transport**: WiFi main, BLE for commissioning
- **Chips**: ESP32, ESP32-S3, ESP32-C3, ESP32-C6
- **Thread**: **NOT implemented** (despite being mentioned in docs, no actual code exists)

### rs-matter-embassy

- **Repository**: <https://github.com/sysgrok/rs-matter-embassy>
- **Purpose**: Run rs-matter with Embassy (bare-metal, no_std)
- **Transport**: WiFi and Ethernet implemented
- **Thread**: Listed as "future" — **NOT implemented**
- **Status**: Early-to-moderate maturity

### esp-openthread

- **Repository**: <https://github.com/esp-rs/esp-openthread>
- **Purpose**: OpenThread Rust bindings for ESP32
- **Supported chips**: ESP32-C6, ESP32-H2 (via esp-ieee802154)
- **Status**: Pre-release, examples work
- **Implemented**:
  - ✅ MTD (Minimal Thread Device) — **sufficient for garland**
  - ✅ Native UDP sockets
  - ✅ embassy-net integration
  - ✅ SRP (Service Registration Protocol)
- **Planned**: Sleepy end-device, FTD
- **Out of scope**: Thread Border Router

### esp-ieee802154

- **Part of**: esp-hal (<https://github.com/esp-rs/esp-hal>)
- **Status**: ✅ Working, fully open source
- **Purpose**: Low-level 802.15.4 radio driver for ESP32-C6/H2

### Embassy

- **Repository**: <https://github.com/embassy-rs/embassy>
- **Website**: <https://embassy.dev/>
- **Purpose**: Modern async embedded framework for Rust
- **Role**: Alternative to RTOS (like FreeRTOS), provides HAL and async runtime

## Component Status Matrix (for MTD use case)

| Component | Status | Notes |
|-----------|--------|-------|
| rs-matter | ✅ Working | WiFi + BLE commissioning proven |
| esp-idf-matter | ✅ WiFi only | Thread transport not implemented |
| rs-matter-embassy | ✅ WiFi only | Thread transport not implemented |
| esp-openthread | ✅ MTD works | UDP, embassy-net, SRP all working |
| esp-ieee802154 | ✅ Working | Low-level radio driver |
| **rs-matter + Thread** | ❌ No glue | Both sides ready, integration missing |

## The Missing Piece

Both stacks work independently:

1. **rs-matter** can do Matter over any IPv6 transport (proven with WiFi)
2. **esp-openthread** provides Thread networking with UDP sockets and embassy-net

**What's missing**: Someone needs to write the glue code that:

- Initializes esp-openthread as the network layer
- Provides UDP socket implementation to rs-matter
- Handles Thread commissioning flow

This is likely **days to weeks of work**, not months — the hard parts are done.

## Implications for This Project

### Option A: Use C (ESP-Matter) — Current choice

**Pros**:

- Production-ready today
- Official Espressif support
- Full documentation

**Cons**:

- C/C++ codebase
- Less type safety

### Option B: Use Rust with custom glue — Possible now

**Required work**:

1. Write Thread transport adapter for rs-matter (~100-500 lines)
2. Test Matter commissioning over Thread
3. Debug any integration issues

**Pros**:

- Pure Rust, memory safe
- Modern async code
- Could contribute back to community

**Cons**:

- Uncharted territory
- Debugging will be harder
- No community support yet

### Option C: Wait for ecosystem

**Timeline**: Unknown, could be weeks or months
**Risk**: Nobody might do it

## Recommendation

For a garland project where MTD is sufficient:

1. **If timeline is flexible**: Option B is viable — the pieces are there
2. **If need production soon**: Option A (C SDK) is safer
3. **Middle ground**: Start with C, plan Rust port when glue exists

## References

- [rs-matter GitHub](https://github.com/project-chip/rs-matter)
- [esp-openthread GitHub](https://github.com/esp-rs/esp-openthread)
- [rs-matter-embassy GitHub](https://github.com/sysgrok/rs-matter-embassy)
- [esp-idf-matter GitHub](https://github.com/sysgrok/esp-idf-matter)
- [Embassy website](https://embassy.dev/)
- [ESP Matter Solution (official C SDK)](https://www.espressif.com/en/solutions/device-connectivity/esp-matter-solution)
- [esp-rs/esp-idf-hal Matter issue #212](https://github.com/esp-rs/esp-idf-hal/issues/212)
- [ESPHome OpenThread (2025.6)](https://www.matteralpha.com/news/esphome-2025-6-adds-openthread-support)
