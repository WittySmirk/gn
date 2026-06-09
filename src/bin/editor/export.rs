use std::path::{Path, PathBuf};
use std::process::Command;
use std::time::{SystemTime, UNIX_EPOCH};

pub fn export_clip(input: &Path, output_dir: &Path, in_pts: f64, out_pts: f64) {
    let ts = SystemTime::now()
        .duration_since(UNIX_EPOCH)
        .unwrap_or_default()
        .as_secs();

    std::fs::create_dir_all(output_dir).ok();

    let output = output_dir.join(format!("gn_export_{ts}.mp4"));

    log::info!("Exporting {} -> {}", input.display(), output.display());

    let input_path = input.to_path_buf();
    let output_path = output;

    std::thread::spawn(move || {
        let result = Command::new("ffmpeg")
            .arg("-i")
            .arg(&input_path)
            .arg("-ss")
            .arg(format!("{:.6}", in_pts))
            .arg("-to")
            .arg(format!("{:.6}", out_pts))
            .arg("-c")
            .arg("copy")
            .arg("-y")
            .arg(&output_path)
            .output();

        match result {
            Ok(out) if out.status.success() => {
                log::info!("Export complete: {}", output_path.display());
            }
            Ok(out) => {
                let stderr = String::from_utf8_lossy(&out.stderr);
                log::error!("Export failed:\n{stderr}");
            }
            Err(e) => {
                log::error!("Failed to run ffmpeg for export: {e}");
            }
        }
    });
}

pub fn default_clips_folder() -> PathBuf {
    let docs = dirs::document_dir().unwrap_or_else(|| PathBuf::from("."));
    docs.join("gn").join("clips")
}
