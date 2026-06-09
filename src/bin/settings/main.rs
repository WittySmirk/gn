#![allow(clippy::cast_precision_loss)]

#[path = "../../settings.rs"]
mod settings;

use std::path::PathBuf;

use macroquad::prelude::*;
use settings::Settings;

const BG: Color = Color::new(0.15, 0.15, 0.15, 0.7);
const WIN_BG: Color = Color::new(0.12, 0.12, 0.12, 0.9);
const BTN_BG: Color = Color::new(0.25, 0.25, 0.25, 1.0);
const BTN_HOVER: Color = Color::new(0.35, 0.35, 0.35, 1.0);
const SAVE_BG: Color = Color::new(0.2, 0.5, 0.2, 1.0);
const SAVE_HOVER: Color = Color::new(0.3, 0.6, 0.3, 1.0);
const CANCEL_BG: Color = Color::new(0.4, 0.2, 0.2, 1.0);
const CANCEL_HOVER: Color = Color::new(0.5, 0.3, 0.3, 1.0);
const INPUT_BG: Color = Color::new(0.18, 0.18, 0.18, 1.0);
const LABEL_C: Color = Color::new(0.8, 0.8, 0.8, 1.0);

const WIN_W: f32 = 600.0;
const WIN_H: f32 = 480.0;
const IX: f32 = 20.0;
const IW: f32 = WIN_W - IX * 2.0;
const LW: f32 = 140.0;
const CX: f32 = IX + LW + 8.0;
const CW: f32 = IW - LW - 8.0;
const RH: f32 = 36.0;
const RS: f32 = RH + 6.0;
const RT: f32 = 45.0;
const BW: f32 = 65.0;
const DBW: f32 = 90.0;
const DOPT_H: f32 = 28.0;
const ACW: f32 = 90.0;
const ACH: f32 = 32.0;
const FS: u16 = 16;
const FSH: u16 = 24;

const FPS_OPTS: &[&str] = &["30", "60", "120", "144"];
const GPU_OPTS: &[&str] = &["NVIDIA", "AMD", "Other"];
const DISP_OPTS: &[&str] = &["Auto", "0", "1", "2", "3"];

fn ry(i: usize) -> f32 {
    RT + i as f32 * RS
}

fn over(x: f32, y: f32, w: f32, h: f32, mx: f32, my: f32) -> bool {
    mx >= x && mx <= x + w && my >= y && my <= y + h
}

fn trunc(s: &str, mw: f32, font: Option<&Font>, sz: u16) -> String {
    if measure_text(s, font, sz, 1.0).width <= mw {
        return s.to_string();
    }
    let mut r = s.to_string();
    while !r.is_empty() && measure_text(&format!("{r}\u{2026}"), font, sz, 1.0).width > mw {
        r.pop();
    }
    format!("{r}\u{2026}")
}

fn ld(path_arg: &Option<String>) -> (Settings, Option<PathBuf>) {
    if let Some(p) = path_arg {
        let pb = PathBuf::from(p);
        let c = std::fs::read_to_string(&pb).ok();
        let s = c.and_then(|x| toml::from_str(&x).ok()).unwrap_or_default();
        (s, Some(pb))
    } else {
        let (s, _) = Settings::load();
        (s, None)
    }
}

fn sv(s: &Settings, path: &Option<PathBuf>) {
    let r = if let Some(p) = path {
        if let Some(parent) = p.parent() {
            std::fs::create_dir_all(parent).ok();
        }
        toml::to_string_pretty(s)
            .map_err(|e| std::io::Error::other(e.to_string()))
            .and_then(|c| std::fs::write(p, c))
    } else {
        s.save()
    };
    if let Err(e) = r {
        log::error!("Failed to save settings: {e}");
    }
}

fn draw_row_bg(y: f32) {
    draw_rectangle(0.0, y, WIN_W, RH, Color::new(0.14, 0.14, 0.14, 0.5));
}

