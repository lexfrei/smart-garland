# ADR-003: Migration to ESP-Matter SDK

**Status:** Accepted
**Date:** 2024-12-28
**Supersedes:** ADR-001

## Context

After implementing the rs-matter (Rust) approach from ADR-001, we encountered blocking issues:

1. **Radio layer bug**: After `rx_when_idle=true`, the radio stops receiving packets
2. **SRP timeout**: Service Registration Protocol fails due to missing RX
3. **Experimental status**: rs-matter is officially "Experimental", not production-ready
4. **Debugging overhead**: Weeks spent on low-level radio issues

The goal is a **working garland**, not a Rust learning project.

## Decision

**Switch from rs-matter (Rust) to ESP-Matter SDK (C++)**

### Rationale

| Factor | rs-matter | ESP-Matter |
|--------|-----------|------------|
| Status | Experimental | Production-ready (v1.4.2) |
| Thread support | Bugs in radio layer | Works out of box |
| Time to MVP | Weeks (debugging) | Hours |
| Documentation | Minimal | Extensive |
| Vendor support | None | Espressif official |

### Trade-offs Accepted

- Larger binary (ESP-IDF + FreeRTOS overhead)
- C++ instead of Rust (less memory safety)
- Vendor lock-in to ESP-IDF

### Modular Architecture

Project supports multiple targets and configurations:

```
smart-garland/
├── main/                    # Application code
│   ├── app_main.cpp        # Entry point
│   ├── app_driver.cpp/h    # LED abstraction
│   └── idf_component.yml   # Dependencies
├── sdkconfig.defaults       # Common config
├── sdkconfig.defaults.esp32c6  # C6-specific (future)
├── sdkconfig.defaults.esp32h2  # H2-specific (future)
└── .devcontainer/           # Isolated build environment
```

**Targets:**
- ESP32-C6 (development, has WiFi+Thread+BLE)
- ESP32-H2 (production, Thread+BLE only, lower power)

**Garland variants** (future):
- Single LED (MVP/testing)
- WS2812B strip (N LEDs)
- SK6812 RGBW strip
- Individual GPIO LEDs

## Consequences

### Positive
- Working device in hours, not weeks
- Production path with certification support
- Official vendor support
- DevContainer for reproducible builds

### Negative
- Larger footprint
- Lost Rust benefits
- More complex C++ codebase

### Migration
- Rust code archived in `rust-experimental` branch
- ADR-001 superseded, kept for history
- New C++ structure in `main/`

## References

- [ESP-Matter SDK](https://github.com/espressif/esp-matter)
- [ESP-Matter Docker](https://hub.docker.com/r/espressif/esp-matter)
- [rust-experimental branch](../../tree/rust-experimental) — preserved Rust work
