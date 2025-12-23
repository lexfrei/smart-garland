# Architecture Decisions

## Overview

Smart garland with Thread/Matter support for ESP32-C6/H2.

## Hardware Decisions

| Decision | Choice | Rationale |
|----------|--------|-----------|
| Dev chip | ESP32-C6 | WiFi+Thread, better tooling, more RAM |
| Prod chip | ESP32-H2 | Thread-only, lower power, cheaper |
| LED protocol | WS2812B (MVP) | Single-wire, ubiquitous, cheap |
| LED backend | RMT peripheral | Hardware timing, CPU-free updates |

## Software Stack

| Layer | Technology | Notes |
|-------|------------|-------|
| RTOS | FreeRTOS (ESP-IDF) | Native ESP32 support |
| SDK | ESP-IDF v5.4.1 | Latest stable with Matter |
| Matter | ESP-Matter v1.4+ | Official Espressif implementation |
| Build | Bazel + CMake | Bazel orchestrates, CMake for ESP-IDF |

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
- Requires Thread Border Router (Apple TV, HomePod, Google Nest Hub)
- FTD (Full Thread Device) on C6, MTD (Minimal Thread Device) on H2

## Modular Features (via Kconfig)

All features toggleable at compile time:

- `CONFIG_GARLAND_EFFECTS_ENABLED` — animation engine
- `CONFIG_GARLAND_SEGMENTS_ENABLED` — LED segmentation
- `CONFIG_GARLAND_LED_TYPE` — LED type selection

## Component Architecture

```
┌─────────────────────────────────────────────────┐
│                  Matter Stack                    │
│  (On/Off, Level, Color, Identify, Scenes)       │
└─────────────────────┬───────────────────────────┘
                      │ callbacks
┌─────────────────────▼───────────────────────────┐
│              garland_matter                      │
│  (attribute handlers, endpoint mapping)          │
└─────────────────────┬───────────────────────────┘
                      │
┌─────────────────────▼───────────────────────────┐
│              garland_effects                     │
│  (animation engine, frame rendering)             │
└─────────────────────┬───────────────────────────┘
                      │
┌─────────────────────▼───────────────────────────┐
│              garland_segments                    │
│  (LED strip segmentation)                        │
└─────────────────────┬───────────────────────────┘
                      │
┌─────────────────────▼───────────────────────────┐
│              garland_led_driver                  │
│  (HAL: RMT/SPI backends)                         │
└─────────────────────┬───────────────────────────┘
                      │
              ┌───────▼───────┐
              │   Hardware    │
              │  (WS2812B)    │
              └───────────────┘
```

## Implementation Phases

### Phase 1: MVP (Single LED) ✅
- Project skeleton with Bazel + CMake
- Thread sdkconfig for ESP32-C6/H2
- Matter stack with Extended Color Light endpoint
- On/Off, Level, Color Control, Identify clusters
- Single WS2812B LED via RMT

### Phase 2: LED Strip Support
- `components/garland_led_driver/` with RMT backend
- Kconfig for LED count, GPIO, type
- Multi-LED support
- ESP32-H2 testing

### Phase 3: Animation Engine
- `components/garland_effects/` — frame-based renderer
- Effects: rainbow, fade, wave, breathing, sparkle
- Effect selection via Scenes cluster

### Phase 4: Segmentation
- `components/garland_segments/`
- Multiple Matter endpoints (one per segment)
- Per-segment color/brightness/effect

### Phase 5: Multiple LED Types
- SPI backend for APA102/SK9822
- RGBW support for SK6812

## Known Issues

| Issue | Mitigation |
|-------|------------|
| RMT timing bugs on C6 | Use led_strip v3.0.2+ |
| H2 memory constraints | Use MTD, disable unused features |
| No Matter effects cluster | Map effects to Scenes cluster |
