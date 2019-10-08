#include "controlpanel.h"
#include "ui_controlpanel.h"
#include <QStandardPaths>
#include <QFileDialog>
#include "global.h"

ControlPanel::ControlPanel(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::ControlPanel)
{
    ui->setupUi(this);
    memset(&m_stVideoCFG, 0x00,sizeof(IPAC_DEV_VIDEOCFG));
    m_lUserID = static_cast<long>(IPAC_DEV_Login(IPADDR, PORT, "admin","admin", TRUE));

    short len;
    bool bRet = IPAC_DEV_GetConfig(m_lUserID, IPAC_DEV_GET_VIDEOCFG, 1,
                       &m_stVideoCFG, sizeof(IPAC_DEV_VIDEOCFG),
                       (LPDWORD)&len);
    if(bRet)
    {
        ui->sliderContrast->setValue(m_stVideoCFG.nContrast);
        ui->labelContrast->setText(QString::number(m_stVideoCFG.nContrast));
        ui->sliderBrightness->setValue(m_stVideoCFG.nBright);
        ui->labelBrightness->setText(QString::number(m_stVideoCFG.nBright));
        ui->sliderSaturate->setValue(m_stVideoCFG.nSaturat);
        ui->labelSaturate->setText(QString::number(m_stVideoCFG.nSaturat));
    }

    m_path = QDir::toNativeSeparators(QStandardPaths::writableLocation(QStandardPaths::DesktopLocation));
    ui->labelPath->setText(m_path);
    ui->sliderGain->setValue(5);
    ui->sliderGain->setRange(1, 10);
    ui->sliderGain->setTickInterval(1);
    ui->sliderContrast->setRange(0, 256);
    ui->sliderContrast->setTickInterval(10);
    ui->sliderBrightness->setRange(0, 256);
    ui->sliderBrightness->setTickInterval(10);
    ui->sliderSaturate->setRange(0, 256);
    ui->sliderSaturate->setTickInterval(10);
    ui->cbDispMode->setCurrentIndex(2);
    ui->editCycle->setValidator(new QIntValidator(28, 5700, this));

    connect(ui->sliderGain, SIGNAL(valueChanged(int)), this, SLOT(gainChanged(int)));
    connect(ui->cbDispMode, SIGNAL(currentIndexChanged(int)), this, SLOT(modeChanged(int)));
    connect(ui->sliderContrast, SIGNAL(valueChanged(int)), this, SLOT(contrastChanged(int)));
    connect(ui->sliderBrightness, SIGNAL(valueChanged(int)), this, SLOT(brightnessChanged(int)));
    connect(ui->sliderSaturate, SIGNAL(valueChanged(int)), this, SLOT(saturateChanged(int)));
    connect(ui->btnPhoto, SIGNAL(clicked()), this, SLOT(saveImage()));
    connect(ui->btnVideo, SIGNAL(clicked()), this, SLOT(recordVideo()));
    connect(ui->btnSetIntegral, SIGNAL(clicked()), this, SLOT(onSetIntegral()));
    connect(ui->btnSetCycle, SIGNAL(clicked()), this, SLOT(onSetCycle()));
    connect(ui->cbHistogram, SIGNAL(stateChanged(int)), this, SLOT(histogramCheckBoxChanged(int)));
    connect(ui->cbSmooth, SIGNAL(stateChanged(int)), this, SLOT(smoothCheckBoxChanged(int)));
    connect(ui->cbHighgain, SIGNAL(stateChanged(int)), this, SLOT(highgainCheckBoxChanged(int)));
    connect(ui->cbIntegral, SIGNAL(stateChanged(int)), this, SLOT(integralCheckBoxChanged(int)));
    connect(ui->cbNonuniform, SIGNAL(stateChanged(int)), this, SLOT(nonuniformCheckBoxChanged(int)));
    connect(ui->cbAWB, SIGNAL(stateChanged(int)), this, SLOT(whitebalanceCheckBoxChanged(int)));
    connect(ui->cbMirror, SIGNAL(stateChanged(int)), this, SLOT(mirrorCheckBoxChanged(int)));
    connect(ui->cbKeepRatio, SIGNAL(stateChanged(int)), this, SLOT(keepratioCheckBoxChanged(int)));
    connect(ui->btnBrowse, SIGNAL(clicked()), this, SLOT(onBrowse()));
}

ControlPanel::~ControlPanel()
{
    delete ui;
    if (m_lUserID)
        IPAC_DEV_Logout(static_cast<ULONG>(m_lUserID));
}

void ControlPanel::onBrowse()
{
    QFileDialog *fileDialog = new QFileDialog(this);
    fileDialog->setDirectory(m_path);
    fileDialog->setFileMode(QFileDialog::DirectoryOnly);
    if(fileDialog->exec() == QDialog::Accepted)
    {
        m_path = QDir::toNativeSeparators(fileDialog->selectedFiles()[0]);
        ui->labelPath->setText(m_path);
        emit updatePath(m_path);
    }
}

