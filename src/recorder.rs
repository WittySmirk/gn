use std::io::Write;
use std::path::{Path, PathBuf};
use std::process::{Child, Command, Stdio};
use std::time::{SystemTime, UNIX_EPOCH};
use std::time::Duration;

use crate::settings::Settings;

pub struct RecordingManager {
    child: Option<Child>,
    output_path: Option<PathBuf>,
}

impl RecordingManager {
    pub fn new() -> Self {
        Self {
            child: None,
            output_path: None,
        }
    }

    pub fn is_recording(&self) -> bool {
        self.child.is_some()
    }

    pub fn start(&mut self, settings: &Settings) -> Result<(), String> {
        if self.is_recording() {
            return Err("Already recording".into());
        }

        let output_path = generate_output_path(&settings.output_folder);

        if let Some(parent) = output_path.parent() {
            std::fs::create_dir_all(parent)
                .map_err(|e| format!("Failed to create output directory: {e}"))?;
        }

        let mut cmd = build_ffmpeg_command(settings, &output_path)?;

        log::info!("Starting recording to {}", output_path.display());

        let child = cmd.spawn().map_err(|e| format!("Failed to spawn ffmpeg: {e}"))?;

        self.child = Some(child);
        self.output_path = Some(output_path);
        Ok(())
    }

    pub fn stop(&mut self) {
        let mut child = match self.child.take() {
            Some(c) => c,
            None => return,
        };

        log::info!("Stopping recording");

        if let Some(stdin) = child.stdin.as_mut() {
            let _ = stdin.write_all(b"q\n");
            let _ = stdin.flush();
        }

        for _ in 0..30 {
            match child.try_wait() {
                Ok(Some(_)) => break,
                Ok(None) => {}
                Err(_) => break,
            }
            std::thread::sleep(Duration::from_millis(100));
        }

        if child.try_wait().ok().flatten().is_none() {
            log::warn!("FFmpeg did not exit gracefully, sending kill");
            let _ = child.kill();
            let _ = child.wait();
        }

        if let Some(path) = &self.output_path {
            log::info!("Recording saved to {}", path.display());
        }
        self.output_path = None;
    }
}

fn generate_output_path(folder: &Path) -> PathBuf {
    let ts = SystemTime::now()
        .duration_since(UNIX_EPOCH)
        .unwrap_or_default()
        .as_secs();
    folder.join(format!("gn_{ts}.mp4"))
}

fn build_ffmpeg_command(settings: &Settings, output: &Path) -> Result<Command, String> {
    let mut cmd = Command::new("ffmpeg");
    cmd.stdin(Stdio::piped())
        .stdout(Stdio::null())
        .stderr(Stdio::inherit());

    #[cfg(target_os = "macos")]
    {
        let video_idx = settings.display_index.unwrap_or(1);
        let audio_idx = settings.audio_device.as_deref().unwrap_or("1");
        cmd.args(["-f", "avfoundation"]);
        cmd.args(["-framerate", &settings.fps.to_string()]);
        cmd.args(["-capture_cursor", "1"]);
        cmd.args(["-i", &format!("{video_idx}:{audio_idx}")]);
        cmd.args(["-c:v", "h264_videotoolbox"]);
        cmd.args(["-b:v", "5M"]);
    }

    #[cfg(target_os = "windows")]
    {
        let display_idx = settings.display_index.unwrap_or(0);
        cmd.args(["-f", "lavfi"]);
        cmd.args(["-i", &format!("ddagrab={display_idx}")]);
        cmd.args(["-framerate", &settings.fps.to_string()]);
        cmd.args(["-draw_mouse", "1"]);
        if let Some(audio) = &settings.audio_device {
            cmd.args(["-f", "dshow"]);
            cmd.args(["-i", &format!("audio={audio}")]);
        }
        let encoder = if settings.nvidia {
            "h264_nvenc"
        } else if settings.amd {
            "h264_amf"
        } else {
            "libx264"
        };
        cmd.args(["-c:v", encoder]);
        match encoder {
            "h264_nvenc" => {
                cmd.args(["-cq", "23"]);
            }
            "h264_amf" => {
                cmd.args(["-qp_i", "23"]);
            }
            _ => {
                cmd.args(["-preset", "ultrafast", "-crf", "23"]);
            }
        }
    }

    #[cfg(target_os = "linux")]
    {
        cmd.args(["-f", "x11grab"]);
        cmd.args(["-framerate", &settings.fps.to_string()]);
        cmd.args(["-draw_mouse", "1"]);
        cmd.args(["-i", ":0.0"]);
        if let Some(audio) = &settings.audio_device {
            cmd.args(["-f", "pulse"]);
            cmd.args(["-i", audio]);
        }
        cmd.args(["-c:v", "libx264"]);
        cmd.args(["-preset", "ultrafast", "-crf", "23"]);
    }

    cmd.args(["-c:a", "aac"]);
    cmd.args(["-b:a", "192k"]);
    cmd.args(["-movflags", "+faststart"]);
    cmd.arg(output);
    Ok(cmd)
}
