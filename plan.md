# GN Background Daemon — Rust Rewrite Plan

## Goal
Rewrite the Windows-only C++ background daemon as a cross-platform Rust application supporting macOS, Windows, and Linux.

## Responsibilities
- System tray icon with menu (Open Editor, Settings, Start/Stop Recording, Quit)
- Global hotkeys (Shift+F3/F4 on Windows/Linux, Cmd+Shift+F3/F4 on macOS)
- Launch child processes for editor/settings GUIs
- Manage recording state (start/stop via FFmpeg subprocess)

## Architecture

```
main()
  ├── Init logging
  ├── Load settings (TOML config file)
  ├── Auto-detect audio device if not configured
  ├── Create App (tray + hotkeys on correct thread per platform)
  └── Run event loop → dispatch actions
```

### Module Structure

```
src/
  main.rs      — Entry point: init logging, detect FFmpeg/audio, wire App
  app.rs       — Daemon: tray icon, hotkeys, event loop, action dispatch
  settings.rs  — Settings load/save via serde + TOML
  recorder.rs  — RecordingManager: build FFmpeg command, spawn/stop subprocess
```

### Dependencies

| Crate | Purpose |
|---|---|
| `tray-icon` | Cross-platform system tray (NSStatusBar / Win32 / GTK) |
| `global-hotkey` | Cross-platform global hotkeys |
| `serde` + `toml` | Settings persistence |
| `dirs` | Platform config/data directories |
| `log` + `env_logger` | Logging (RUST_LOG env var) |
| `objc2` + `objc2-app-kit` + `objc2-foundation` | macOS: NSApplication, NSEvent dispatch |

std-only — no additional crate dependencies beyond those above.

### Threading

- **All platforms**: Everything on main thread
- Recording runs as an FFmpeg subprocess managed by `RecordingManager`
- No dedicated Rust recording thread needed — FFmpeg handles its own I/O

### Recording (FFmpeg Subprocess)

`RecordingManager` spawns `ffmpeg` as a child process with platform-specific arguments:

| Platform | Capture | Audio source | Video encoder |
|---|---|---|---|
| **macOS** | `avfoundation` (display index 1) | BlackHole virtual device | `h264_videotoolbox -b:v 5M` |
| **Windows** | `ddagrab=0` (DXGI) | Stereo Mix / dshow | `h264_nvenc` / `h264_amf` / fallback `libx264` |
| **Linux** | `x11grab` | PulseAudio monitor source | `libx264 -preset ultrafast -crf 23` |

Common output: `-c:a aac -b:a 192k -movflags +faststart`

Stop by writing `q\n` to FFmpeg stdin — it flushes the MP4 trailer and exits cleanly.

Output file: `{output_folder}/gn_{YYYY-MM-DD}_{HH-MM-SS}.mp4`

### Actions

| Trigger | Action |
|---|---|
| Tray: Start/Stop Recording | `RecordingManager::start()` / `::stop()` |
| Tray: Open Editor | Spawn editor process (placeholder) |
| Tray: Settings | Spawn settings process (placeholder) |
| Tray: Quit | `std::process::exit(0)` |
| Hotkey: Toggle Recording | Same as tray action |
| Hotkey: Open Editor | Same as tray action |

### Audio Auto-Detection

On first run (when `audio_device` is None in settings):

1. **macOS:** Run `ffmpeg -f avfoundation -list_devices true -i ""`, parse stderr for `BlackHole`, extract its device index → save to settings
2. **Windows:** Run `ffmpeg -list_devices true -f dshow -i dummy`, parse for `Stereo Mix` → save device name
3. **Linux:** Run `pactl list short sources`, filter for `.monitor` → save source name
4. **Not found:** Log setup instructions

### Settings

- TOML file at platform config dir:
  - macOS: `~/Library/Application Support/gn/settings.toml`
  - Linux: `~/.config/gn/settings.toml`
  - Windows: `%APPDATA%/gn/settings.toml`
- Fields: `fps`, `output_folder`, `clips_folder`, `audio_device`, `display_index`, `nvidia`, `amd`
- First run detected by missing config file
