# ADR-001: Matter SDK Selection

**Status:** Accepted (Updated)
**Date:** 2024-12-23
**Updated:** 2024-12-23

## Context

Need to choose Matter SDK for ESP32-C6/H2 smart garland with Thread networking.

## Decision History

### Initial Decision: ESP-Matter (C++)

Initially chose ESP-IDF + ESP-Matter because rs-matter lacked Thread transport.

### Updated Decision: rs-matter (Rust) + Custom Glue

After deeper research (see `../research-rust-matter-thread.md`), discovered that:

1. **esp-openthread** provides working Thread MTD support
2. **rs-matter** has modular transport layer
3. **Missing piece** is just glue code (~100-500 lines)

## Final Decision

**Use rs-matter (Rust) with custom Thread transport glue**

### Stack

```
rs-matter (Matter protocol)
        ↓
thread-transport (custom glue)
        ↓
esp-openthread (Thread MTD)
        ↓
esp-ieee802154 (802.15.4 radio)
        ↓
Embassy (async runtime)
```

## Rationale

1. **MTD is sufficient** — garland is end device, doesn't need routing
2. **Glue is tractable** — both sides ready, just need UDP adapter
3. **Pure Rust benefits** — memory safety, modern async, no C++ footguns
4. **Learning opportunity** — contribute to Rust embedded ecosystem

## Risks

| Risk | Mitigation |
|------|------------|
| Uncharted territory | Start with working esp-openthread examples |
| rs-matter API changes | Pin to specific git commit |
| Debugging complexity | Use defmt + probe-rs |

## Consequences

### Positive
- Pure Rust, memory safe
- Modern async code with Embassy
- Could contribute glue back to community
- No ESP-IDF/FreeRTOS dependency

### Negative
- More upfront work (write glue code)
- Less documentation
- No vendor support

## References

- [Research document](../research-rust-matter-thread.md)
- [rs-matter](https://github.com/project-chip/rs-matter)
- [esp-openthread](https://github.com/esp-rs/esp-openthread)
