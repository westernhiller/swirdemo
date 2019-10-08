#ifndef CONTROLPANEL_H
#define CONTROLPANEL_H

#include <QWidget>
#include "glcanvas.h"
#include "IPACNetSDK.h"
#include "swircapturer.h"

namespace Ui {
class ControlPanel;
}

class ControlPanel : public QWidget
{
    Q_OBJECT

public:
    explicit ControlPanel(QWidget *parent = nullptr);
    ~ControlPanel() override;

signals:
    void updateGain(int nGain);
    void sendImages();
    void startRecording();
    void stopRecording();
    void updateDispMode(int);
    void updateMode(uint32_t nMode);
    void updateIntegral(double);
    void updateCycle(double);
    void enableHighgain(bool bEnable);
    void enableNonuniform(bool bEnable);
    void enableIntegral(bool bEnable);
    void updatePath(QString path);
    void enableHistogram(bool);
    void enableWhiteBalance(bool);
    void enableMirror(bool);
    void enableKeepRatio(bool);
    void enableSmooth(bool);

private:
    Ui::ControlPanel *ui;
    LONG m_lUserID;
    QString m_path;
    IPAC_DEV_VIDEOCFG m_stVideoCFG;

public slots:
    void gainChanged(int nGain);
    void contrastChanged(int nContrast);
    void brightnessChanged(int nBrightness);
    void saturateChanged(int nSaturate);
    void recordVideo();
    void saveImage();
    void modeChanged(int nMode);
    void dispMessage(QString);
    void setIntegral(double integral);
    void setFrameCycle(double cycle);
    void setMode(uint32_t mode);
    void onSetIntegral();
    void onSetCycle();
    void histogramCheckBoxChanged(int);
    void highgainCheckBoxChanged(int);
    void integralCheckBoxChanged(int);
    void nonuniformCheckBoxChanged(int);
    void whitebalanceCheckBoxChanged(int);
    void mirrorCheckBoxChanged(int);
    void smoothCheckBoxChanged(int);
    void keepratioCheckBoxChanged(int);
    void onBrowse();
};

#endif // CONTROLPANEL_H
