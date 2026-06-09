# GN Background Daemon — Rust Rewrite Plan

## Goal
Rewrite the Windows-only C++ background daemon as a cross-platform Rust application supporting macOS, Windows, and Linux.

## Responsibilities
- System tray icon with menu (Open Editor, Settings, Start/Stop Recording, Quit)
- Global hotkeys (Shift+F3/F4 on Windows/Linux, Cmd+F3/F4 on macOS)
- Launch child processes for editor/settings GUIs
- Manage recording state (start/stop via thread)

## Architecture

```
main()
  ├── Init logging
  ├── Load settings (TOML config file)
  ├── Create App (tray + hotkeys on correct thread per platform)
  └── Run event loop → dispatch actions
```

### Module Structure

```
src/
  main.rs      — Entry point, wires everything together
  app.rs       — Daemon: tray icon, hotkeys, action dispatch
  settings.rs  — Settings load/save via serde + TOML
```

### Dependencies

| Crate | Purpose |
|---|---|
| `tray-icon` | Cross-platform system tray (Win32/NSStatusBar/GTK) |
| `global-hotkey` | Cross-platform global hotkeys |
| `serde` + `toml` | Settings persistence |
| `dirs` | Platform-appropriate config/data directories |
| `log` + `env_logger` | Logging |
| `core-foundation` (macOS only) | Pump CFRunLoop on macOS |

### Threading

- **macOS**: Everything on main thread, pump CFRunLoop for event delivery
- **Windows/Linux**: Simple polling loop on main thread

### Actions

| Trigger | Action |
|---|---|
| Tray: Start/Stop Recording | Toggle `AtomicBool`, spawn/kill recording thread |
| Tray: Open Editor | Spawn editor process (placeholder for now) |
| Tray: Settings | Spawn settings process (placeholder for now) |
| Tray: Quit | `std::process::exit(0)` |
| Hotkey: Toggle Recording | Same as tray action |
| Hotkey: Open Editor | Same as tray action |

### Recording

- Runs in a `std::thread` controlled by `Arc<AtomicBool>`
- Stub for now — actual capture logic will be implemented later

### Settings

- TOML file at platform config dir (`~/.config/gn/settings.toml` on Linux, `~/Library/Application Support/gn/settings.toml` on macOS, `%APPDATA%/gn/settings.toml` on Windows)
- Fields: `fps`, `output_folder`, `clips_folder`, `nvidia`, `amd`
- First run detected by missing config file
