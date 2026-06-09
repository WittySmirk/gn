mod export;
mod player;

use std::path::{Path, PathBuf};

use export::{default_clips_folder, export_clip};
use macroquad::prelude::*;
use player::VideoPlayer;

#[macroquad::main("gn-editor")]
async fn main() {
    ffmpeg_next::init().unwrap();
    env_logger::Builder::from_env(env_logger::Env::default().default_filter_or("info")).init();

    let font = load_ttf_font_from_bytes(include_bytes!("../../../resources/FunnelSans.ttf")).unwrap();

    let mut args: Vec<String> = std::env::args().collect();
    args.remove(0);

    let file_path = if !args.is_empty() {
        Some(PathBuf::from(&args[0]))
    } else {
        rfd::FileDialog::new()
            .add_filter("Video", &["mp4", "mov", "mkv", "avi"])
            .pick_file()
    };

    let clips_folder = if args.len() > 1 {
        PathBuf::from(&args[1])
    } else {
        default_clips_folder()
    };

    let path = match file_path {
        Some(p) => p,
        None => return,
    };

    let mut player = match VideoPlayer::open(&path) {
        Ok(p) => p,
        Err(e) => {
            log::error!("{e}");
            return;
        }
    };

    let mut win_w = player.width() as f32;
    let mut win_h = player.height() as f32;
    while win_w > 1920.0 || win_h > 1080.0 {
        win_w *= 0.5;
        win_h *= 0.5;
    }
    request_new_screen_size(win_w, win_h);

    let mut texture: Option<Texture2D> = None;
    let mut play_start: Option<(std::time::Instant, f64)> = None;
    let mut seek_hold_timer = 0.0;
    let mut seek_repeat_acc = 0.0;

    if let Some(rgba) = player.decode_next() {
        texture = Some(Texture2D::from_rgba8(
            player.width() as u16,
            player.height() as u16,
            &rgba,
        ));
    }

    loop {
        let dt = get_frame_time() as f64;

        player.update_export_timer(dt);

        let seek_result = handle_input(&mut player, &path, &clips_folder, &mut play_start);

        let w = player.width() as u16;
        let h = player.height() as u16;
        if let Some(data) = seek_result {
            texture = Some(Texture2D::from_rgba8(w, h, &data));
        }

        if player.is_playing() {
            let target = match play_start {
                Some((start, base_secs)) => base_secs + start.elapsed().as_secs_f64(),
                None => player.current_time(),
            };
            loop {
                if player.current_time() >= target || player.current_time() >= player.total_duration() {
                    break;
                }
                match player.decode_next() {
                    Some(rgba) => texture = Some(Texture2D::from_rgba8(w, h, &rgba)),
                    None => {
                        player.toggle_play();
                        break;
                    }
                }
            }
        }

        let left_down = is_key_down(KeyCode::Left) || is_key_down(KeyCode::H);
        let right_down = is_key_down(KeyCode::Right) || is_key_down(KeyCode::L);
        let just_pressed = is_key_pressed(KeyCode::Left) || is_key_pressed(KeyCode::H)
            || is_key_pressed(KeyCode::Right) || is_key_pressed(KeyCode::L);
        if just_pressed {
            if player.is_playing() {
                player.toggle_play();
            }
            play_start = None;
            seek_hold_timer = 0.0;
            seek_repeat_acc = 0.0;
            let dir = if left_down { -1 } else { 1 };
            if let Some(data) = player.step_frames(dir) {
                texture = Some(Texture2D::from_rgba8(w, h, &data));
            }
        }
        if (left_down || right_down) && !player.is_playing() && !just_pressed && seek_hold_timer > 0.3 {
            let dir = if left_down { -1 } else { 1 };
            seek_repeat_acc += dt;
            while seek_repeat_acc >= 0.05 {
                seek_repeat_acc -= 0.05;
                if let Some(data) = player.step_frames(dir) {
                    texture = Some(Texture2D::from_rgba8(w, h, &data));
                }
            }
        }
        if left_down || right_down {
            seek_hold_timer += dt;
        } else {
            seek_hold_timer = 0.0;
            seek_repeat_acc = 0.0;
        }

        clear_background(BLACK);

        if let Some(ref tex) = texture {
            draw_video_frame(tex, player.width() as f32, player.height() as f32);
        }

        draw_ui(&player, &font);

        next_frame().await;
    }
}