fn draw_label(font: &Font, text: &str, y: f32) {
    draw_text_ex(
        text,
        IX,
        y + RH / 2.0 + FS as f32 / 2.0 - 1.0,
        TextParams { font: Some(font), font_size: FS, color: LABEL_C, ..Default::default() },
    );
}

fn draw_dropdown_btn(font: &Font, x: f32, y: f32, w: f32, text: &str, open: bool, hov: bool) {
    let c = if hov { BTN_HOVER } else { BTN_BG };
    draw_rounded_rect(x, y, w, RH, 4.0, c);
    draw_text_ex(
        text,
        x + 8.0,
        y + RH / 2.0 + FS as f32 / 2.0 - 1.0,
        TextParams { font: Some(font), font_size: FS, color: WHITE, ..Default::default() },
    );
    let arrow = if open { "\u{25B2}" } else { "\u{25BC}" };
    let apos = x + w - 18.0;
    draw_text_ex(
        arrow,
        apos,
        y + RH / 2.0 + 6.0,
        TextParams { font: Some(font), font_size: 10, color: WHITE, ..Default::default() },
    );
}

fn draw_dropdown_options(
    font: &Font, x: f32, y: f32, w: f32,
    options: &[&str], selected: usize, mx: f32, my: f32,
) {
    for (i, opt) in options.iter().enumerate() {
        let oy = y + i as f32 * DOPT_H;
        let is_hov = over(x, oy, w, DOPT_H, mx, my);
        let bg = if is_hov { BTN_HOVER } else { INPUT_BG };
        draw_rectangle(x, oy, w, DOPT_H, bg);
        if i == selected {
            draw_rectangle(x, oy, 3.0, DOPT_H, WHITE);
        }
        draw_text_ex(
            opt,
            x + 8.0,
            oy + DOPT_H / 2.0 + FS as f32 / 2.0 - 1.0,
            TextParams { font: Some(font), font_size: FS, color: WHITE, ..Default::default() },
        );
    }
}

fn draw_browse_btn(font: &Font, x: f32, y: f32, hov: bool) {
    let c = if hov { BTN_HOVER } else { BTN_BG };
    draw_rounded_rect(x, y, BW, RH, 4.0, c);
    draw_text_ex(
        "Browse",
        x + BW / 2.0 - measure_text("Browse", Some(font), FS, 1.0).width / 2.0,
        y + RH / 2.0 + FS as f32 / 2.0 - 1.0,
        TextParams { font: Some(font), font_size: FS, color: WHITE, ..Default::default() },
    );
}

fn draw_input(font: &Font, x: f32, y: f32, w: f32, text: &str, focused: bool, time: f64, hov: bool) {
    let c = if focused {
        Color::new(0.22, 0.22, 0.28, 1.0)
    } else if hov {
        Color::new(0.22, 0.22, 0.22, 1.0)
    } else {
        INPUT_BG
    };
    draw_rounded_rect(x, y, w, RH, 4.0, c);
    let display = if text.is_empty() && !focused {
        "Auto".to_string()
    } else {
        trunc(text, w - 8.0, Some(font), FS)
    };
    draw_text_ex(
        &display,
        x + 6.0,
        y + RH / 2.0 + FS as f32 / 2.0 - 1.0,
        TextParams {
            font: Some(font),
            font_size: FS,
            color: if text.is_empty() && !focused {
                LABEL_C
            } else {
                WHITE
            },
            ..Default::default()
        },
    );
    if focused && (time * 2.5).fract() > 0.5 {
        let tw = measure_text(&display, Some(font), FS, 1.0).width;
        draw_line(
            x + 6.0 + tw,
            y + 6.0,
            x + 6.0 + tw,
            y + RH - 6.0,
            1.5,
            WHITE,
        );
    }
}

