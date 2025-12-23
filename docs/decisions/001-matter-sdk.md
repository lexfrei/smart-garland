# ADR-001: Matter SDK Selection

**Status:** Accepted
**Date:** 2024-12-23

## Context

Need to choose Matter SDK for ESP32-C6/H2 smart garland with Thread networking.

### Options Considered

#### Option A: ESP-IDF + ESP-Matter (C++)

- Official Espressif implementation
- Production-ready, stable API
- Full Thread support via OpenThread
- Extensive documentation and examples
- Active community and support

#### Option B: rs-matter (Rust)

- Pure Rust, `no_std`, `no_alloc`, async-first
- Memory safety guarantees
- Experimental status, unstable API
- **Critical: Thread transport NOT implemented**
- Would require custom integration with esp-rs/openthread

## Research Findings

### rs-matter Thread Support Investigation

Searched rs-matter repository and documentation:
- Supports: WiFi, Ethernet, BLE for commissioning
- **Does NOT support:** Thread/OpenThread/802.15.4 transport
- Related projects exist ([esp-rs/openthread](https://github.com/esp-rs/openthread), [esp-ieee802154](https://github.com/esp-rs/esp-ieee802154)) but not integrated

### Quote from rs-matter README:

> "A pure-Rust, no_std, no-alloc, async-first implementation of the Matter protocol"

Transport protocols mentioned: Ethernet, WiFi, custom IP network, BLE GATT.
**Thread conspicuously absent.**

## Decision

**Choose ESP-IDF + ESP-Matter (C++)**

## Rationale

1. **Thread is mandatory** — project requires Thread-only networking (no WiFi)
2. **rs-matter lacks Thread** — would require significant custom work
3. **Production readiness** — ESP-Matter is battle-tested
4. **Time to market** — need working prototype quickly

## Consequences

### Positive
- Full Thread support out of the box
- Stable, documented APIs
- Can leverage existing examples
- Community support available

### Negative
- C++ instead of Rust (less memory safety)
- Larger binary size compared to potential Rust solution
- Vendor lock-in to Espressif toolchain

## Future Considerations

Revisit rs-matter when Thread transport is implemented. Monitor:
- https://github.com/project-chip/rs-matter
- https://github.com/esp-rs/openthread
