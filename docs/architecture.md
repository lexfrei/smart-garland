# Architecture Decisions

## Overview

Smart garland with Thread/Matter support for ESP32-C6/H2.

**Language**: Pure Rust (no_std, async)

## Decision Records

- [ADR-001: Matter SDK Selection](decisions/001-matter-sdk.md) — rs-matter (Rust) with custom Thread glue
- [ADR-002: Thread Configuration](decisions/002-thread-config.md) — Thread-only networking, MTD mode

## Hardware Decisions

| Decision | Choice | Rationale |
|----------|--------|-----------|
| Dev chip | ESP32-C6 | WiFi+Thread, better tooling, more RAM |
| Prod chip | ESP32-H2 | Thread-only, lower power, cheaper |
| LED protocol | WS2812B (MVP) | Single-wire, ubiquitous, cheap |
| LED backend | RMT peripheral | Hardware timing, CPU-free updates |

## Software Stack (Rust)

| Layer | Technology | Notes |
|-------|------------|-------|
| Runtime | Embassy | Async, no FreeRTOS |
| Matter | rs-matter | Pure Rust Matter implementation |
| Thread | esp-openthread | Rust bindings for OpenThread |
| Radio | esp-ieee802154 | 802.15.4 driver (pure Rust) |
| HAL | esp-hal | Hardware abstraction |
| Build | Cargo + espflash | Standard Rust tooling |

## Component Architecture

```
┌─────────────────────────────────────────────────┐
│              Application Code                    │
│  (light control, effects, LED driver)           │
└─────────────────────┬───────────────────────────┘
                      │
┌─────────────────────▼───────────────────────────┐
│              rs-matter                           │
│  (On/Off, Level, Color, Identify clusters)      │
└─────────────────────┬───────────────────────────┘
                      │
┌─────────────────────▼───────────────────────────┐
│         thread-transport (CUSTOM GLUE)          │
│  (UDP adapter between rs-matter and OpenThread) │
└─────────────────────┬───────────────────────────┘
                      │
┌─────────────────────▼───────────────────────────┐
│              esp-openthread                      │
│  (Thread MTD, IPv6 networking)                  │
└─────────────────────┬───────────────────────────┘
                      │
┌─────────────────────▼───────────────────────────┐
│              esp-ieee802154                      │
│  (802.15.4 radio driver)                        │
└─────────────────────┬───────────────────────────┘
                      │
┌─────────────────────▼───────────────────────────┐
│              Embassy                             │
│  (async executor, timers, HAL)                  │
└─────────────────────────────────────────────────┘
```

## Matter Implementation

**Device Type**: Extended Color Light (0x010D)

### Clusters (MVP)

| Cluster | ID | Purpose |
|---------|-----|---------|
| On/Off | 0x0006 | Power control |
| Level Control | 0x0008 | Brightness (0-254) |
| Color Control | 0x0300 | HSV color mode |
| Identify | 0x0003 | Visual identification during pairing |

### Clusters (Future)

| Cluster | ID | Purpose |
|---------|-----|---------|
| Scenes | 0x0005 | Effect presets (rainbow, fade, etc.) |

## Networking

- **Thread only** — no WiFi even on C6
- **MTD mode** — Minimal Thread Device (end device only)
- Requires Thread Border Router (Apple TV, HomePod, Google Nest Hub)
- BLE for commissioning

## Implementation Phases

### Phase 1: Thread Networking MVP
- Setup Rust Embassy project for ESP32-C6
- Get esp-openthread running (MTD mode)
- Join Thread network, verify connectivity

### Phase 2: Thread Transport Glue (KEY WORK)
- Study rs-matter transport trait requirements
- Implement ThreadTransport using esp-openthread UDP
- Handle IPv6 multicast for Matter discovery
- Test with Matter controller

### Phase 3: Matter Light Device
- Implement Extended Color Light endpoint
- On/Off, Level, Color Control clusters
- BLE commissioning flow

### Phase 4: LED Driver
- WS2812B driver using esp-hal RMT
- HSV color support
- Brightness control

### Phase 5: Polish
- Effects engine (rainbow, fade, etc.)
- Scenes cluster for effect presets
- ESP32-H2 support
- Power optimization

## Known Issues

| Issue | Mitigation |
|-------|------------|
| rs-matter API unstable | Pin to specific git commit |
| Thread glue missing | Write custom ThreadTransport (~100-500 lines) |
| BLE commissioning | May need esp-wifi crate for BLE |
| Debugging async | Use defmt + probe-rs |

## References

- [Research: Rust for Matter/Thread](research-rust-matter-thread.md)
- [rs-matter](https://github.com/project-chip/rs-matter)
- [esp-openthread](https://github.com/esp-rs/esp-openthread)
- [Embassy](https://embassy.dev/)
