#include "Capture.h"

Capture::Capture() {}

void Capture::startScreenRecord(std::string _filename, Settings* _settings) {
    obs_startup("en-US", nullptr, nullptr);
    obs_add_data_path("./data/libobs/");
    obs_add_module_path("./obs-plugins/64bit/", "./data/obs-plugins/%module%/");

    std::cout << "libobs version: " << obs_get_version() << std::endl;

    obs_video_info vInfo = {};
    vInfo.adapter = 0;
    vInfo.base_width = 2560;
    vInfo.base_height = 1440;
    vInfo.output_width = 1920;
    vInfo.output_height = 1080;
    vInfo.output_format = VIDEO_FORMAT_NV12;
    vInfo.fps_num = _settings->fps;
    vInfo.fps_den = 1;
    vInfo.graphics_module = "libobs-d3d11";
    vInfo.gpu_conversion = true;
    vInfo.colorspace = VIDEO_CS_709;
    vInfo.range = VIDEO_RANGE_PARTIAL;
    vInfo.scale_type = OBS_SCALE_BILINEAR;

    if(obs_reset_video(&vInfo) != 0) {
        std::cerr << "video init failed" << std::endl;
        exit(1);
    }
    
    obs_audio_info aInfo = {};
    aInfo.samples_per_sec = 48000;
    aInfo.speakers = SPEAKERS_STEREO;
    if(!obs_reset_audio(&aInfo)) {
        std::cerr << "audio init failed" << std::endl;
        exit(1);
    }

    obs_load_all_modules();
    obs_log_loaded_modules();
    obs_post_load_modules();

    capture = obs_source_create("monitor_capture", "video capture", nullptr, nullptr);
    obs_set_output_source(0, capture);
    obs_data_t* vEncSettings = obs_data_create();
    obs_data_set_bool(vEncSettings, "use_busize", true);
    obs_data_set_string(vEncSettings, "profile", "high");
    obs_data_set_string(vEncSettings, "preset", "veryfast");
    obs_data_set_string(vEncSettings, "rate_control", "CRF");
    obs_data_set_int(vEncSettings, "crf", 20);
    obs_encoder_t* vEncoder = nullptr;
    vEncoder = obs_video_encoder_create("obs_x264", "video_recording", vEncSettings, nullptr);
    obs_encoder_set_video(vEncoder, obs_get_video());
    obs_data_release(vEncSettings);

    obs_source_t* aSource = obs_source_create("wasapi_output_capture", "audio capture", nullptr, nullptr);
    obs_set_output_source(1, aSource);
    obs_encoder_t* aEncoder = obs_audio_encoder_create("ffmpeg_aac", "simple_aac_recording", nullptr, (size_t)0, nullptr);
    obs_encoder_set_audio(aEncoder, obs_get_audio());


obs_enum_outputs([](void*, obs_output_t* out) {
    std::cout << "Output: " << obs_output_get_id(out) << "\n";
    return true;
}, nullptr);

    obs_data_t* recordOutputSettings = obs_data_create();
    obs_data_set_string(recordOutputSettings, "path", _filename.c_str());
    output= obs_output_create("ffmpeg_muxer", "simple_ffmpeg_output", recordOutputSettings, nullptr);
    obs_data_release(recordOutputSettings);

    obs_output_set_video_encoder(output, vEncoder);
    obs_output_set_audio_encoder(output, aEncoder, (size_t)0);

    if(!obs_output_start(output)) {
        std::cerr << "output failed to start" << std::endl;
        exit(1);
    }
}

void Capture::endScreenRecord() {
    obs_output_stop(output);
    obs_output_release(output);
    obs_source_release(capture);
    obs_shutdown();
}