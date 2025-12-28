# Architecture Decision Records

Index of all architectural decisions for the Smart Garland project.

## Active Decisions

| ADR | Title | Status | Date |
|-----|-------|--------|------|
| [003](003-esp-matter-migration.md) | ESP-Matter Migration | Accepted | 2024-12-28 |
| [002](002-thread-config.md) | Thread Configuration | Accepted | 2024-12-23 |

## Superseded Decisions

| ADR | Title | Status | Superseded By |
|-----|-------|--------|---------------|
| [001](001-matter-sdk.md) | Matter SDK Selection | Superseded | ADR-003 |

## Decision Summary

- **ADR-003**: Use ESP-Matter SDK (C++) for production stability
- **ADR-002**: Thread-only networking, MTD mode for both C6 and H2
- ~~ADR-001~~: ~~rs-matter (Rust)~~ — superseded due to radio layer bugs

## Status Legend

- **Proposed** — under discussion
- **Accepted** — decision made, implementing
- **Superseded** — replaced by newer ADR
- **Deprecated** — no longer relevant