void ControlPanel::recordVideo()
{
    static bool bRecording = false;
    bRecording = !bRecording;
    if(bRecording)
    {
        ui->btnVideo->setText(QStringLiteral("Stop Recording"));
        emit startRecording();
    }
    else
    {
        ui->btnVideo->setText(QStringLiteral("Start Recording"));
        emit stopRecording();
    }
}

void ControlPanel::modeChanged(int nMode)
{
    emit updateDispMode(nMode);
}

void ControlPanel::saveImage()
{
    emit sendImages();
}

void ControlPanel::gainChanged(int nGain)
{
    ui->labelGain->setText(QString::number(nGain));

    emit updateGain(nGain);
}

void ControlPanel::contrastChanged(int nContrast)
{
    ui->labelContrast->setText(QString::number(nContrast));
    m_stVideoCFG.nContrast = static_cast<uchar>(nContrast);
    IPAC_DEV_SetConfig(m_lUserID,IPAC_DEV_SET_VIDEOCFG,1,&m_stVideoCFG,sizeof(m_stVideoCFG));
}

void ControlPanel::brightnessChanged(int nBrightness)
{
    ui->labelBrightness->setText(QString::number(nBrightness));
    m_stVideoCFG.nBright = static_cast<uchar>(nBrightness);
    IPAC_DEV_SetConfig(m_lUserID,IPAC_DEV_SET_VIDEOCFG,1,&m_stVideoCFG,sizeof(m_stVideoCFG));
}

void ControlPanel::saturateChanged(int nSaturate)
{
    ui->labelSaturate->setText(QString::number(nSaturate));
    m_stVideoCFG.nSaturat = static_cast<uchar>(nSaturate);
    IPAC_DEV_SetConfig(m_lUserID,IPAC_DEV_SET_VIDEOCFG,1,&m_stVideoCFG,sizeof(m_stVideoCFG));
}

void ControlPanel::dispMessage(QString message)
{
    ui->labelMessage->setText(message);
}

void ControlPanel::setMode(uint32_t mode)
{
    UNION_REG value;
    value.data32 = mode;
    if((value.data8[0] & 0x04) == 0x04)
        ui->cbHighgain->setChecked(true);
    else {
        ui->cbHighgain->setChecked(false);
    }

    if((value.data8[0] & 0x10) == 0x10)
        ui->cbNonuniform->setChecked(true);
    else {
        ui->cbNonuniform->setChecked(false);
    }

    if((value.data8[1] & 0x02) == 0x02)
        ui->cbIntegral->setChecked(true);
    else {
        ui->cbIntegral->setChecked(false);
    }
}

void ControlPanel::setIntegral(double integral)
{
    ui->editIntegral->setText(QString::number(integral, 10, 3));
}

void ControlPanel::setFrameCycle(double cycle)
{
    ui->editCycle->setText(QString::number(cycle, 10, 3));
    int maxv = int(cycle - 28);
    ui->editIntegral->setValidator(new QIntValidator(1, maxv, this));
}

void ControlPanel::onSetIntegral()
{
    double value = ui->editIntegral->text().toDouble();

    emit updateIntegral(value);
}

void ControlPanel::onSetCycle()
{
    double value = ui->editCycle->text().toDouble();
    emit updateCycle(value);
}

void ControlPanel::histogramCheckBoxChanged(int state)
{
    emit enableHistogram(state == Qt::Checked);
}

void ControlPanel::integralCheckBoxChanged(int state)
{
    emit enableIntegral(state == Qt::Checked);
}

void ControlPanel::highgainCheckBoxChanged(int state)
{
    emit enableHighgain(state == Qt::Checked);
}

void ControlPanel::nonuniformCheckBoxChanged(int state)
{
    emit enableNonuniform(state == Qt::Checked);
}

void ControlPanel::whitebalanceCheckBoxChanged(int state)
{
    if(state == Qt::Checked)
    {
        m_stVideoCFG.bOpenWB = 1;
    }
    else
    {
        m_stVideoCFG.bOpenWB = 0;
    }
    IPAC_DEV_SetConfig(m_lUserID,IPAC_DEV_SET_VIDEOCFG,1,&m_stVideoCFG,sizeof(m_stVideoCFG));

    emit enableWhiteBalance(state == Qt::Checked);
}

void ControlPanel::smoothCheckBoxChanged(int state)
{
    emit enableSmooth(state == Qt::Checked);
}

void ControlPanel::mirrorCheckBoxChanged(int state)
{
    emit enableMirror(state == Qt::Checked);
}

void ControlPanel::keepratioCheckBoxChanged(int state)
{
    emit enableKeepRatio(state == Qt::Checked);
}