fn handle_input(
    player: &mut VideoPlayer,
    path: &Path,
    clips_folder: &Path,
    play_start: &mut Option<(std::time::Instant, f64)>,
) -> Option<Vec<u8>> {
    if is_key_pressed(KeyCode::Escape) || is_key_pressed(KeyCode::Q) {
        std::process::exit(0);
    }

    if is_key_pressed(KeyCode::Space) || is_key_pressed(KeyCode::K) {
        player.toggle_play();
        if player.is_playing() {
            *play_start = Some((std::time::Instant::now(), player.current_time()));
        } else {
            *play_start = None;
        }
    }

    if is_key_pressed(KeyCode::J) {
        if player.is_playing() {
            player.toggle_play();
        }
        *play_start = None;
        return player.seek_to_time(-5.0);
    }

    if is_key_pressed(KeyCode::Semicolon) {
        if player.is_playing() {
            player.toggle_play();
        }
        *play_start = None;
        return player.seek_to_time(5.0);
    }

    if is_key_pressed(KeyCode::I) {
        player.set_in_point();
    }

    if is_key_pressed(KeyCode::O) {
        player.set_out_point();
    }

    if is_key_pressed(KeyCode::E) {
        if let (Some(in_pts), Some(out_pts)) = (player.in_point_pts(), player.out_point_pts()) {
            let tb = player.stream_time_base();
            let in_secs = in_pts as f64 * tb.0 as f64 / tb.1 as f64;
            let out_secs = out_pts as f64 * tb.0 as f64 / tb.1 as f64;
            if (out_secs - in_secs).abs() > 0.01 {
                export_clip(path, clips_folder, in_secs, out_secs);
                player.set_export_message("Export started!".to_string());
            } else {
                player.set_export_message("Selection too short".to_string());
            }
        } else {
            player.set_export_message("Set in/out points first".to_string());
        }
    }

    if is_key_pressed(KeyCode::G) {
        if !is_key_down(KeyCode::LeftShift) && !is_key_down(KeyCode::RightShift) {
            if player.is_playing() {
                player.toggle_play();
            }
            *play_start = None;
            if let Some(pts) = player.in_point_pts() {
                return player.seek_to(pts);
            }
        } else {
            if player.is_playing() {
                player.toggle_play();
            }
            *play_start = None;
            if let Some(pts) = player.out_point_pts() {
                return player.seek_to(pts);
            }
        }
    }

    if is_mouse_button_pressed(MouseButton::Left) {
        let (mx, my) = mouse_position();
        let sw = screen_width();
        let sh = screen_height();
        let bar_x = 20.0;
        let bar_y = sh - 100.0;
        let bar_w = sw - 40.0;
        let bar_h = 28.0;

        if my >= bar_y && my <= bar_y + bar_h + 4.0 && mx >= bar_x && mx <= bar_x + bar_w {
            let fraction = ((mx - bar_x) / bar_w) as f64;
            let tb = player.stream_time_base();
            let target_pts =
                (fraction * player.total_duration() / tb.0 as f64 * tb.1 as f64) as i64;
            if player.is_playing() {
                player.toggle_play();
            }
            *play_start = None;
            return player.seek_to(target_pts);
        }
    }

    None
}

fn draw_video_frame(tex: &Texture2D, video_w: f32, video_h: f32) {
    let sw = screen_width();
    let sh = screen_height();

    let scale = (sw / video_w).max(sh / video_h);
    let dw = video_w * scale;
    let dh = video_h * scale;
    let dx = (sw - dw) / 2.0;
    let dy = (sh - dh) / 2.0;

    draw_texture_ex(
        tex,
        dx,
        dy,
        WHITE,
        DrawTextureParams {
            dest_size: Some(Vec2::new(dw, dh)),
            ..Default::default()
        },
    );
}

fn format_time(seconds: f64) -> String {
    let total_ms = (seconds * 1000.0) as i64;
    let ms = total_ms % 1000;
    let total_secs = total_ms / 1000;
    let secs = total_secs % 60;
    let mins = total_secs / 60;
    format!("{:02}:{:02}.{:03}", mins, secs, ms)
}

const BG: Color = Color::new(0.15, 0.15, 0.15, 0.7);

