#ifndef EDITOR_H
#define EDITOR_H

extern "C" {
    #include <libavcodec/avcodec.h>
    #include <libavformat/avformat.h>
    #include <libswscale/swscale.h>
    #include <libavutil/imgutils.h>
}

#include <iostream>
#include <string>
#include <fstream>
#include <SDL3/SDL.h>

#include "Element.h"

class Editor {
    public:
        Editor();
        void init(std::string _file, Element* _element);
        bool read();
        void cleanup();
    private:
        AVFormatContext* pFormatCtx = nullptr;
        AVCodecContext* pCodecCtx = nullptr;
        const AVCodec* pCodec = nullptr;

        int videoStream = -1;
        
        AVFrame* pFrame = nullptr;
        AVFrame* pFrameRGB = nullptr;
        uint8_t*  buffer = nullptr;

        Element* element;

        struct SwsContext* sws_ctx = nullptr;
        AVPacket packet;
};

#endif