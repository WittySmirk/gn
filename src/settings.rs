use serde::{Deserialize, Serialize};
use std::path::PathBuf;

#[derive(Debug, Clone, Serialize, Deserialize)]
pub struct Settings {
    pub fps: u32,
    pub output_folder: PathBuf,
    pub clips_folder: PathBuf,
    #[serde(default)]
    pub nvidia: bool,
    #[serde(default)]
    pub amd: bool,
    /// Audio device identifier (platform-specific):
    ///   macOS: BlackHole device index (e.g. "1")
    ///   Windows: Stereo Mix device name (e.g. "Stereo Mix (Realtek Audio)")
    ///   Linux: PulseAudio monitor source name
    #[serde(default)]
    pub audio_device: Option<String>,
    /// Display/monitor index to capture (0 = primary)
    #[serde(default)]
    pub display_index: Option<u32>,
}

impl Default for Settings {
    fn default() -> Self {
        let docs = dirs::document_dir().unwrap_or_else(|| PathBuf::from("."));
        Self {
            fps: 60,
            output_folder: docs.join("gn").join("captures"),
            clips_folder: docs.join("gn").join("clips"),
            nvidia: false,
            amd: false,
            audio_device: None,
            display_index: None,
        }
    }
}

#[allow(dead_code)]
impl Settings {
    pub fn config_dir() -> PathBuf {
        dirs::config_dir()
            .unwrap_or_else(|| PathBuf::from("."))
            .join("gn")
    }

    pub fn config_path() -> PathBuf {
        Self::config_dir().join("settings.toml")
    }

    pub fn load() -> (Self, bool) {
        let path = Self::config_path();
        if !path.exists() {
            log::info!("No settings file found at {:?}, using defaults", path);
            return (Self::default(), false);
        }
        match std::fs::read_to_string(&path) {
            Ok(content) => match toml::from_str(&content) {
                Ok(settings) => {
                    log::info!("Settings loaded from {:?}", path);
                    (settings, true)
                }
                Err(e) => {
                    log::warn!("Failed to parse settings: {e}, using defaults");
                    (Self::default(), false)
                }
            },
            Err(e) => {
                log::warn!("Failed to read settings: {e}, using defaults");
                (Self::default(), false)
            }
        }
    }

    pub fn save(&self) -> Result<(), std::io::Error> {
        let path = Self::config_path();
        if let Some(parent) = path.parent() {
            std::fs::create_dir_all(parent)?;
        }
        let content =
            toml::to_string_pretty(self).map_err(|e| std::io::Error::other(e.to_string()))?;
        std::fs::write(&path, content)?;
        log::info!("Settings saved to {:?}", path);
        Ok(())
    }

    pub fn is_first_run() -> bool {
        !Self::config_path().exists()
    }
}
