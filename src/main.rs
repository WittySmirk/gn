mod app;
mod recorder;
mod settings;

use std::process::{Command, Stdio};

use app::App;
use settings::Settings;

#[cfg(target_os = "macos")]
fn setup_macos_app() {
    use objc2::msg_send;
    use objc2::MainThreadMarker;
    use objc2_app_kit::NSApplication;

    let mtm = MainThreadMarker::new().expect("Must be on main thread");
    let app = NSApplication::sharedApplication(mtm);
    // NSApplicationActivationPolicyAccessory = 1 — no dock icon, no menu bar
    unsafe {
        let _: bool = msg_send![&app, setActivationPolicy: 1isize];
    }
    app.finishLaunching();
}

fn check_ffmpeg() -> bool {
    Command::new("ffmpeg")
        .arg("-version")
        .stdout(Stdio::null())
        .stderr(Stdio::null())
        .status()
        .is_ok()
}

fn detect_system_audio() -> Option<String> {
    #[cfg(target_os = "macos")]
    {
        let output = Command::new("ffmpeg")
            .args(["-f", "avfoundation", "-list_devices", "true", "-i", ""])
            .stderr(Stdio::piped())
            .stdout(Stdio::null())
            .output()
            .ok()?;
        let stderr = String::from_utf8_lossy(&output.stderr);
        let section = stderr.split("AVFoundation audio devices:").nth(1)?;
        for line in section.lines() {
            if line.contains("BlackHole") {
                let idx = line.split('[').nth(1)?.split(']').next()?.parse::<u32>().ok()?;
                return Some(idx.to_string());
            }
        }
        None
    }

    #[cfg(target_os = "windows")]
    {
        let output = Command::new("ffmpeg")
            .args(["-list_devices", "true", "-f", "dshow", "-i", "dummy"])
            .stderr(Stdio::piped())
            .stdout(Stdio::null())
            .output()
            .ok()?;
        let stderr = String::from_utf8_lossy(&output.stderr);
        for line in stderr.lines() {
            if line.contains("Stereo Mix") {
                if let Some(start) = line.find('"') {
                    let rest = &line[start + 1..];
                    if let Some(end) = rest.find('"') {
                        return Some(rest[..end].to_string());
                    }
                }
            }
        }
        None
    }

    #[cfg(target_os = "linux")]
    {
        let output = Command::new("pactl")
            .args(["list", "short", "sources"])
            .output()
            .ok()?;
        let stdout = String::from_utf8_lossy(&output.stdout);
        for line in stdout.lines() {
            let parts: Vec<&str> = line.split_whitespace().collect();
            if parts.len() >= 2 && parts[1].contains(".monitor") {
                return Some(parts[1].to_string());
            }
        }
        None
    }

    #[cfg(not(any(target_os = "macos", target_os = "windows", target_os = "linux")))]
    None
}

fn main() {
    env_logger::Builder::from_env(env_logger::Env::default().default_filter_or("info")).init();

    log::info!("gn background daemon starting");

    #[cfg(target_os = "macos")]
    setup_macos_app();

    if !check_ffmpeg() {
        log::error!("ffmpeg not found in PATH");
        log::error!("  macOS: brew install ffmpeg");
        log::error!("  Windows: winget install ffmpeg");
        log::error!("  Linux: apt install ffmpeg");
        std::process::exit(1);
    }

    let (mut settings, loaded) = Settings::load();
    if !loaded {
        log::info!("First run detected — using default settings");
    }

    if settings.audio_device.is_none() {
        log::info!("No audio device configured — scanning for system audio...");
        if let Some(device) = detect_system_audio() {
            settings.audio_device = Some(device.clone());
            settings.save().ok();
            log::info!("Auto-detected audio device: {device}");
        } else {
            log::warn!("No system audio loopback device found.");
            #[cfg(target_os = "macos")]
            log::warn!("  Install BlackHole: brew install --cask blackhole-2ch");
            #[cfg(target_os = "macos")]
            log::warn!("  Then open Audio MIDI Setup and create a Multi-Output Device");
            #[cfg(target_os = "windows")]
            log::warn!("  Enable Stereo Mix in Sound Control Panel > Show Disabled Devices");
            #[cfg(target_os = "linux")]
            log::warn!("  Ensure PulseAudio is running");
        }
    }

    let app = App::new(settings);
    app.run();
}
