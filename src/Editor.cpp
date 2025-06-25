#include "Editor.h"

Editor::Editor() {}

void Editor::init(std::string _file, Element* _element) {
    element = _element;
    if(avformat_open_input(&pFormatCtx, _file.c_str(), nullptr, nullptr) < 0) {
        std::cerr << "Could not open file" << std::endl;
        exit(1);
    }
    if(avformat_find_stream_info(pFormatCtx, nullptr) < 0) {
        std::cerr << "Could not find stram info" << std::endl;
        exit(1);
    }

    // av_dump_format(pFormatCtx, 0, _file.c_str(), 0); // Dumps to error (TODO: remove this later)

    for(int i = 0; i < pFormatCtx->nb_streams; i++) {
        if(pFormatCtx->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_VIDEO && videoStream < 0) {
            videoStream = i;
        }
        if(pFormatCtx->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_AUDIO && audioStream <0) {
            audioStream = i;
        }
    }

    if(videoStream == -1) {
        std::cerr << "Could not find the video stream" << std::endl;
        exit(1);
    }

    if(audioStream == -1) {
        std::cerr << "Could not find the audio stream" << std::endl;
        exit(1);
    }

    pCodec = avcodec_find_decoder(pFormatCtx->streams[videoStream]->codecpar->codec_id);
    if(!pCodec) {
        std::cerr << "Could not find video coedc" << std::endl;
        exit(1);
    }

    pCodecCtx = avcodec_alloc_context3(pCodec);
    if(!pCodecCtx) {
        std::cerr << "Could not allocate video codec" << std::endl;
        exit(1);
    }

    if(avcodec_parameters_to_context(pCodecCtx, pFormatCtx->streams[videoStream]->codecpar) < 0) {
        std::cerr << "Could not copy video codecpar" << std::endl;
        exit(1);
    }

    if(avcodec_open2(pCodecCtx, pCodec, nullptr) < 0) {
        std::cerr << "Could not open video codec" << std::endl;
        exit(1);
    }

    aCodec = avcodec_find_decoder(pFormatCtx->streams[audioStream]->codecpar->codec_id);
    if(!aCodec) {
        std::cerr << "Could not find audio codec" << std::endl;
        exit(1);
    }

    aCodecCtx = avcodec_alloc_context3(aCodec);
    if(avcodec_parameters_to_context(aCodecCtx, pFormatCtx->streams[audioStream]->codecpar) < 0) {
        std::cerr << "Could not copy audio codecpar" << std::endl;
        exit(1);
    }

    if(avcodec_open2(aCodecCtx, aCodec, nullptr) < 0) {
        std::cerr << "Could not open audio codec" << std::endl;
        exit(1);
    }

    aSpec.format = SDL_AUDIO_S16;
    aSpec.freq = aCodecCtx->sample_rate;
    aSpec.channels = aCodecCtx->ch_layout.nb_channels;

    if(SDL_Init(SDL_INIT_AUDIO) < 0) {
        std::cerr << "Could not init SDL audio: " << SDL_GetError() << std::endl;
        exit(1);
    }

    aStream = SDL_OpenAudioDeviceStream(SDL_AUDIO_DEVICE_DEFAULT_PLAYBACK, &aSpec, nullptr, nullptr);
    if(!aStream) {
        std::cerr << "SDL_OpenAudioDeviceStream error: " << SDL_GetError() << std::endl;
        exit(1);
    }

    AVChannelLayout out = {};
    av_channel_layout_default(&out, aSpec.channels);
    AVChannelLayout in = {};
    av_channel_layout_default(&in, aCodecCtx->ch_layout.nb_channels);

    swr_alloc_set_opts2(&swrCtx,
        &out,
        AV_SAMPLE_FMT_S16,
        aSpec.freq,
        &in,
        aCodecCtx->sample_fmt,
        aCodecCtx->sample_rate,
        0,
        nullptr
    );

    swr_init(swrCtx);

    aFrame = av_frame_alloc();

    pFrame = av_frame_alloc();
    pFrameRGB = av_frame_alloc();

    int numBytes;
    
    numBytes = av_image_get_buffer_size(AV_PIX_FMT_YUV420P, pCodecCtx->width, pCodecCtx->height, 1);
    
    buffer = (uint8_t*)av_malloc(numBytes * sizeof(uint8_t));
    if(!buffer) {
        std::cerr << "Could not allocate buffer" << std::endl;
        exit(1);
    }

    pFrameRGB->format = AV_PIX_FMT_YUV420P;
    pFrameRGB->width = pCodecCtx->width;
    pFrameRGB->height = pCodecCtx->height;

    if(av_image_fill_arrays(
        pFrameRGB->data, pFrameRGB->linesize, buffer, AV_PIX_FMT_YUV420P, pCodecCtx->width, pCodecCtx->height, 1
    ) < 0) {
        std::cerr << "Could not create pFrameRGB" << std::endl;
        exit(1);
    }

    swsCtx = sws_getContext(
        pCodecCtx->width, pCodecCtx->height, pCodecCtx->pix_fmt, 
        pCodecCtx->width, pCodecCtx->height, AV_PIX_FMT_YUV420P, 
        SWS_BILINEAR, nullptr, nullptr, nullptr);
}

