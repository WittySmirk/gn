#ifndef EDITOR_H
#define EDITOR_H

extern "C" {
    #include <libavcodec/avcodec.h>
    #include <libavformat/avformat.h>
    #include <libswscale/swscale.h>
    #include <libswresample/swresample.h>
    #include <libavutil/imgutils.h>
    #include <libavutil/channel_layout.h>
}

#include <iostream>
#include <string>
#include <fstream>
#include <SDL3/SDL.h>
#include <vector>

#include "Element.h"

struct VideoFrame {
    SDL_Texture* text;
    double pts;
};

class Editor {
    public:
        Editor();
        void init(std::string _file, Element* _element);
        bool read();
        void renderVideo();
        void cleanup();
    private:
        AVFormatContext* pFormatCtx = nullptr;
        AVCodecContext* pCodecCtx = nullptr;
        const AVCodec* pCodec = nullptr;

        AVFormatContext* aFormatCtx = nullptr;
        AVCodecContext* aCodecCtx = nullptr;
        const AVCodec* aCodec = nullptr;

        SDL_AudioSpec aSpec;
        SDL_AudioStream* aStream;

        int videoStream = -1;
        int audioStream = -1;
        
        double getAudioClock();
        Uint32 totalBytesQueued = 0;

        AVFrame* aFrame = nullptr;

        AVFrame* pFrame = nullptr;
        AVFrame* pFrameRGB = nullptr;
        uint8_t*  buffer = nullptr;

        Element* element;

        struct SwsContext* swsCtx = nullptr;
        struct SwrContext* swrCtx = nullptr;
        AVPacket packet;

        std::vector<VideoFrame> videoQueue;
};

#endif