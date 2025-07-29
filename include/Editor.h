#ifndef EDITOR_H
#define EDITOR_H

extern "C" {
    #include <libavcodec/avcodec.h>
    #include <libavformat/avformat.h>
    #include <libswscale/swscale.h>
    #include <libswresample/swresample.h>
    #include <libavutil/imgutils.h>
    #include <libavutil/channel_layout.h>
    #include <libavcodec/bsf.h>
}

#include <iostream>
#include <string>
#include <fstream>
#include <SDL3/SDL.h>
#include <vector>
#include <thread>

#include "Settings.h"
#include "Element.h"
#include "Input.h"

struct VideoFrame {
    SDL_Texture* text;
    double pts;
};

// The screen itself will be this class, which has children for the timeline
class Editor: public Element {
    public:
        Editor(SDL_Renderer* _renderer, TTF_Font* _font, Settings* _settings);
        ~Editor();
        void init(std::string _file);
        bool read();
        void renderVideo();
        void createMarker();
        void clearMarkers();
        void exportClip();
        void completeExport();
        void seek(double _offSeconds);

        void draw();
        void collectText(std::string _text);
        void deleteText();

        bool getFocused();
        bool getPaused();
        void setPaused(bool _paused);
    private:
        void finishSeek(double _target);
        AVFormatContext* pFormatCtx = nullptr;
        AVCodecContext* pCodecCtx = nullptr;
        const AVCodec* pCodec = nullptr;

        AVFormatContext* aFormatCtx = nullptr;
        AVCodecContext* aCodecCtx = nullptr;
        const AVCodec* aCodec = nullptr;

        SDL_AudioSpec aSpec;
        SDL_AudioStream* aStream;

        double videoDuration = 0.0;
        int videoStream = -1;
        int audioStream = -1;
        
        double getAudioClock();
        double audioClockBase = 0;
        Uint32 totalBytesQueued = 0;

        AVFrame* aFrame = nullptr;

        AVFrame* pFrame = nullptr;
        AVFrame* pFrameRGB = nullptr;
        uint8_t*  buffer = nullptr;

        struct SwsContext* swsCtx = nullptr;
        struct SwrContext* swrCtx = nullptr;
        AVPacket packet;

        std::vector<VideoFrame> videoQueue;
        std::vector<Element*> children;

        double markerOne = -1.0;
        double markerTwo = -1.0;

        bool gettingInput = false; 
        int currentSeek = 1;
        TTF_Font* font;

        std::atomic<bool> seeking = false;
        bool paused = false;
        std::thread seekThread;

        Settings* settings;
};

#endif