bool Editor::read() {
    int i = 0;
    SDL_ResumeAudioStreamDevice(aStream);
    if (av_read_frame(pFormatCtx, &packet) >= 0) {
        if(packet.stream_index == videoStream) {
            if(avcodec_send_packet(pCodecCtx, &packet) < 0) {
                std::cout << "Failed to send packet" << std::endl;
                return false;
            }

            if(avcodec_receive_frame(pCodecCtx, pFrame) == 0) {
                sws_scale(swsCtx, (uint8_t const* const*)pFrame->data,
                    pFrame->linesize, 0, pCodecCtx->height, 
                    pFrameRGB->data, pFrameRGB->linesize);
                SDL_Texture* text = SDL_CreateTexture(element->getRenderer(), SDL_PIXELFORMAT_IYUV, SDL_TEXTUREACCESS_STREAMING, pCodecCtx->width, pCodecCtx->height);
                if(!text) {
                    std::cerr << "Failed to create texture" << std::endl;
                    exit(1);
                }

                SDL_UpdateTexture(text, nullptr, pFrameRGB->data[0], pFrameRGB->linesize[0]);

                SDL_FRect* rect = new SDL_FRect{0,0, SCREEN_W, SCREEN_H};

                element->setTexture(text);
                if(element->getRect() == nullptr) {
                    element->setRect(rect);
                }
            }
        } else if (packet.stream_index == audioStream) {
            if(avcodec_send_packet(aCodecCtx, &packet) < 0) {
                std::cerr << "Failed to send audio packet" << std::endl;
                return false;
            }
            if(avcodec_receive_frame(aCodecCtx, aFrame) == 0) {
                uint8_t* outBuf = NULL;
                int outLineSize;

                int out_samples = av_rescale_rnd(
                    swr_get_delay(swrCtx, aCodecCtx->sample_rate) + aFrame->nb_samples,
                    aSpec.freq, aCodecCtx->sample_rate,
                    AV_ROUND_UP);

                av_samples_alloc(&outBuf, &outLineSize, aSpec.channels, out_samples, AV_SAMPLE_FMT_S16, 0);

                int converted_samples = swr_convert(swrCtx, &outBuf, out_samples, (const uint8_t**)aFrame->data, aFrame->nb_samples);

                int buffer_size = av_samples_get_buffer_size(NULL, aSpec.channels, converted_samples, AV_SAMPLE_FMT_S16, 1);

                SDL_PutAudioStreamData(aStream, outBuf, buffer_size);

                av_freep(&outBuf);
            }
        }
        av_packet_unref(&packet);
        return true;
    }

    return false;
}

void Editor::cleanup() {
    av_free(buffer);
    av_frame_free(&pFrameRGB);
    av_frame_free(&pFrame);
    avcodec_close(pCodecCtx);
    avformat_close_input(&pFormatCtx);
    
}
