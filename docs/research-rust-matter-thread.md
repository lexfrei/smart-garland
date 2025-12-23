# Rust for Matter/Thread on ESP32

Research date: 2025-12-23

## Summary

Rust support for Matter/Thread on ESP32 is **not production-ready**. Matter over WiFi works, but Thread integration is incomplete.

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
```

## Key Projects

### rs-matter

- **Repository**: <https://github.com/project-chip/rs-matter>
- **Status**: Experimental, but functional with major controllers
- **Features**:
  - Pure Rust, `no_std`, no-alloc, async-first
  - Works with Google Home, Apple HomeKit, Alexa, Home Assistant
  - Scales from 1MB flash / 256KB RAM to Linux
- **Hardware**: ESP32, Nordic NRF52840, RP2040
- **Limitation**: APIs still changing, backwards-incompatible changes expected

### esp-idf-matter

- **Repository**: <https://github.com/sysgrok/esp-idf-matter>
- **Purpose**: Run rs-matter on ESP-IDF (with Rust std library)
- **Transport**: WiFi main, BLE for commissioning
- **Chips**: ESP32, ESP32-S3, ESP32-C3, ESP32-C6
- **Thread**: Not implemented

### rs-matter-embassy

- **Repository**: <https://github.com/sysgrok/rs-matter-embassy>
- **Purpose**: Run rs-matter with Embassy (bare-metal, no_std)
- **Transport**: WiFi and Ethernet implemented, Thread planned
- **Status**: Early-to-moderate maturity
- **Limitation**: Thread support listed as "future" capability

### esp-openthread

- **Repository**: <https://github.com/esp-rs/esp-openthread>
- **Purpose**: OpenThread Rust bindings for ESP32
- **Supported chips**: ESP32-C6, ESP32-H2 (via esp-ieee802154)
- **Status**: Pre-release, examples work
- **Implemented**: MTD (Minimal Thread Device), native UDP sockets, embassy-net integration
- **Planned**: Sleepy end-device, FTD (Full Thread Device)
- **Out of scope**: Thread Border Router

### esp-ieee802154

- **Part of**: esp-hal (<https://github.com/esp-rs/esp-hal>)
- **Status**: Working, open source
- **Purpose**: Low-level 802.15.4 radio driver for ESP32-C6/H2

### Embassy

- **Repository**: <https://github.com/embassy-rs/embassy>
- **Website**: <https://embassy.dev/>
- **Purpose**: Modern async embedded framework for Rust
- **Role**: Alternative to RTOS (like FreeRTOS), provides HAL and async runtime

## Component Status Matrix

| Component | Status | Notes |
|-----------|--------|-------|
| rs-matter | ✅ Working | WiFi + BLE commissioning |
| esp-idf-matter | ✅ Working | WiFi only |
| rs-matter-embassy | ⚠️ Partial | WiFi works, Thread planned |
| esp-openthread | ⚠️ MTD only | FTD in development |
| esp-ieee802154 | ✅ Working | Low-level radio driver |
| **Matter + Thread** | ❌ Not ready | Integration not complete |

## Implications for This Project

### Current Decision: Use C (ESP-Matter)

The project uses ESP-Matter (C SDK) because:

1. **Thread support is incomplete in Rust** — esp-openthread only supports MTD, not FTD needed for ESP32-C6
2. **rs-matter + Thread integration doesn't exist** — only WiFi transport is implemented
3. **ESP-Matter is production-ready** — official Espressif SDK, full Thread/Matter support
4. **Timeline** — Rust ecosystem may take 6-12+ months to mature

### Future Consideration: Rust Migration

Worth revisiting when:

- esp-openthread supports FTD
- rs-matter-embassy implements Thread transport
- Someone creates rs-matter + esp-openthread integration

### Alternative: Contribute

If Rust is strongly preferred, contributing to these projects is an option:

- esp-openthread: Add FTD support
- rs-matter-embassy: Implement Thread transport
- Create glue code between rs-matter and esp-openthread

## References

- [rs-matter GitHub](https://github.com/project-chip/rs-matter)
- [esp-openthread GitHub](https://github.com/esp-rs/esp-openthread)
- [rs-matter-embassy GitHub](https://github.com/sysgrok/rs-matter-embassy)
- [esp-idf-matter GitHub](https://github.com/sysgrok/esp-idf-matter)
- [Embassy website](https://embassy.dev/)
- [ESP Matter Solution (official C SDK)](https://www.espressif.com/en/solutions/device-connectivity/esp-matter-solution)
- [esp-rs/esp-idf-hal Matter issue #212](https://github.com/esp-rs/esp-idf-hal/issues/212)
