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

    av_dump_format(pFormatCtx, 0, _file.c_str(), 0); // Dumps to error (TODO: remove this later)
    

    for(int i = 0; i < pFormatCtx->nb_streams; i++) {
        if(pFormatCtx->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_VIDEO) {
            videoStream = i;
            break;
        }
    }

    if(videoStream == -1) {
        std::cerr << "Could not find the video stream" << std::endl;
        exit(1);
    }

    pCodec = avcodec_find_decoder(pFormatCtx->streams[videoStream]->codecpar->codec_id);
    if(!pCodec) {
        std::cerr << "Could not find the coedc" << std::endl;
        exit(1);
    }

    pCodecCtx = avcodec_alloc_context3(pCodec);
    if(!pCodecCtx) {
        std::cerr << "Could not allocate codec" << std::endl;
        exit(1);
    }

    if(avcodec_parameters_to_context(pCodecCtx, pFormatCtx->streams[videoStream]->codecpar) < 0) {
        std::cerr << "Could not copy codecpar" << std::endl;
        exit(1);
    }

    if(avcodec_open2(pCodecCtx, pCodec, nullptr) < 0) {
        std::cerr << "Could not open codec" << std::endl;
        exit(1);
    }

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

    sws_ctx = sws_getContext(
        pCodecCtx->width, pCodecCtx->height, pCodecCtx->pix_fmt, 
        pCodecCtx->width, pCodecCtx->height, AV_PIX_FMT_YUV420P, 
        SWS_BILINEAR, nullptr, nullptr, nullptr);
}

void Editor::read() {
    int i = 0;
    while(av_read_frame(pFormatCtx, &packet) >= 0) {
        if(packet.stream_index == videoStream) {
            if(avcodec_send_packet(pCodecCtx, &packet) < 0) {
                std::cout << "Failed to send packet" << std::endl;
                exit (1);
            }

            while(avcodec_receive_frame(pCodecCtx, pFrame) == 0) {
                sws_scale(sws_ctx, (uint8_t const* const*)pFrame->data,
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
            av_packet_unref(&packet);
        }
    }

    av_free(buffer);
    av_frame_free(&pFrameRGB);
    av_frame_free(&pFrame);
    avcodec_close(pCodecCtx);
    avformat_close_input(&pFormatCtx);
}
