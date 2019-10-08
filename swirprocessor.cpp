#include "swirprocessor.h"
#include <QDebug>
#include <QDateTime>

#include "global.h"

SwirProcessor::SwirProcessor(QObject* parent)
    :QThread(parent)
    , m_bSmooth(false)
    , m_bFPS(false)
    , m_nFPS(0)
    , m_bHistogram(false)
    , m_nGain(8)
{
    if(m_bFPS)
    {
        QTimer* t = new QTimer(this);
        connect(t, SIGNAL(timeout()), this, SLOT(onTimer()));
        t->start(1000);
    }
}

SwirProcessor::~SwirProcessor()
{
    requestInterruption();
    clearFrames();
    quit();
    wait();
}

void SwirProcessor::clearFrames()
{
    m_mutex.lock();
    while(!m_framelist.empty())
        m_framelist.pop_front();
    m_mutex.unlock();
}

void SwirProcessor::onTimer()
{
    m_mutex2.lock();
    m_nFPS = m_nFrames;
    m_nFrames = 0;
    m_mutex2.unlock();
}

void SwirProcessor::run()
{
    while (!isInterruptionRequested())
    {
        m_mutex.lock();
        if(!m_framelist.empty())
        {
            QByteArray frame = m_framelist.front();
            m_framelist.pop_front();
            process(frame);
        }
        m_mutex.unlock();
        msleep(5);
    }
}

void SwirProcessor::parseFrame(QByteArray frame)
{
    m_mutex.lock();
    while(m_framelist.size() > 5)
        m_framelist.pop_front();
    m_framelist.append(frame);
    m_mutex.unlock();
}

void SwirProcessor::process(QByteArray frame)
{
    memcpy(&m_packageBuffer, frame.data(), FRAMEBUFSIZE);

    Mat imgFrame = frame2Mat(m_packageBuffer.image);
    if(m_bHistogram)
        equalizeHist(imgFrame, imgFrame);

    if(m_bSmooth)
        smoothImage(imgFrame);

    if(m_bFPS)
        addFPS(imgFrame);

    QImage tmpImg = mat2qimage(imgFrame);

    m_mutex2.lock();
    m_nFrames++;
    m_mutex2.unlock();

    emit getFrame(tmpImg);    //发送信号
}

void SwirProcessor::updateGain(int nGain)
{
    // change to 0 ~ 9 index
    m_nGain = nGain - 1;
}

void SwirProcessor::addFPS(Mat imgFrame)
{
    QString strFPS = tr("FPS: ") + QString::number(m_nFPS);
    cv::putText(imgFrame, strFPS.toStdString(), Point(20, 80), FONT_HERSHEY_PLAIN, 3, Scalar(255, 255, 255));
}

void SwirProcessor::enableHistogram(bool bEnable)
{
    m_bHistogram = bEnable;
}


void SwirProcessor::enableSmooth(bool bEnable)
{
    m_bSmooth = bEnable;
}


Mat SwirProcessor::frame2Mat(uint16_t *buffer)
{
    static uint8_t unit[10] = {1, 2, 3, 4, 6, 8, 12, 16, 24, 32};
    double alpha = 1.0 / unit[m_nGain];

    Mat matFrame(SWIRHEIGHT, SWIRWIDTH, CV_8UC1);
    //    uint8_t goffset = offset[m_nGain];
    double acc = 0.0;
    for(int k = 0; k < FRAME_PXCNT; k++)
        acc += buffer[k];
    int avg= static_cast<int>(acc/FRAME_PXCNT);
    int k = 0;
    for(int i = 0; i < SWIRHEIGHT; i++)
    {
        uchar* ptr = static_cast<uchar*>(matFrame.ptr<uchar>(i));
        for(int j = 0; j < SWIRWIDTH; j++, k++)
            ptr[j] = saturate_cast<uchar>((buffer[k] - avg)* alpha + 128);
    }

    return matFrame;
}

void SwirProcessor::smoothImage(Mat imgFrame)
{
    static Mat imgPrev[3];
    static int nImages = 0;
    static int idx = 0;

    if(nImages < 3)
    {
        imgPrev[nImages++] = imgFrame/4;
    }
    else
    {
        Mat imgQuarter = imgFrame / 4;
        imgFrame = imgPrev[0] + imgPrev[1] + imgPrev[2] + imgQuarter;
        imgQuarter.copyTo(imgPrev[idx++]);
        if(idx == 3)
            idx = 0;
    }
}
