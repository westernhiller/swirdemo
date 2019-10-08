#ifndef SWIRPROCESSOR_H
#define SWIRPROCESSOR_H

#include <QTimer>
#include <QThread>
#include <QImage>
#include <QMutex>
#include "global.h"

typedef struct _framebuffer {
        uint16_t image[FRAME_PXCNT];
        uint32_t param[FRAME_PMNUM];
} PACKAGEBUFFER;
typedef PACKAGEBUFFER* LPPACKAGEBUFFER;

class SwirProcessor : public QThread
{
    Q_OBJECT

public:
    explicit SwirProcessor(QObject* parent = nullptr);
    ~SwirProcessor() override;

signals:
    void getFrame(QImage image);

protected:
    void run() override;

private:
    bool m_bSmooth;
    bool m_bFPS;
    int m_nFPS;
    bool m_bHistogram;
    int m_nFrames;
    int m_nGain;
    QList<QByteArray> m_framelist;
    QMutex m_mutex;
    QMutex m_mutex2;
    PACKAGEBUFFER m_packageBuffer;

    void process(QByteArray frame);
    void clearFrames();
    void smoothImage(Mat imgFrame);
    Mat frame2Mat(uint16_t *buffer);
    void addFPS(Mat imgFrame);

private slots:
    void onTimer();

public slots:
    void parseFrame(QByteArray frame);
    void updateGain(int);
    void enableHistogram(bool);
    void enableSmooth(bool);
};

#endif // RTSPCAPTURER_H
