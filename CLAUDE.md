# Smart Garland - Project Instructions

## Documentation Requirements

- **`./docs/` is the single source of truth** for all architectural decisions, research results, and technical specifications
- All research findings MUST be documented in `./docs/` before implementation
- Decisions and their rationale go to docs, not just code comments
- **Auto-document research**: When gathering new technical information (web searches, API exploration, technology comparisons), write findings to `./docs/` immediately without asking for confirmation

## Decision Records (ADRs)

- **Index**: Maintain `docs/decisions/README.md` with list of all ADRs
- **New decisions**: Create new ADR file (e.g., `003-new-topic.md`)
- **Changed decisions**: Mark old ADR as `Status: Superseded by ADR-XXX`, create new ADR with updated decision
- **Never delete**: Old ADRs stay for history, just mark as outdated/superseded
- **Format**: `docs/decisions/NNN-short-name.md`

## Project Structure

- `docs/` — documentation and decisions (ADRs, research)
- `firmware/` — Rust Embassy project

## Language & Build

- **Language**: Rust (no_std, async)
- **Runtime**: Embassy
- **Build**: `cargo build --release`
- **Flash**: `espflash flash --monitor`
- **Target**: `riscv32imac-unknown-none-elf` (ESP32-C6/H2)

## Key Dependencies

- `embassy-executor` — async runtime
- `esp-hal` — hardware abstraction
- `esp-openthread` — Thread networking (MTD)
- `rs-matter` — Matter protocol

## Hardware

- **Development**: ESP32-C6 (Thread only, WiFi disabled)
- **Production**: ESP32-H2 (Thread only)
- **LED**: WS2812B (MVP: single onboard LED, GPIO8)
- **Port**: `/dev/cu.usbmodem101` (native USB)

## Protocol Stack

- Thread (802.15.4) for networking — MTD mode
- Matter for smart home integration
- Device Type: Extended Color Light (0x010D)

## The Glue

This project implements **custom Thread transport for rs-matter** — connecting:
- rs-matter (Matter protocol)
- esp-openthread (Thread networking)

This is the missing piece in the Rust ecosystem for Matter-over-Thread.
