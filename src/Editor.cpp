#include "Editor.h"

Editor::Editor(SDL_Renderer* _renderer, TTF_Font* _font): Element(_renderer), font(_font) {}

Editor::~Editor() {
    av_free(buffer);
    av_frame_free(&pFrameRGB);
    av_frame_free(&pFrame);
    av_frame_free(&aFrame);
    avcodec_free_context(&pCodecCtx);
    avcodec_free_context(&aCodecCtx);
    avformat_close_input(&pFormatCtx);
    avformat_close_input(&aFormatCtx);

    for(Element* e : children) {
        delete e;
    }
}

void Editor::init(std::string _file) {
    if(avformat_open_input(&pFormatCtx, _file.c_str(), nullptr, nullptr) < 0) {
        std::cerr << "Could not open file" << std::endl;
        exit(1);
    }
    if(avformat_find_stream_info(pFormatCtx, nullptr) < 0) {
        std::cerr << "Could not find stram info" << std::endl;
        exit(1);
    }

    videoDuration = (double)pFormatCtx->duration / AV_TIME_BASE; 
    // av_dump_format(pFormatCtx, 0, _file.c_str(), 0); // Dumps to error (TODO: remove this later)

    for(unsigned int i = 0; i < pFormatCtx->nb_streams; i++) {
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

    if(!SDL_Init(SDL_INIT_AUDIO)) {
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

    Element* fullSeek = new Element(renderer);
    SDL_FRect* fullRect = new SDL_FRect{SCREEN_W * 0.1, SCREEN_H * 0.9, SCREEN_W * 0.8, 25.0f};
    fullSeek->setRect(fullRect);
    SDL_Color softGray = SDL_Color{211, 211, 211, 255};
    fullSeek->setBackColor(softGray);
    children.push_back(fullSeek);

    Element* partialSeek = new Element(renderer);
    SDL_FRect* partialRect = new SDL_FRect{fullRect->x, fullRect->y, 0.0f, 0.0f};
    partialSeek->setRect(partialRect);
    partialSeek->setBackColor(FOREGROUND);
    children.push_back(partialSeek);
}

bool Editor::read() {
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
                SDL_Texture* text = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_IYUV, SDL_TEXTUREACCESS_STREAMING, pCodecCtx->width, pCodecCtx->height);
                if(!text) {
                    std::cerr << "Failed to create texture" << std::endl;
                    exit(1);
                }

                SDL_UpdateTexture(text, nullptr, pFrameRGB->data[0], pFrameRGB->linesize[0]);

                double pts = packet.pts * av_q2d(pFormatCtx->streams[videoStream]->time_base);

                videoQueue.push_back({text, pts});

                if(getRect() == nullptr) {
                    SDL_FRect* rect = new SDL_FRect{0,0, SCREEN_W, SCREEN_H};
                    setRect(rect);
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

                int out_samples = (int)av_rescale_rnd(
                    swr_get_delay(swrCtx, aCodecCtx->sample_rate) + aFrame->nb_samples,
                    aSpec.freq, aCodecCtx->sample_rate,
                    AV_ROUND_UP);

                av_samples_alloc(&outBuf, &outLineSize, aSpec.channels, out_samples, AV_SAMPLE_FMT_S16, 0);

                int convertedSamples = swr_convert(swrCtx, &outBuf, out_samples, (const uint8_t**)aFrame->data, aFrame->nb_samples);

                int bufferSize = av_samples_get_buffer_size(NULL, aSpec.channels, convertedSamples, AV_SAMPLE_FMT_S16, 1);

                SDL_PutAudioStreamData(aStream, outBuf, bufferSize);

                totalBytesQueued += bufferSize;

                av_freep(&outBuf);
            }
        }
        av_packet_unref(&packet);
        return true;
    }
    return false;
}

double Editor::getAudioClock() {
    Uint32 bytesLeft = SDL_GetAudioStreamQueued(aStream);
    Uint32 bytesPlayed = totalBytesQueued > bytesLeft ? totalBytesQueued - bytesLeft : 0;
    //Uint32 bytesPlayed = totalBytesQueued - bytesLeft;

    int bitsPerSample = SDL_AUDIO_BITSIZE(aSpec.format) / 8;
    double seconds = (double)bytesPlayed / (aSpec.channels * aSpec.freq * bitsPerSample);
    return audioClockBase + seconds;
}

void Editor::renderVideo() {
    if(!videoQueue.empty()) {
        if(videoQueue[0].pts <= getAudioClock()) {
            std::cout << "Audio: " << getAudioClock() << "Video: " << videoQueue[0].pts << std::endl;
            setTexture(videoQueue[0].text);
            double pts = videoQueue[0].pts;
            videoQueue.erase(videoQueue.begin());

            double percentage = pts / videoDuration;
            SDL_FRect* full = children[0]->getRect();
            if(currentSeek == 1) {
                SDL_FRect* newPartial = new SDL_FRect {full->x, full->y, full->w * (float)percentage, full->h};
                children[currentSeek]->setRect(newPartial);
                return;
            }
            SDL_FRect* last = children[currentSeek-1]->getRect();
            float newW = (full->w * (float)percentage) - last->w;
            SDL_FRect* newPartial = new SDL_FRect {last->x + last->w, last->y, newW, last->h};
            children[currentSeek]->setRect(newPartial);
        }
    }
}

