#include "rtspcapturer.h"
#include <QDebug>
#include <QDateTime>
#ifdef Q_OS_WIN32
#else
#include <sys/time.h>
#include <unistd.h>
#endif

#include "global.h"

RtspCapturer::RtspCapturer(QString url)
    : m_videoStream(-1)
    , m_rtspURL(url)
    , m_pFormatContext(nullptr)
    , m_pCodec(nullptr)
    , m_pCodecContext(nullptr)
    , m_pSwsContext(nullptr)
    , m_pPacket(nullptr)
    , m_pFrame(nullptr)
    , m_bWhiteBalance(false)
{
}

RtspCapturer::~RtspCapturer()
{
    requestInterruption();
    av_free(m_pPacket);
    av_free(m_pFrame);
    if(m_pCodecContext)
        avcodec_close(m_pCodecContext);
    if(m_pSwsContext)
        sws_freeContext(m_pSwsContext);
    quit();
    wait();
}

void RtspCapturer::enableWhiteBalance(bool bEnable)
{
    m_bWhiteBalance = bEnable;
}

void RtspCapturer::run()
{
    if(!init(m_rtspURL))
        return;

    int nFrame = 0;
    AVFrame *pFrameRGB = av_frame_alloc();
    int numBytes = avpicture_get_size(AV_PIX_FMT_RGB24, m_pCodecContext->width, m_pCodecContext->height);
    uint8_t* pBuffer = (uint8_t *)av_malloc(numBytes * sizeof(uint8_t));
    avpicture_fill((AVPicture *)pFrameRGB, pBuffer, AV_PIX_FMT_RGB24,
        m_pCodecContext->width, m_pCodecContext->height);

    while (!isInterruptionRequested())
    {
        if (av_read_frame(m_pFormatContext, m_pPacket) < 0)
        {
            break;                                 //这里认为视频读取完了
        }
        if (m_pPacket->stream_index == m_videoStream)
        {
            int got_picture;
            int ret = avcodec_decode_video2(m_pCodecContext, m_pFrame, &got_picture, m_pPacket);
            if (ret < 0)
            {
 //               qDebug() << "decode error.";
                continue;
            }

            if (got_picture)
            {
                sws_scale(m_pSwsContext, (uint8_t const * const *)m_pFrame->data,
                    m_pFrame->linesize, 0, m_pCodecContext->height,
                          pFrameRGB->data,  pFrameRGB->linesize);

                if(m_bWhiteBalance)
                {
                    Mat img(m_pCodecContext->height, m_pCodecContext->width, CV_8UC3, pBuffer);
                    emit getFrame(mat2qimage(autoWhiteBalance(img)));
                }
                else
                {
                    QImage tmpImg((uchar *)pBuffer, m_pCodecContext->width, m_pCodecContext->height, QImage::Format_RGB888);
                    emit getFrame(tmpImg);    //发送信号
//                    qDebug() << QDateTime::currentDateTime().toString("yyyy-MM-dd HH:mm:ss") << ". get a frame";
                }
            }
        }
        msleep(5);
    }

    av_free(pBuffer);
    av_free(pFrameRGB);
    avcodec_close(m_pCodecContext);
    avformat_close_input(&m_pFormatContext);
}

bool RtspCapturer::init(QString url)
{
    //Allocate an AVFormatContext.
    avformat_network_init();
    av_register_all();

    m_pFormatContext = avformat_alloc_context();

    AVDictionary *avdic = nullptr;
    char option_key[] = "rtsp_transport";
    char option_value[] = "tcp";
    av_dict_set(&avdic, option_key, option_value, 0);
    char option_key2[] = "max_delay";
    char option_value2[] = "100";
    av_dict_set(&avdic, option_key2, option_value2, 0);

    if (avformat_open_input(&m_pFormatContext, url.toStdString().c_str(), nullptr, &avdic) != 0)
    {
        qDebug() << "Can't open the RTSP stream " << url;
        emit sendMessage(tr("Can't open the RTSP stream"));
        return false;
    }

    if (avformat_find_stream_info(m_pFormatContext, nullptr) < 0)
    {
        qDebug() << "Could't find stream infomation.";
        emit sendMessage(tr("Can't find stream infomation."));
        return false;
    }

    for (int i = 0; i < m_pFormatContext->nb_streams; i++)
    {
        if (m_pFormatContext->streams[i]->codec->codec_type == AVMEDIA_TYPE_VIDEO)
        {
            m_videoStream = i;
        }
    }

    //如果videoStream为-1 说明没有找到视频流
    if (-1 == m_videoStream)
    {
        qDebug() << "Didn't find a video stream.";
        emit sendMessage(tr("Didn't find a video stream."));
        return false;
    }

    //查找解码器
    m_pCodecContext = m_pFormatContext->streams[m_videoStream]->codec;
    m_pCodecContext->bit_rate = 0;           //初始化为0
    m_pCodecContext->time_base.num = 1;     //下面两行：一秒钟25帧
    m_pCodecContext->time_base.den = 10;
    m_pCodecContext->frame_number = 1;      //每包一个视频帧

    m_pCodec = avcodec_find_decoder(m_pCodecContext->codec_id);
    if (nullptr == m_pCodec)
    {
        qDebug() << "Codec not found.";
        emit sendMessage(tr("Codec not found."));
        return false;
    }

    //打开解码器
    if (avcodec_open2(m_pCodecContext, m_pCodec, nullptr) < 0)
    {
        qDebug() << "Could not open codec.";
        emit sendMessage(tr("Could not open codec."));
        return false;
    }

    int y_size = m_pCodecContext->width * m_pCodecContext->height;
    m_pPacket = (AVPacket *)malloc(sizeof(AVPacket)); //分配一个packet
    av_new_packet(m_pPacket, y_size);                  //分配packet的数据

//    av_dump_format(m_pFormatContext, 0, url.toStdString().c_str(), 0);
    m_pFrame = av_frame_alloc();
    m_pSwsContext = sws_getContext(m_pCodecContext->width, m_pCodecContext->height,
                                   m_pCodecContext->pix_fmt, m_pCodecContext->width,
                                   m_pCodecContext->height, AV_PIX_FMT_RGB24,
                                   SWS_BICUBIC, nullptr, nullptr, nullptr);

    return true;
}
