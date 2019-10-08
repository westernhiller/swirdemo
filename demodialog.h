#ifndef DEMODIALOG_H
#define DEMODIALOG_H

#include <QDialog>
#include "glcanvas.h"
#include "swircapturer.h"
#include "swirprocessor.h"
#include "rtspcapturer.h"
#include "videoencoder.h"

class DemoDialog : public QDialog
{
    Q_OBJECT

public:
    explicit DemoDialog(QWidget *parent = nullptr);
    ~DemoDialog();

signals:
    void updateVisible(QImage image);
    void updateSwir(QImage image);
    void saveVisibleVideoFrame(QImage image);
    void saveSwirVideoFrame(QImage image);
    void updateStatus(QString message);

private:
    QImage          m_imgVisible;
    QImage          m_imgSwir;
    GLCanvas*       m_pCanvas;
    SwirCapturer*   m_pSwirCapturer;
    RtspCapturer*   m_pCamCapturer;
    SwirProcessor*  m_pSwirProcessor;
    VideoEncoder*   m_pSwirEncoder;
    VideoEncoder*   m_pVisibleEncoder;
    bool            m_bRecording;
    int             m_nHeartBeatVisible;
    int             m_nHeartBeatSwir;
    QString         m_path;

private slots:
    void gotVisible(QImage);
    void gotSwir(QImage);
    void saveImages(QImage, QImage);
    void onTimer();
    void startRecording();
    void stopRecording();
public slots:
    void updatePath(QString path);
};

#endif // DEMODIALOG_H