void Editor::createMarker() {
    if(gettingInput) {
        return;
    }
    if(!videoQueue.empty()) {
        if(markerOne == -1.0) {
            markerOne = videoQueue[0].pts;
            Element* nextSeek = new Element(renderer);
            SDL_FRect* old = children[currentSeek]->getRect();
            SDL_FRect* nextRect = new SDL_FRect {old->x + old->w, old->y, 0.0, 0.0};
            nextSeek->setRect(nextRect);
            nextSeek->setBackColor(HIGHLIGHT);
            children.push_back(nextSeek);

            currentSeek = 2;
        } else if(markerTwo == -1.0) {
            markerTwo = videoQueue[0].pts;
            currentSeek = 1;
        }
    }
}

void Editor::clearMarkers() {
    if(gettingInput) {
        children.pop_back();
        gettingInput = false;
        return;
    }
    if(markerTwo != -1.0 || markerOne != -1.0) {
        children.pop_back();
        currentSeek = 1;
        markerOne = -1.0;
        markerTwo = -1.0;
    }
}

void Editor::exportClip() {
    if(gettingInput)
        return;
    if(markerOne == -1.0 || markerTwo == -1.0)
        return;
    
    Input* input = new Input(renderer, font, "Enter Clip Name", 1.0f, 40); 
    children.push_back(input);
    gettingInput = true;

    input->setFocused(true);
}

void Editor::completeExport() {
    if(!gettingInput)
        return;
    
    std::string clipName = children.back()->getText();
    children.pop_back();
    gettingInput = false;

    std::cout << clipName << std::endl;
}

void Editor::seek(double _offSeconds) {
    AVStream* vStream = pFormatCtx->streams[videoStream];
    double current = videoQueue.empty() ? getAudioClock() : videoQueue[0].pts;
    double target = std::max(0.0, current + _offSeconds);
    int64_t seekTarget = target / av_q2d(vStream->time_base);

    if (av_seek_frame(pFormatCtx, videoStream, seekTarget, AVSEEK_FLAG_BACKWARD) < 0) {
        std::cerr << "Seek failed to " << target << std::endl;
        return;
    }

    avcodec_flush_buffers(pCodecCtx);
    avcodec_flush_buffers(aCodecCtx);
    SDL_ClearAudioStream(aStream);
    totalBytesQueued = 0;
    audioClockBase = target;
    setTexture(nullptr);

    for (VideoFrame& f : videoQueue)
        SDL_DestroyTexture(f.text);
    videoQueue.clear();

    av_frame_unref(pFrame);
    av_packet_unref(&packet);

    bool found = false;
    const double EPS = 0.01; // Allow minor float error
    while (!found && av_read_frame(pFormatCtx, &packet) >= 0) {
        if (packet.stream_index == videoStream) {
            if (avcodec_send_packet(pCodecCtx, &packet) >= 0 &&
                avcodec_receive_frame(pCodecCtx, pFrame) == 0) {

                double framePts = pFrame->pts * av_q2d(vStream->time_base);
                if (framePts + EPS >= target) {
                    // Convert and show this frame
                    sws_scale(swsCtx, (uint8_t const* const*)pFrame->data,
                        pFrame->linesize, 0, pCodecCtx->height,
                        pFrameRGB->data, pFrameRGB->linesize);

                    SDL_Texture* text = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_IYUV, SDL_TEXTUREACCESS_STREAMING, pCodecCtx->width, pCodecCtx->height);
                    SDL_UpdateTexture(text, nullptr, pFrameRGB->data[0], pFrameRGB->linesize[0]);

                    setTexture(text); // Show this frame immediately
                    videoQueue.push_back({text, framePts});
                    found = true;
                }
            }
        }
        av_packet_unref(&packet);
    }

    int preload = 5;
    while (preload-- > 0 && read()) {}

    std::cout << "Seeked to " << target << std::endl;
}

// Overloads for element
void Editor::draw() {
    Element::draw();

    for(Element* e : children) {
        e->draw();
    }
}

void Editor::collectText(std::string _text) {
    if(gettingInput)
        children.back()->collectText(_text);
}

void Editor::deleteText() {
    children.back()->deleteText();
}

bool Editor::getFocused() {
    if(gettingInput) {
        return children.back()->getFocused();
    }
    return false;
}

bool Editor::getPaused() {
    return paused;
}

void Editor::setPaused(bool _paused) {
    if(_paused)
        SDL_PauseAudioStreamDevice(aStream);
    paused = _paused;
}