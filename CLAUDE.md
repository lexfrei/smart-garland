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
├── docs/                    # Documentation and ADRs
├── main/                    # Application source code
│   ├── app_main.cpp        # Entry point, Matter endpoint
│   ├── app_driver.cpp/h    # LED driver abstraction
│   ├── CMakeLists.txt      # Component build config
│   └── idf_component.yml   # ESP Component Registry deps
├── scripts/dev.sh          # Container helper (compose workflow)
├── Makefile                # Main build interface
├── Containerfile           # Build environment image
├── compose.yaml            # Container orchestration (dev)
├── CMakeLists.txt          # Project root CMake
├── sdkconfig.defaults      # Default ESP-IDF config
└── partitions.csv          # Flash partition table
```

## Language & Build

- **Language**: C++ (ESP-IDF framework)
- **SDK**: ESP-Matter (official Espressif Matter SDK)
- **Build environment**: Containerized (Podman + ESP-Matter image)
- **Target chips**: ESP32-C6 (dev), ESP32-H2 (production)

### Build Commands

```bash
# Build firmware (runs in container)
make

# Build for specific target
make TARGET=esp32c6   # default
make TARGET=esp32h2   # production

# Flash and monitor (runs on host)
make flash-monitor

# Flash with specific port
make PORT=/dev/cu.usbmodem2101 flash

# Interactive shell in container
make shell

# ESP-IDF menuconfig
make menuconfig

# Clean
make clean
make fullclean   # includes managed_components
```

### How It Works

1. `make` builds the container image from `Containerfile` (based on `espressif/esp-matter`)
2. Runs `idf.py build` inside the container with workspace mounted
3. Output: `build/smart-garland.bin`
4. `make flash` uses `espflash` on the host (USB not available in container on macOS)

### Container Image Protection

The image is labeled with `keep=true` to survive cleanup. Safe prune command:

```bash
podman system prune --filter "label!=keep"
```

### Alternative: Compose Workflow

For interactive development with persistent container:

```bash
./scripts/dev.sh up        # Start container
./scripts/dev.sh build     # Build
./scripts/dev.sh shell     # Interactive shell
./scripts/dev.sh down      # Stop container
```

## Hardware

- **Development**: ESP32-C6-WROOM-1 (Thread + WiFi + BLE)
- **Production**: ESP32-H2 (Thread + BLE, lower power)
- **LED**: WS2812B via RMT
  - MVP: GPIO8 (onboard RGB LED on C6 devkit)
  - Production: External strip (configurable pin/count)
- **Port**: `/dev/cu.usbmodem*` (native USB, auto-detected)

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

## Claude Code Limitations

- **NEVER attempt to read serial monitor output** — espflash monitor requires interactive terminal, cat/stty don't work reliably. Ask user to run monitor manually and paste relevant output.

## Historical Note

Previous Rust implementation (rs-matter + Embassy) is preserved in `rust-experimental` branch.
Migrated to ESP-Matter due to radio layer bugs in Rust OpenThread bindings.
See ADR-003 for details.