fn draw_ui(player: &VideoPlayer, font: &Font) {
    let sw = screen_width();
    let sh = screen_height();
    let total_dur = player.total_duration().max(0.001);
    let current_t = player.current_time().clamp(0.0, total_dur);
    let progress = (current_t / total_dur) as f32;

    let bar_x = 20.0;
    let bar_w = sw - 40.0;
    let bar_h = 28.0;
    let bar_y = sh - 100.0;

    // Unwatched: semi-transparent dark gray
    draw_rectangle(
        bar_x + bar_w * progress,
        bar_y,
        bar_w * (1.0 - progress),
        bar_h,
        BG,
    );

    // Watched: bright
    draw_rectangle(bar_x, bar_y, bar_w * progress, bar_h, WHITE);

    draw_line(
        bar_x + bar_w * progress,
        bar_y,
        bar_x + bar_w * progress,
        bar_y + bar_h,
        2.0,
        WHITE,
    );

    let current_label = format_time(current_t);
    let total_label = format_time(total_dur);

    let cur_w = measure_text(&current_label, Some(font), 15, 1.0).width;
    let tot_w = measure_text(&total_label, Some(font), 15, 1.0).width;

    draw_rectangle(bar_x - 4.0, bar_y - 24.0, cur_w + 8.0, 20.0, BG);
    draw_rectangle(bar_x + bar_w - tot_w - 4.0, bar_y - 24.0, tot_w + 8.0, 20.0, BG);

    draw_text_ex(
        &current_label,
        bar_x,
        bar_y - 10.0,
        TextParams {
            font: Some(font),
            font_size: 15,
            color: WHITE,
            ..Default::default()
        },
    );
    draw_text_ex(
        &total_label,
        bar_x + bar_w - tot_w,
        bar_y - 10.0,
        TextParams {
            font: Some(font),
            font_size: 15,
            color: WHITE,
            ..Default::default()
        },
    );

    if let (Some(in_secs), Some(out_secs)) = (player.in_point(), player.out_point()) {
        let in_frac = (in_secs / total_dur).clamp(0.0, 1.0) as f32;
        let out_frac = (out_secs / total_dur).clamp(0.0, 1.0) as f32;
        let sel_x = bar_x + bar_w * in_frac;
        let sel_w = bar_w * (out_frac - in_frac);
        if sel_w > 0.0 {
            draw_rectangle(
                sel_x,
                bar_y - 6.0,
                sel_w,
                bar_h + 12.0,
                Color::new(0.3, 0.6, 1.0, 0.5),
            );
            let in_label = format_time(in_secs);
            let out_label = format_time(out_secs);
            let in_w = measure_text(&in_label, Some(font), 11, 1.0).width;
            let out_w = measure_text(&out_label, Some(font), 11, 1.0).width;
            draw_rectangle(sel_x + 1.0, bar_y + bar_h + 2.0 - 11.0, in_w + 4.0, 13.0, BG);
            draw_rectangle(sel_x + sel_w - out_w - 3.0, bar_y + bar_h + 2.0 - 11.0, out_w + 4.0, 13.0, BG);
            draw_text_ex(
                &in_label,
                sel_x + 2.0,
                bar_y + bar_h + 2.0,
                TextParams {
                    font: Some(font),
                    font_size: 11,
                    color: WHITE,
                    ..Default::default()
                },
            );
            draw_text_ex(
                &out_label,
                sel_x + sel_w - out_w - 2.0,
                bar_y + bar_h + 2.0,
                TextParams {
                    font: Some(font),
                    font_size: 11,
                    color: WHITE,
                    ..Default::default()
                },
            );
        }
    }

    let hints = match player.is_playing() {
        true => "Space/K:pause  I:in  O:out  E:xport  H/L:frame  J/;:5s  G:go to  Q:quit",
        false => "Space/K:play  I:in  O:out  E:xport  H/L:frame  J/;:5s  G:go to  Q:quit",
    };
    let hint_h = 30.0;
    let hint_y = bar_y + bar_h + 22.0;
    draw_rectangle(bar_x, hint_y - 4.0, bar_w, hint_h, BG);
    draw_text_ex(
        hints,
        bar_x,
        hint_y + 20.0,
        TextParams {
            font: Some(font),
            font_size: 22,
            color: WHITE,
            ..Default::default()
        },
    );

    if let Some((msg, _)) = player.export_message() {
        let text_width = measure_text(msg, Some(font), 20, 1.0).width;
        draw_rectangle(
            (sw - text_width) / 2.0 - 8.0,
            sh / 2.0 - 14.0,
            text_width + 16.0,
            28.0,
            BG,
        );
        draw_text_ex(
            msg,
            (sw - text_width) / 2.0,
            sh / 2.0,
            TextParams {
                font: Some(font),
                font_size: 20,
                color: Color::new(0.3, 1.0, 0.3, 1.0),
                ..Default::default()
            },
        );
    }
}
