use std::sync::atomic::{AtomicBool, Ordering};
use std::sync::Arc;
use std::thread;
use std::time::Duration;

use global_hotkey::hotkey::{Code as HkCode, HotKey, Modifiers as HkMods};
use global_hotkey::{GlobalHotKeyEvent, GlobalHotKeyManager, HotKeyState};
use tray_icon::menu::accelerator::{Accelerator, Code as AccelCode, Modifiers as AccelMods};
use tray_icon::menu::{Menu, MenuEvent, MenuItem, PredefinedMenuItem};
use tray_icon::{Icon, TrayIcon, TrayIconBuilder, TrayIconEvent};

use crate::settings::Settings;

pub struct App {
    _settings: Settings,
    _tray_icon: TrayIcon,
    _hotkey_manager: GlobalHotKeyManager,
    menu_start_stop: MenuItem,
    menu_open_editor: MenuItem,
    menu_settings: MenuItem,
    menu_quit: MenuItem,
    hotkey_record: u32,
    hotkey_editor: u32,
    recording: Arc<AtomicBool>,
}

impl App {
    pub fn new(settings: Settings) -> Self {
        let recording = Arc::new(AtomicBool::new(false));

        let menu = Menu::new();

        let record_accel = Accelerator::new(
            Some(if cfg!(target_os = "macos") { AccelMods::SUPER | AccelMods::SHIFT } else { AccelMods::SHIFT }),
            AccelCode::F4,
        );
        let editor_accel = Accelerator::new(
            Some(if cfg!(target_os = "macos") { AccelMods::SUPER | AccelMods::SHIFT } else { AccelMods::SHIFT }),
            AccelCode::F3,
        );

        let start_stop = MenuItem::new("Start Recording", true, Some(record_accel));
        let open_editor = MenuItem::new("Open Editor", true, Some(editor_accel));
        let settings_item = MenuItem::new("Settings", true, None);
        let separator = PredefinedMenuItem::separator();
        let quit = MenuItem::new("Quit", true, None);

        menu.append(&start_stop).ok();
        menu.append(&open_editor).ok();
        menu.append(&settings_item).ok();
        menu.append(&separator).ok();
        menu.append(&quit).ok();

        let icon = create_tray_icon();

        let tray_icon = TrayIconBuilder::new()
            .with_icon(icon)
            .with_icon_as_template(true)
            .with_menu(Box::new(menu))
            .with_tooltip("gn")
            .build()
            .expect("Failed to create tray icon");

        #[cfg(target_os = "macos")]
        if let Some(_ns_item) = tray_icon.ns_status_item() {
            log::info!("MacOS: got NSStatusItem");
        }

        let hotkey_manager = GlobalHotKeyManager::new().expect("Failed to create hotkey manager");

        let (hk_record, hk_editor) = Self::register_hotkeys(&hotkey_manager);

        Self {
            _settings: settings,
            _tray_icon: tray_icon,
            _hotkey_manager: hotkey_manager,
            menu_start_stop: start_stop,
            menu_open_editor: open_editor,
            menu_settings: settings_item,
            menu_quit: quit,
            hotkey_record: hk_record,
            hotkey_editor: hk_editor,
            recording,
        }
    }

    fn register_hotkeys(manager: &GlobalHotKeyManager) -> (u32, u32) {
        let record_mod = if cfg!(target_os = "macos") {
            HkMods::SUPER | HkMods::SHIFT
        } else {
            HkMods::SHIFT
        };

        let record_hk = HotKey::new(Some(record_mod), HkCode::F4);
        let editor_hk = HotKey::new(Some(record_mod), HkCode::F3);

        manager.register(record_hk).expect("Failed to register recording hotkey");
        manager.register(editor_hk).expect("Failed to register editor hotkey");

        log::info!("Registered hotkeys");

        (record_hk.id, editor_hk.id)
    }