fn draw_action(font: &Font, x: f32, y: f32, text: &str, hov: bool, bg: Color, hbg: Color) {
    let c = if hov { hbg } else { bg };
    draw_rounded_rect(x, y, ACW, ACH, 4.0, c);
    draw_text_ex(
        text,
        x + ACW / 2.0 - measure_text(text, Some(font), FSH, 1.0).width / 2.0,
        y + ACH / 2.0 + FSH as f32 / 2.0 - 2.0,
        TextParams { font: Some(font), font_size: FSH, color: WHITE, ..Default::default() },
    );
}

fn draw_rounded_rect(x: f32, y: f32, w: f32, h: f32, r: f32, color: Color) {
    draw_circle(x + r, y + r, r, color);
    draw_circle(x + w - r, y + r, r, color);
    draw_circle(x + r, y + h - r, r, color);
    draw_circle(x + w - r, y + h - r, r, color);
    draw_rectangle(x + r, y, w - r * 2.0, h, color);
    draw_rectangle(x, y + r, w, h - r * 2.0, color);
}

fn gpu_idx(s: &Settings) -> usize {
    match (s.nvidia, s.amd) {
        (true, false) => 0,
        (false, true) => 1,
        _ => 2,
    }
}

fn apply_gpu(edit: &mut Settings, idx: usize) {
    edit.nvidia = idx == 0;
    edit.amd = idx == 1;
}

fn fps_idx(v: u32) -> usize {
    match v {
        30 => 0,
        60 => 1,
        120 => 2,
        144 => 3,
        _ => 1,
    }
}

fn apply_fps(edit: &mut Settings, idx: usize) {
    edit.fps = match idx {
        0 => 30,
        1 => 60,
        2 => 120,
        3 => 144,
        _ => 60,
    };
}

fn disp_idx(d: &Option<u32>) -> usize {
    match d {
        None => 0,
        Some(0) => 1,
        Some(1) => 2,
        Some(2) => 3,
        Some(3) => 4,
        _ => 0,
    }
}

fn apply_disp(edit: &mut Settings, idx: usize) {
    edit.display_index = match idx {
        0 => None,
        1 => Some(0),
        2 => Some(1),
        3 => Some(2),
        4 => Some(3),
        _ => None,
    };
}

