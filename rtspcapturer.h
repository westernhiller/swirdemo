#ifndef RTSPCAPTURER_H
#define RTSPCAPTURER_H

#include <QTimer>
#include <QThread>
#include <QImage>

extern "C"
{
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libswscale/swscale.h>
}

class RtspCapturer : public QThread
{
    Q_OBJECT

public:
    explicit RtspCapturer(QString url);
    ~RtspCapturer();

signals:
    void getFrame(QImage image);
    void sendMessage(QString message);

protected:
    void run() override;

private:
    int m_videoStream;
    QString m_rtspURL;
    AVFormatContext *m_pFormatContext;
    AVCodec *m_pCodec;			/*!<表示解码指针的变量 */
    AVCodecContext *m_pCodecContext;
    SwsContext *m_pSwsContext;		/*!<表示编码变换的变量 */
    AVPacket *m_pPacket;			/*!<表示包的变量 */
    AVFrame *m_pFrame;			/*!<表示帧的变量 */
    bool m_bWhiteBalance;

    bool init(QString url);
public slots:
    void enableWhiteBalance(bool);
};

#endif // RTSPCAPTURER_H
