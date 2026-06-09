use std::path::Path;

use ffmpeg_next::codec::decoder::Video as VideoDecoder;
use ffmpeg_next::format::context::Input;
use ffmpeg_next::software::scaling::context::Context as Scaler;
use ffmpeg_next::software::scaling::flag::Flags;
use ffmpeg_next::util::format::Pixel;
use ffmpeg_next::util::frame::video::Video as VideoFrame;
use ffmpeg_next::{Rational, Rescale};

pub struct VideoPlayer {
    ictx: Input,
    video_stream_index: usize,
    decoder: VideoDecoder,
    scaler: Scaler,
    stream_tb: Rational,
    pts_per_frame: i64,
    width: u32,
    height: u32,
    total_duration: f64,
    current_pts: i64,
    playing: bool,
    in_point: Option<i64>,
    out_point: Option<i64>,
    export_message: Option<(String, f64)>,
}

impl VideoPlayer {
    pub fn open<P: AsRef<Path>>(path: P) -> Result<Self, String> {
        let ictx =
            ffmpeg_next::format::input(&path).map_err(|e| format!("Failed to open file: {e}"))?;

        let input_stream = ictx
            .streams()
            .best(ffmpeg_next::media::Type::Video)
            .ok_or("No video stream found")?;

        let video_stream_index = input_stream.index();
        let stream_tb = input_stream.time_base();

        let context_decoder = ffmpeg_next::codec::context::Context::from_parameters(
            input_stream.parameters(),
        )
        .map_err(|e| format!("Failed to create codec context: {e}"))?;

        let decoder = context_decoder
            .decoder()
            .video()
            .map_err(|e| format!("Failed to open video decoder: {e}"))?;

        let width = decoder.width();
        let height = decoder.height();

        let scaler = Scaler::get(
            decoder.format(),
            width,
            height,
            Pixel::RGBA,
            width,
            height,
            Flags::BILINEAR,
        )
        .map_err(|e| format!("Failed to create scaler: {e}"))?;

        let total_duration =
            input_stream.duration() as f64 * stream_tb.0 as f64 / stream_tb.1 as f64;

        let fr = input_stream.avg_frame_rate();
        let frame_rate = if fr.1 > 0 { fr.0 as f64 / fr.1 as f64 } else { 30.0 };
        let pts_per_frame = if frame_rate > 0.0 {
            (stream_tb.1 as f64 / (frame_rate * stream_tb.0 as f64)).round() as i64
        } else {
            1
        }
        .max(1);

        Ok(Self {
            ictx,
            video_stream_index,
            decoder,
            scaler,
            stream_tb,
            pts_per_frame,
            width,
            height,
            total_duration,
            current_pts: 0,
            playing: false,
            in_point: None,
            out_point: None,
            export_message: None,
        })
    }

    pub fn width(&self) -> u32 {
        self.width
    }
    pub fn height(&self) -> u32 {
        self.height
    }
    pub fn current_time(&self) -> f64 {
        self.pts_to_secs(self.current_pts)
    }
    pub fn total_duration(&self) -> f64 {
        self.total_duration
    }
    pub fn is_playing(&self) -> bool {
        self.playing
    }
    pub fn toggle_play(&mut self) {
        self.playing = !self.playing;
    }
    pub fn in_point(&self) -> Option<f64> {
        self.in_point.map(|p| self.pts_to_secs(p))
    }
    pub fn out_point(&self) -> Option<f64> {
        self.out_point.map(|p| self.pts_to_secs(p))
    }
    pub fn in_point_pts(&self) -> Option<i64> {
        self.in_point
    }
    pub fn out_point_pts(&self) -> Option<i64> {
        self.out_point
    }
    pub fn stream_time_base(&self) -> Rational {
        self.stream_tb
    }
    pub fn export_message(&self) -> Option<&(String, f64)> {
        self.export_message.as_ref()
    }

    pub fn set_in_point(&mut self) {
        self.in_point = Some(self.current_pts);
    }
    pub fn set_out_point(&mut self) {
        self.out_point = Some(self.current_pts);
    }
    pub fn set_export_message(&mut self, msg: String) {
        self.export_message = Some((msg, 0.0));
    }

    pub fn update_export_timer(&mut self, dt: f64) {
        if let Some((_, ref mut t)) = self.export_message {
            *t += dt;
            if *t > 3.0 {
                self.export_message = None;
            }
        }
    }

    fn pts_to_secs(&self, pts: i64) -> f64 {
        pts as f64 * self.stream_tb.0 as f64 / self.stream_tb.1 as f64
    }

    fn pts_to_av_ts(&self, pts: i64) -> i64 {
        pts.rescale(self.stream_tb, ffmpeg_next::rescale::TIME_BASE)
    }

    pub fn decode_next(&mut self) -> Option<Vec<u8>> {
        for (stream, packet) in self.ictx.packets() {
            if stream.index() == self.video_stream_index {
                self.decoder.send_packet(&packet).ok()?;
                let mut frame = VideoFrame::empty();
                if self.decoder.receive_frame(&mut frame).is_ok() {
                    if let Some(pts) = frame.pts() {
                        self.current_pts = pts;
                    }
                    return Some(self.convert_frame(&frame));
                }
            }
        }
        None
    }

    pub fn seek_to(&mut self, target_pts: i64) -> Option<Vec<u8>> {
        let seek_ts = self.pts_to_av_ts(target_pts);
        self.ictx.seek(seek_ts, ..seek_ts).ok()?;
        self.decoder.flush();

        for (stream, packet) in self.ictx.packets() {
            if stream.index() == self.video_stream_index {
                self.decoder.send_packet(&packet).ok()?;
                let mut frame = VideoFrame::empty();
                while let Ok(()) = self.decoder.receive_frame(&mut frame) {
                    let pts = frame.pts().unwrap_or(0);
                    self.current_pts = pts;
                    if pts >= target_pts {
                        return Some(self.convert_frame(&frame));
                    }
                }
            }
        }
        None
    }

    pub fn seek_to_time(&mut self, seconds: f64) -> Option<Vec<u8>> {
        let pts_delta = (seconds * self.stream_tb.1 as f64 / self.stream_tb.0 as f64) as i64;
        let target = (self.current_pts + pts_delta).max(0);
        self.seek_to(target)
    }

    pub fn step_frames(&mut self, count: i64) -> Option<Vec<u8>> {
        if count > 0 {
            let mut result = None;
            for _ in 0..count {
                result = self.decode_next();
                if result.is_none() {
                    break;
                }
            }
            result
        } else {
            let target = (self.current_pts + count * self.pts_per_frame).max(0);
            self.seek_to(target)
        }
    }

    fn convert_frame(&mut self, frame: &VideoFrame) -> Vec<u8> {
        let mut rgba_frame = VideoFrame::empty();
        if self.scaler.run(frame, &mut rgba_frame).is_err() {
            return vec![0u8; (self.width * self.height * 4) as usize];
        }

        let src = rgba_frame.data(0);
        let stride = rgba_frame.stride(0);
        let w = self.width as usize;
        let h = self.height as usize;
        let mut rgba = Vec::with_capacity(w * h * 4);
        for y in 0..h {
            let start = y * stride;
            rgba.extend_from_slice(&src[start..start + w * 4]);
        }
        rgba
    }
}
