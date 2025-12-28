# Smart Garland - Project Instructions

## Documentation Requirements

- **`./docs/` is the single source of truth** for all architectural decisions, research results, and technical specifications
- All research findings MUST be documented in `./docs/` before implementation
- Decisions and their rationale go to docs, not just code comments
- **Auto-document research**: When gathering new technical information (web searches, API exploration, technology comparisons), write findings to `./docs/` immediately without asking for confirmation

## Decision Records (ADRs)

- **Index**: Maintain `docs/decisions/README.md` with list of all ADRs
- **New decisions**: Create new ADR file (e.g., `004-new-topic.md`)
- **Changed decisions**: Mark old ADR as `Status: Superseded by ADR-XXX`, create new ADR with updated decision
- **Never delete**: Old ADRs stay for history, just mark as outdated/superseded
- **Format**: `docs/decisions/NNN-short-name.md`

## Project Structure

```
smart-garland/
├── .devcontainer/           # DevContainer for isolated builds
├── docs/                    # Documentation and ADRs
├── main/                    # Application source code
│   ├── app_main.cpp        # Entry point
│   ├── app_driver.cpp/h    # LED driver abstraction
│   ├── CMakeLists.txt      # Component build config
│   └── idf_component.yml   # ESP Component Registry deps
├── CMakeLists.txt          # Project root CMake
├── sdkconfig.defaults      # Default ESP-IDF config
└── partitions.csv          # Flash partition table
```

## Language & Build

- **Language**: C++ (ESP-IDF framework)
- **SDK**: ESP-Matter (official Espressif Matter SDK)
- **Build environment**: DevContainer with `espressif/esp-matter` image
- **Target chips**: ESP32-C6 (dev), ESP32-H2 (production)

### Build Commands (inside DevContainer)

```bash
# Source environment
source $IDF_PATH/export.sh
source $ESP_MATTER_PATH/export.sh

# Set target
idf.py set-target esp32c6

# Build
idf.py build

# Flash (from host, not container on macOS)
espflash flash build/smart-garland.bin --port /dev/cu.usbmodem2101
```

### macOS Note

Docker/Podman on macOS doesn't support USB passthrough. Build in container, flash from host.

## Hardware

- **Development**: ESP32-C6-DevKitC-1 (Thread + WiFi + BLE)
- **Production**: ESP32-H2 (Thread + BLE, lower power)
- **LED**: WS2812B via RMT
  - MVP: GPIO8 (onboard RGB LED on C6 devkit)
  - Production: External strip (configurable pin/count)
- **Port**: `/dev/cu.usbmodem2101` (native USB)

## Modular Architecture

Project is designed for multiple targets and garland types:

**Targets:**
- `esp32c6` — development, has WiFi for debugging
- `esp32h2` — production, Thread-only, lower power

**Garland types** (future):
- Single LED (MVP/testing)
- WS2812B strip (N addressable LEDs)
- SK6812 RGBW strip
- Simple GPIO LEDs

## Protocol Stack

- Matter for smart home integration
- Thread (802.15.4) for networking — MTD mode
- BLE for commissioning
- Device Type: Extended Color Light (0x010D)

## Key Dependencies

- `esp-matter` — Espressif's Matter SDK
- `chip` — Project CHIP (Matter core)
- `led_strip` — ESP Component Registry LED driver
- `openthread` — Thread stack (via esp-matter)

## Historical Note

Previous Rust implementation (rs-matter + Embassy) is preserved in `rust-experimental` branch.
Migrated to ESP-Matter due to radio layer bugs in Rust OpenThread bindings.
See ADR-003 for details.
