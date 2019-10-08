#ifndef VIDEOENCODER_H
#define VIDEOENCODER_H

#include <QObject>
#include <QImage>
#include <QFile>

extern "C"
{
#include "libavcodec/avcodec.h"
#include "libavformat/avformat.h"
#include "libavutil/pixfmt.h"
#include "libswscale/swscale.h"
#include <libavutil/opt.h>
#include <libavutil/imgutils.h>
}

class VideoEncoder : public QObject
{
    Q_OBJECT
public:
    explicit VideoEncoder();
    ~VideoEncoder();

    bool    open(const char* file);
    bool    addFrame(QImage image);
    bool    close();
private:
    bool    m_bFirstFrame;
    AVCodecContext* m_pCodecCtx;
    AVCodec* m_pCodec;
    AVPacket* m_pkt;
    QFile*   m_pFile;
    SwsContext *m_pSwsContext;
    AVFrame* m_pFrame;
    int m_framecnt;
    int m_channels;

    bool    encode(AVFrame* frame = nullptr);
};

#endif // VIDEOENCODER_H