#[macroquad::main("gn-settings")]
async fn main() {
    env_logger::Builder::from_env(env_logger::Env::default().default_filter_or("info")).init();
    let font = load_ttf_font_from_bytes(include_bytes!("../../../resources/FunnelSans.ttf")).unwrap();

    let mut args: Vec<String> = std::env::args().collect();
    args.remove(0);
    let sv_p = args.first().cloned();

    let (mut edit, save_path) = ld(&sv_p);
    let mut audio_text = edit.audio_device.clone().unwrap_or_default();
    let mut text_focused = false;
    let mut open_dropdown: Option<usize> = None;

    request_new_screen_size(WIN_W, WIN_H);

    // Pre-compute selected indices
    let mut sel_fps = fps_idx(edit.fps);
    let mut sel_disp = disp_idx(&edit.display_index);
    let mut sel_gpu = gpu_idx(&edit);

    loop {
        let _dt = get_frame_time();
        let (mx, my) = mouse_position();
        let clicked = is_mouse_button_pressed(MouseButton::Left);

        // Audio text input
        if text_focused {
            while let Some(ch) = get_char_pressed() {
                if ch.is_ascii_graphic() || ch == ' ' {
                    audio_text.push(ch);
                }
            }
            if is_key_pressed(KeyCode::Backspace) && !audio_text.is_empty() {
                audio_text.pop();
            }
        }

        // Mouse clicks
        if clicked {
            let mut click_handled = false;

            // If a dropdown is open, check option clicks or dismiss
            if let Some(open_row) = open_dropdown {
                let dd_btn_x = CX + CW - DBW;
                let opts: &[&str] = match open_row {
                    0 => FPS_OPTS,
                    3 => GPU_OPTS,
                    5 => DISP_OPTS,
                    _ => unreachable!(),
                };
                let opt_top = ry(open_row) + RH;
                for i in 0..opts.len() {
                    let oy = opt_top + i as f32 * DOPT_H;
                    if over(dd_btn_x, oy, DBW, DOPT_H, mx, my) {
                        match open_row {
                            0 => { apply_fps(&mut edit, i); sel_fps = i; }
                            3 => { apply_gpu(&mut edit, i); sel_gpu = i; }
                            5 => { apply_disp(&mut edit, i); sel_disp = i; }
                            _ => {}
                        }
                        break;
                    }
                }
                open_dropdown = None;
                click_handled = true;
            }

            if !click_handled {
                // Row 0: FPS dropdown
                let y0 = ry(0);
                let fps_dx = CX + CW - DBW;
                if over(fps_dx, y0, DBW, RH, mx, my) {
                    open_dropdown = Some(0);
                }

                // Row 1: Output folder
                let y1 = ry(1);
                let b1x = CX + CW - BW;
                if over(b1x, y1, BW, RH, mx, my) {
                    if let Some(f) = rfd::FileDialog::new().pick_folder() {
                        edit.output_folder = f;
                    }
                }

                // Row 2: Clips folder
                let y2 = ry(2);
                let b2x = CX + CW - BW;
                if over(b2x, y2, BW, RH, mx, my) {
                    if let Some(f) = rfd::FileDialog::new().pick_folder() {
                        edit.clips_folder = f;
                    }
                }

                // Row 3: GPU dropdown
                let y3 = ry(3);
                let gpu_dx = CX + CW - DBW;
                if over(gpu_dx, y3, DBW, RH, mx, my) {
                    open_dropdown = Some(3);
                }

                // Row 4: Audio text
                let y4 = ry(4);
                if over(CX, y4, CW, RH, mx, my) {
                    text_focused = true;
                }

                // Row 5: Display dropdown
                let y5 = ry(5);
                let disp_dx = CX + CW - DBW;
                if over(disp_dx, y5, DBW, RH, mx, my) {
                    open_dropdown = Some(5);
                }

                // Save / Cancel
                let aby = WIN_H - 20.0 - ACH;
                let ctr = WIN_W / 2.0;
                if over(ctr - ACW - 6.0, aby, ACW, ACH, mx, my) {
                    edit.audio_device = if audio_text.is_empty() { None } else { Some(audio_text.clone()) };
                    sv(&edit, &save_path);
                    std::process::exit(0);
                }
                if over(ctr + 6.0, aby, ACW, ACH, mx, my) {
                    std::process::exit(0);
                }
            }
        }

        // Keyboard
        if is_key_pressed(KeyCode::Escape) {
            if text_focused {
                text_focused = false;
            } else if open_dropdown.is_some() {
                open_dropdown = None;
            } else {
                std::process::exit(0);
            }
        }
        if is_key_pressed(KeyCode::Enter) || is_key_pressed(KeyCode::Tab) {
            text_focused = false;
        }

        // ---- Draw ----
        clear_background(WIN_BG);

        draw_text_ex(
            "Settings",
            IX,
            38.0,
            TextParams { font: Some(&font), font_size: FSH, color: WHITE, ..Default::default() },
        );
        draw_line(IX, 46.0, WIN_W - IX, 46.0, 1.0, BG);

        // Row 0: FPS
        let y0 = ry(0);
        draw_row_bg(y0);
        draw_label(&font, "FPS", y0);
        let fps_dx = CX + CW - DBW;
        let fps_open = open_dropdown == Some(0);
        let fps_hov = over(fps_dx, y0, DBW, RH, mx, my);
        draw_dropdown_btn(&font, fps_dx, y0, DBW, FPS_OPTS[sel_fps], fps_open, fps_hov);

        // Row 1: Output Folder
        let y1 = ry(1);
        draw_row_bg(y1);
        draw_label(&font, "Output Folder", y1);
        let of_text = edit.output_folder.to_string_lossy();
        let of_trunc = trunc(&of_text, CW - BW - 8.0, Some(&font), FS);
        draw_text_ex(
            &of_trunc,
            CX + 4.0,
            y1 + RH / 2.0 + FS as f32 / 2.0 - 1.0,
            TextParams { font: Some(&font), font_size: FS, color: WHITE, ..Default::default() },
        );
        let b1x = CX + CW - BW;
        let b1hov = over(b1x, y1, BW, RH, mx, my);
        draw_browse_btn(&font, b1x, y1, b1hov);

        // Row 2: Clips Folder
        let y2 = ry(2);
        draw_row_bg(y2);
        draw_label(&font, "Clips Folder", y2);
        let cf_text = edit.clips_folder.to_string_lossy();
        let cf_trunc = trunc(&cf_text, CW - BW - 8.0, Some(&font), FS);
        draw_text_ex(
            &cf_trunc,
            CX + 4.0,
            y2 + RH / 2.0 + FS as f32 / 2.0 - 1.0,
            TextParams { font: Some(&font), font_size: FS, color: WHITE, ..Default::default() },
        );
        let b2x = CX + CW - BW;
        let b2hov = over(b2x, y2, BW, RH, mx, my);
        draw_browse_btn(&font, b2x, y2, b2hov);

        // Row 3: GPU
        let y3 = ry(3);
        draw_row_bg(y3);
        draw_label(&font, "GPU", y3);
        let gpu_dx = CX + CW - DBW;
        let gpu_open = open_dropdown == Some(3);
        let gpu_hov = over(gpu_dx, y3, DBW, RH, mx, my);
        draw_dropdown_btn(&font, gpu_dx, y3, DBW, GPU_OPTS[sel_gpu], gpu_open, gpu_hov);

        // Row 4: Audio Device
        let y4 = ry(4);
        draw_row_bg(y4);
        draw_label(&font, "Audio Device", y4);
        let af_hov = over(CX, y4, CW, RH, mx, my);
        draw_input(&font, CX, y4, CW, &audio_text, text_focused, get_time(), af_hov);

        // Row 5: Display Index
        let y5 = ry(5);
        draw_row_bg(y5);
        draw_label(&font, "Display Index", y5);
        let disp_dx = CX + CW - DBW;
        let disp_open = open_dropdown == Some(5);
        let disp_hov = over(disp_dx, y5, DBW, RH, mx, my);
        draw_dropdown_btn(&font, disp_dx, y5, DBW, DISP_OPTS[sel_disp], disp_open, disp_hov);

        // Dropdown options overlay (drawn on top of everything)
        if let Some(open_row) = open_dropdown {
            let (dd_btn_x, dd_opts): (f32, &[&str]) = match open_row {
                0 => (CX + CW - DBW, FPS_OPTS),
                3 => (CX + CW - DBW, GPU_OPTS),
                5 => (CX + CW - DBW, DISP_OPTS),
                _ => unreachable!(),
            };
            let opt_top = ry(open_row) + RH;
            let opt_sel = match open_row {
                0 => sel_fps,
                3 => sel_gpu,
                5 => sel_disp,
                _ => 0,
            };
            draw_dropdown_options(&font, dd_btn_x, opt_top, DBW, dd_opts, opt_sel, mx, my);
        }

        // Save / Cancel
        let aby = WIN_H - 20.0 - ACH;
        let ctr = WIN_W / 2.0;
        let s_hov = over(ctr - ACW - 6.0, aby, ACW, ACH, mx, my);
        let c_hov = over(ctr + 6.0, aby, ACW, ACH, mx, my);
        draw_action(&font, ctr - ACW - 6.0, aby, "Save", s_hov, SAVE_BG, SAVE_HOVER);
        draw_action(&font, ctr + 6.0, aby, "Cancel", c_hov, CANCEL_BG, CANCEL_HOVER);

        next_frame().await;
    }
}