    pub fn run(&self) {
        log::info!("Background daemon started");

        let menu_receiver = MenuEvent::receiver();
        let tray_receiver = TrayIconEvent::receiver();
        let hotkey_receiver = GlobalHotKeyEvent::receiver();

        #[cfg(target_os = "macos")]
        let mut count: u64 = 0;

        loop {
            #[cfg(target_os = "macos")]
            {
                Self::pump_macos_events();
                count += 1;
                if count.is_multiple_of(100) {
                    log::info!("Event loop heartbeat #{}", count);
                }
            }

            while let Ok(event) = menu_receiver.try_recv() {
                let id = event.id();
                log::info!("Menu event: {:?}", id);
                if *id == *self.menu_start_stop.id() {
                    self.toggle_recording();
                } else if *id == *self.menu_open_editor.id() {
                    self.open_editor();
                } else if *id == *self.menu_settings.id() {
                    self.open_settings();
                } else if *id == *self.menu_quit.id() {
                    log::info!("Quit requested");
                    std::process::exit(0);
                }
            }

            #[cfg(target_os = "macos")]
            while let Ok(event) = tray_receiver.try_recv() {
                log::info!("TrayIconEvent received: {:?}", event);
            }

            while let Ok(event) = hotkey_receiver.try_recv() {
                log::info!("Hotkey event: id={} state={:?}", event.id(), event.state());
                if event.state() == HotKeyState::Pressed {
                    if event.id() == self.hotkey_record {
                        log::info!("Hotkey: toggle recording");
                        self.toggle_recording();
                    } else if event.id() == self.hotkey_editor {
                        log::info!("Hotkey: open editor");
                        self.open_editor();
                    }
                }
            }

            std::thread::sleep(Duration::from_millis(50));
        }
    }

    #[cfg(target_os = "macos")]
    fn pump_macos_events() {
        use objc2::MainThreadMarker;
        use objc2_app_kit::{NSEventMask, NSApplication};
        use objc2_foundation::{NSDate, NSDefaultRunLoopMode};

        let mtm = MainThreadMarker::new().unwrap();
        let app = NSApplication::sharedApplication(mtm);
        let distant_past = NSDate::distantPast();

        let mode = unsafe { NSDefaultRunLoopMode };
        while let Some(event) = app.nextEventMatchingMask_untilDate_inMode_dequeue(
            NSEventMask::Any,
            Some(&distant_past),
            mode,
            true,
        ) {
            app.sendEvent(&event);
        }
    }

    fn toggle_recording(&self) {
        let recording = !self.recording.load(Ordering::SeqCst);
        self.recording.store(recording, Ordering::SeqCst);

        if recording {
            log::info!("Starting recording");
            self.menu_start_stop.set_text("Stop Recording");
            self.start_recording_thread();
        } else {
            log::info!("Stopping recording");
            self.menu_start_stop.set_text("Start Recording");
        }
    }

    fn open_editor(&self) {
        log::info!("Opening editor");
    }

    fn open_settings(&self) {
        log::info!("Opening settings");
    }

    fn start_recording_thread(&self) {
        let recording = self.recording.clone();
        thread::spawn(move || {
            while recording.load(Ordering::SeqCst) {
                thread::sleep(Duration::from_millis(100));
            }
            log::info!("Recording thread stopped");
        });
    }
}

fn create_tray_icon() -> Icon {
    let width = 16u32;
    let height = 16u32;
    let mut rgba = Vec::with_capacity((width * height * 4) as usize);

    let cx = width as f32 / 2.0;
    let cy = height as f32 / 2.0;
    // Filled circle — most of the interior
    let inner_r = 5.5;

    for y in 0..height {
        for x in 0..width {
            let dx = x as f32 - cx;
            let dy = y as f32 - cy;
            let dist = (dx * dx + dy * dy).sqrt();

            if dist <= inner_r {
                // Solid monochrome (template image on macOS)
                rgba.extend_from_slice(&[0, 0, 0, 255]);
            } else {
                rgba.extend_from_slice(&[0, 0, 0, 0]);
            }
        }
    }

    Icon::from_rgba(rgba, width, height).expect("Failed to create tray icon")
}
