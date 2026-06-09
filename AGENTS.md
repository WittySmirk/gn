# AGENTS.md — gn Rust Daemon

## Project Structure

```
src/
  main.rs      — Entry point: init logging, load settings, create App, run loop
  app.rs       — Daemon core: tray icon, global hotkeys, event loop, action dispatch
  settings.rs  — Settings load/save via serde + TOML
```

## Key Dependencies

| Crate | Purpose |
|---|---|
| `tray-icon` | Cross-platform system tray (NSStatusBar / Win32 / GTK) |
| `global-hotkey` | Cross-platform global hotkeys |
| `serde` + `toml` | Settings persistence |
| `dirs` | Platform config/data directories |
| `log` + `env_logger` | Logging (RUST_LOG env var) |
| `objc2` + `objc2-app-kit` + `objc2-foundation` | macOS: NSApplication, NSRunLoop, NSEvent dispatch |

## macOS Event Pump

On macOS, AppKit events (mouse clicks, keyboard) are NOT processed by `CFRunLoop`/`NSRunLoop::runMode_beforeDate`. Those only dispatch CFRunLoop sources/timers. The daemon uses `NSApplication::nextEventMatchingMask:untilDate:inMode:dequeue:` + `sendEvent:` to properly drain the Cocoa event queue and dispatch mouse events to the tray icon's `TrayTarget` view.

```rust
// src/app.rs — pump_macos_events()
let app = NSApplication::sharedApplication(mtm);
let distant_past = NSDate::distantPast();
while let Some(event) = app.nextEventMatchingMask_untilDate_inMode_dequeue(
    NSEventMask::Any, Some(&distant_past), mode, true,
) {
    app.sendEvent(&event);
}
```

## Tray Icon

- Generated programmatically: 16×16 monochrome filled circle in RGBA
- `with_icon_as_template(true)` for macOS template rendering (adapts to light/dark menu bar)
- Menu built with `tray_icon::menu` (wraps `muda`)

## Hotkeys

| Action | macOS | Windows/Linux |
|---|---|---|
| Toggle Recording | `Cmd+Shift+F4` | `Shift+F4` |
| Open Editor | `Cmd+Shift+F3` | `Shift+F3` |

## Recording (Not Yet Implemented)

`start_recording_thread()` is a stub that loops on `Arc<AtomicBool>` and sleeps. Actual screen capture logic will go here.

Settings define output paths:
- Captures: `~/Documents/gn/captures` (`output_folder`)
- Clips: `~/Documents/gn/clips` (`clips_folder`)

## Settings

TOML file at platform config dir:
- macOS: `~/Library/Application Support/gn/settings.toml`
- Linux: `~/.config/gn/settings.toml`
- Windows: `%APPDATA%/gn/settings.toml`

Fields: `fps`, `output_folder`, `clips_folder`, `nvidia`, `amd`.
