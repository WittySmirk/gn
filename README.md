# gn

Minimalist screen clipping daemon — cross-platform Rust rewrite.

- System tray icon with menu (Start/Stop Recording, Open Editor, Settings, Quit)
- Global hotkeys (`Shift+F3`/`F4`, `Cmd+Shift+F3`/`F4` on macOS)
- Recording thread managed by the daemon process (capture logic TBD)

## Build

```bash
cargo build
cargo run
```

Logging controlled by `RUST_LOG` env var (default: `info`):

```bash
RUST_LOG=debug cargo run
```

## Dependencies

See `Cargo.toml` for full list. Platform-specific macOS deps (`objc2-*`) are gated by `cfg(target_os = "macos")`.
