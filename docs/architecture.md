# Architecture

## Overview

Smart garland with Thread/Matter support for ESP32-C6/H2.

**SDK**: ESP-Matter (C++) — official Espressif implementation
**Build**: ESP-IDF v5.2.1 via DevContainer

## Decision Records

- [ADR-003: ESP-Matter Migration](decisions/003-esp-matter-migration.md) — current approach
- [ADR-002: Thread Configuration](decisions/002-thread-config.md) — Thread-only networking, MTD mode
- ~~[ADR-001: Matter SDK Selection](decisions/001-matter-sdk.md)~~ — superseded by ADR-003

## Hardware Decisions

| Decision | Choice | Rationale |
|----------|--------|-----------|
| Dev chip | ESP32-C6 | WiFi+Thread+BLE, better tooling, more RAM |
| Prod chip | ESP32-H2 | Thread+BLE only, lower power, cheaper |
| LED protocol | WS2812B (MVP) | Single-wire, ubiquitous, cheap |
| LED backend | RMT peripheral | Hardware timing, CPU-free updates |

## Software Stack (ESP-Matter)

| Layer | Technology | Notes |
|-------|------------|-------|
| Runtime | FreeRTOS | ESP-IDF default |
| Matter | ESP-Matter SDK | Official Espressif, production-ready |
| Thread | OpenThread | Integrated in ESP-IDF |
| Radio | esp-ieee802154 | 802.15.4 driver |
| HAL | ESP-IDF drivers | led_strip component for WS2812B |
| Build | CMake + idf.py | Standard ESP-IDF tooling |

## Component Architecture

```
┌─────────────────────────────────────────────────┐
│              Application Code                    │
│  (app_main.cpp, app_driver.cpp)                 │
└─────────────────────┬───────────────────────────┘
                      │
┌─────────────────────▼───────────────────────────┐
│              ESP-Matter SDK                      │
│  (On/Off, Level, Color clusters, endpoints)     │
└─────────────────────┬───────────────────────────┘
                      │
┌─────────────────────▼───────────────────────────┐
│              ESP-IDF Thread Stack               │
│  (OpenThread MTD, IPv6, 6LoWPAN)               │
└─────────────────────┬───────────────────────────┘
                      │
┌─────────────────────▼───────────────────────────┐
│              802.15.4 Radio                     │
│  (Thread networking)                            │
└─────────────────────┬───────────────────────────┘
                      │
┌─────────────────────▼───────────────────────────┐
│              FreeRTOS                           │
│  (tasks, timers, drivers)                       │
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

## Project Structure

```
smart-garland/
├── main/                   # Application code
│   ├── app_main.cpp        # Entry point, Matter setup
│   ├── app_driver.cpp      # LED control implementation
│   ├── app_driver.h        # LED driver interface
│   ├── idf_component.yml   # Dependencies (led_strip)
│   └── CMakeLists.txt
├── scripts/
│   └── dev.sh              # Container helper (compose workflow)
├── CMakeLists.txt          # ESP-IDF build config
├── Makefile                # Main build interface
├── Containerfile           # Build environment image
├── compose.yaml            # Container orchestration (dev)
├── sdkconfig.defaults      # Common ESP-IDF config
├── sdkconfig.defaults.esp32c6  # C6-specific (future)
├── sdkconfig.defaults.esp32h2  # H2-specific (future)
└── partitions.csv          # Flash layout
```

## Build & Flash

```bash
# Build (runs in container, outputs build/smart-garland.bin)
make

# Flash and monitor (runs on host)
make flash-monitor

# Build for H2 instead of C6
make TARGET=esp32h2

# Interactive shell for debugging
make shell
```

## Modular Architecture

**Targets** (compile-time):
- ESP32-C6 (development)
- ESP32-H2 (production)

**Garland variants** (runtime, future):
- Single LED (MVP/testing)
- WS2812B strip (N LEDs)
- SK6812 RGBW strip
- Individual GPIO LEDs

## Implementation Phases

### Phase 1: MVP ✓
- ESP-Matter project structure
- DevContainer for builds
- Single LED control

### Phase 2: Commissioning
- BLE pairing flow
- Join Thread network
- Test with Apple Home / Google Home

### Phase 3: LED Strip
- WS2812B strip support (N LEDs)
- Color gradients
- Brightness control per LED

### Phase 4: Effects
- Rainbow, fade, etc.
- Scenes cluster for presets
- Identify blink pattern

### Phase 5: Production
- ESP32-H2 support
- Power optimization
- Certification prep

## Historical Reference

Rust implementation (rs-matter + Embassy) archived in `rust-experimental` branch.
See [research-rust-matter-thread.md](research-rust-matter-thread.md) for original research.
