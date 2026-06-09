mod app;
mod settings;

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

fn main() {
    env_logger::Builder::from_env(env_logger::Env::default().default_filter_or("info")).init();

    log::info!("gn background daemon starting");

    #[cfg(target_os = "macos")]
    setup_macos_app();

    let (settings, loaded) = Settings::load();
    if !loaded {
        log::info!("First run detected — using default settings");
    }

    let app = App::new(settings);
    app.run();
}
