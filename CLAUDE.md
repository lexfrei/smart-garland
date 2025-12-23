# Smart Garland - Project Instructions

## Documentation Requirements

- **`./docs/` is the single source of truth** for all architectural decisions, research results, and technical specifications
- All research findings MUST be documented in `./docs/` before implementation
- Decisions and their rationale go to docs, not just code comments
- **Auto-document research**: When gathering new technical information (web searches, API exploration, technology comparisons), write findings to `./docs/` immediately without asking for confirmation

## Project Structure

- `docs/` — documentation and decisions
- `firmware/` — ESP-IDF project (CMake native)
- `tools/` — Bazel rules for ESP-IDF

## Build System

- **Bazel** orchestrates builds
- **CMake** (ESP-IDF native) compiles firmware
- Run `bazel build //firmware:all` to build

## Hardware

- **Development**: ESP32-C6 (Thread only, WiFi disabled)
- **Production**: ESP32-H2 (Thread only)
- **LED**: WS2812B (MVP: single onboard LED)

## Protocol Stack

- Thread (802.15.4) for networking
- Matter for smart home integration
- Device Type: Extended Color Light (0x010D)
