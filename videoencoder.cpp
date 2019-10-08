#include "videoencoder.h"
#include <QDebug>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

VideoEncoder::VideoEncoder()
    : m_bFirstFrame(false)
    , m_pCodecCtx(nullptr)
    , m_pCodec(nullptr)
    , m_pkt(nullptr)
    , m_pFile(nullptr)
    , m_pSwsContext(nullptr)
    , m_pFrame(nullptr)
    , m_framecnt(0)
{

}

VideoEncoder::~VideoEncoder()
{
    av_free_packet(m_pkt);
    if(m_pFile)
    {
        if(m_pFile->isOpen())
            m_pFile->close();
        delete m_pFile;
    }
}

bool VideoEncoder::open(const char* file)
{
    m_pCodec = avcodec_find_encoder(AV_CODEC_ID_H264);
    if (!m_pCodec) {
        fprintf(stderr, "Codec not found\n");
        return false;
    }

    m_pCodecCtx = avcodec_alloc_context3(m_pCodec);
    if (!m_pCodecCtx) {
        fprintf(stderr, "Could not allocate video codec context\n");
        return false;
    }

    m_pkt = av_packet_alloc();
    if (!m_pkt)
        return false;

    /* put sample parameters */
    m_pCodecCtx->bit_rate = 400000;
    /* resolution must be a multiple of two */
    /* frames per second */
    AVRational tb{1, 25};
    AVRational fr{25, 1};
    m_pCodecCtx->time_base = tb;
    m_pCodecCtx->framerate = fr;

    m_pCodecCtx->gop_size = 10;
    m_pCodecCtx->max_b_frames = 1;
    m_pCodecCtx->pix_fmt = AV_PIX_FMT_YUV420P;

    av_opt_set(m_pCodecCtx->priv_data, "preset", "slow", 0);

    m_pFile = new QFile(file);
    if (!m_pFile || !m_pFile->open(QFile::WriteOnly))
    {
        qDebug() << "Could not open " << file;
        return false;
    }

    return true;
}

bool VideoEncoder::addFrame(QImage image)
{
    if(!m_bFirstFrame)
    {
        m_pCodecCtx->width = image.width();
        m_pCodecCtx->height = image.height();

        /* open it */
        int ret = avcodec_open2(m_pCodecCtx, m_pCodec, nullptr);
        if (ret < 0)
        {
            qDebug() << "Could not open codec";
            return false;
        }

        m_pFrame = av_frame_alloc();
        if (!m_pFrame)
        {
            qDebug() << "Could not allocate video frame";
            return false;
        }
        m_pFrame->format = m_pCodecCtx->pix_fmt;
        m_pFrame->width  = m_pCodecCtx->width;
        m_pFrame->height = m_pCodecCtx->height;

        ret = av_frame_get_buffer(m_pFrame, 32);
        if (ret < 0)
        {
            qDebug() << "Could not allocate the video frame data";
            return false;
        }
        AVPixelFormat format;
        switch(image.format())
        {
        case QImage::Format_RGB888:
            m_channels = 3;
            format = AVPixelFormat::AV_PIX_FMT_RGB24;
            break;
        case QImage::Format_Indexed8:
        case QImage::Format_Grayscale8:
            m_channels = 1;
            format = AVPixelFormat::AV_PIX_FMT_GRAY8;
            break;
        case QImage::Format_ARGB32:
        case QImage::Format_RGB32:
        case QImage::Format_ARGB32_Premultiplied:
        default:
            m_channels = 4;
            format = AVPixelFormat::AV_PIX_FMT_RGBA;
            break;
        }
        m_pSwsContext = sws_getContext(
            m_pCodecCtx->width, m_pCodecCtx->height, format, // 输入
            m_pCodecCtx->width, m_pCodecCtx->height, AVPixelFormat::AV_PIX_FMT_YUV420P, // 输出
            SWS_BICUBIC, // 算法
            0, 0, 0);

        m_bFirstFrame = true;
    }

    m_pFrame->pts = m_framecnt++;
    const uchar *rgb = image.bits();
    uint8_t *srcSlice[AV_NUM_DATA_POINTERS] = { 0 };
    srcSlice[0] = (uint8_t *)rgb;
    int srcStride[AV_NUM_DATA_POINTERS] = { 0 };
    srcStride[0] = m_pCodecCtx->width * m_channels;

    // 转换
    int h = sws_scale(m_pSwsContext, srcSlice, srcStride, 0, m_pCodecCtx->height, m_pFrame->data, m_pFrame->linesize);
    if (h < 0) {
         qDebug() << "sws_scale failed";
         return false;
    }
    encode(m_pFrame);

    return true;
}

bool VideoEncoder::close()
{
    uint8_t endcode[] = { 0, 0, 1, 0xb7 };

    encode();

    /* add sequence end code to have a real MPEG file */
    m_pFile->write((const char*)endcode, sizeof(endcode));
    m_pFile->close();

    avcodec_free_context(&m_pCodecCtx);
    av_frame_free(&m_pFrame);
    av_packet_free(&m_pkt);

    return true;
}

bool VideoEncoder::encode(AVFrame* frame)
{
    int ret;

    /* send the frame to the encoder */
    ret = avcodec_send_frame(m_pCodecCtx, frame);
    if (ret < 0)
    {
        qDebug() << "Error sending a frame for encoding";
        return false;
    }

    while (ret >= 0)
    {
        ret = avcodec_receive_packet(m_pCodecCtx, m_pkt);
        if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF)
            return false;
        else if (ret < 0)
        {
            qDebug() << "Error during encoding";
            return false;
        }

        m_pFile->write((const char*)m_pkt->data, m_pkt->size);
        av_packet_unref(m_pkt);
    }

    return true;
}
