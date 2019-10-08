#include "demodialog.h"
#include <QDateTime>
#include <QLayout>
#include <QStandardPaths>
#include "controlpanel.h"

DemoDialog::DemoDialog(QWidget *parent)
    : QDialog(parent)
    , m_pSwirEncoder(nullptr)
    , m_pVisibleEncoder(nullptr)
    , m_bRecording(false)
    , m_nHeartBeatVisible(0)
    , m_nHeartBeatSwir(0)
{
//    setWindowFlags(Qt::FramelessWindowHint);

    QColor clearColor;
    clearColor.setHsv(255, 255, 63);

    m_imgVisible = QImage(tr(":/icons/visible.png"));
    m_imgSwir = QImage(tr(":/icons/swir.png"));

    m_pCanvas = new GLCanvas(m_imgVisible, m_imgSwir, this);
    m_pCanvas->setClearColor(clearColor);

    QHBoxLayout* pLayout = new QHBoxLayout(this);
    pLayout->setMargin(0);
    pLayout->addWidget(m_pCanvas);

    m_pCamCapturer = new RtspCapturer(tr(RTSP_URL));
    connect(m_pCamCapturer, SIGNAL(getFrame(QImage)), this, SLOT(gotVisible(QImage)));
    m_pCamCapturer->start();

    m_pSwirCapturer = new SwirCapturer();
    m_pSwirProcessor = new SwirProcessor();
    connect(m_pSwirCapturer, SIGNAL(parseFrame(QByteArray)), m_pSwirProcessor, SLOT(parseFrame(QByteArray)));
    connect(m_pSwirProcessor, SIGNAL(getFrame(QImage)), this, SLOT(gotSwir(QImage)));

    m_pSwirProcessor->start();
    m_pSwirCapturer->start();

    ControlPanel* pControl = new ControlPanel(this);
    pControl->setFixedWidth(286);
    pLayout->addWidget(pControl);
    setLayout(pLayout);

    m_path = QStandardPaths::writableLocation(QStandardPaths::DesktopLocation);

    connect(pControl, SIGNAL(updateGain(int)), m_pSwirProcessor, SLOT(updateGain(int)));
    connect(m_pCanvas, SIGNAL(getImages(QImage, QImage)), this, SLOT(saveImages(QImage, QImage)));
    connect(this, SIGNAL(updateVisible(QImage)), m_pCanvas, SLOT(updateVisible(QImage)));
    connect(this, SIGNAL(updateSwir(QImage)), m_pCanvas, SLOT(updateSwir(QImage)));
    connect(this, SIGNAL(updateStatus(QString)), pControl, SLOT(dispMessage(QString)));

    connect(pControl, SIGNAL(updateDispMode(int)), m_pCanvas, SLOT(changeMode(int)));
    connect(pControl, SIGNAL(sendImages()), m_pCanvas, SLOT(exportImages()));
    connect(pControl, SIGNAL(startRecording()), this, SLOT(startRecording()));
    connect(pControl, SIGNAL(stopRecording()), this, SLOT(stopRecording()));

    connect(m_pSwirCapturer, SIGNAL(sendMessage(QString)), pControl, SLOT(dispMessage(QString)));
    connect(m_pCamCapturer, SIGNAL(sendMessage(QString)), pControl, SLOT(dispMessage(QString)));
    connect(m_pCanvas, SIGNAL(sendMessage(QString)), pControl, SLOT(dispMessage(QString)));
    connect(m_pSwirCapturer, SIGNAL(updateMode(uint32_t)), pControl, SLOT(setMode(uint32_t)));
    connect(m_pSwirCapturer, SIGNAL(updateIntegral(double)), pControl, SLOT(setIntegral(double)));
    connect(m_pSwirCapturer, SIGNAL(updateCycle(double)), pControl, SLOT(setFrameCycle(double)));

    connect(pControl, SIGNAL(enableHighgain(bool)), m_pSwirCapturer, SLOT(enableHighGain(bool)));
    connect(pControl, SIGNAL(enableHistogram(bool)), m_pSwirProcessor, SLOT(enableHistogram(bool)));
    connect(pControl, SIGNAL(enableSmooth(bool)), m_pSwirProcessor, SLOT(enableSmooth(bool)));
    connect(pControl, SIGNAL(enableNonuniform(bool)), m_pSwirCapturer, SLOT(enableNonuniformityCorrection(bool)));
    connect(pControl, SIGNAL(enableIntegral(bool)), m_pSwirCapturer, SLOT(enableIntegralAdjustion(bool)));
    connect(pControl, SIGNAL(updatePath(QString)), this, SLOT(updatePath(QString)));
    connect(pControl, SIGNAL(updateIntegral(double)), m_pSwirCapturer, SLOT(setIntegral(double)));
    connect(pControl, SIGNAL(updateCycle(double)), m_pSwirCapturer, SLOT(setCycle(double)));

    connect(pControl, SIGNAL(enableWhiteBalance(bool)), m_pCamCapturer, SLOT(enableWhiteBalance(bool)));
    connect(pControl, SIGNAL(enableMirror(bool)), m_pCanvas, SLOT(enableMirror(bool)));
    connect(pControl, SIGNAL(enableKeepRatio(bool)), m_pCanvas, SLOT(enableKeepRatio(bool)));

    QTimer *t = new QTimer(this);
    connect(t, SIGNAL(timeout()), this, SLOT(onTimer()));
    t->start(1000);
}

DemoDialog::~DemoDialog()
{
    if(m_pSwirCapturer)
        delete m_pSwirCapturer;
    if(m_pCamCapturer)
        delete m_pCamCapturer;
    if(m_pSwirEncoder)
        delete m_pSwirEncoder;
    if(m_pVisibleEncoder)
        delete m_pVisibleEncoder;
}

void DemoDialog::onTimer()
{
    // restart if no response for 15 seconds
    if(m_nHeartBeatVisible++ > 15)
    {
        m_pCamCapturer->start();
        qDebug() << "restart visible camera connection";
    }
    if(m_nHeartBeatSwir++ > 15)
    {
        m_pSwirCapturer->restart();
        qDebug() << "restart swir camera connection";
    }
}

void DemoDialog::updatePath(QString path)
{
    m_path = path;
}

void DemoDialog::startRecording()
{
    if(!m_pSwirEncoder)
        m_pSwirEncoder = new VideoEncoder();
    if(!m_pVisibleEncoder)
        m_pVisibleEncoder = new VideoEncoder();

    QString visiblefile = m_path + tr("/visible_") + QDateTime::currentDateTime().toString("yyyy-MM-dd-hh-mm-ss") + tr(".mp4");
    m_pVisibleEncoder->open(visiblefile.toStdString().c_str());

    QString swirfile = m_path + tr("/swir_") + QDateTime::currentDateTime().toString("yyyy-MM-dd-hh-mm-ss") + tr(".mp4");
    m_pSwirEncoder->open(swirfile.toStdString().c_str());

    emit updateStatus("Recording " + visiblefile + " and " + swirfile);

    m_bRecording = true;
}

void DemoDialog::stopRecording()
{
    m_pSwirEncoder->close();
    m_pVisibleEncoder->close();
    emit updateStatus("Finished recroding video");
    m_bRecording = false;
}

void DemoDialog::saveImages(QImage imgVisible, QImage imgSwir)
{
    QString visiblefile = m_path + tr("/visible_") + QDateTime::currentDateTime().toString("yyyy-MM-dd-hh-mm-ss") + tr(".jpg");
    imgVisible.save(visiblefile, "JPG", 100);

    QString swirfile = m_path + tr("/swir_") + QDateTime::currentDateTime().toString("yyyy-MM-dd-hh-mm-ss") + tr(".jpg");
    imgSwir.save(swirfile, "JPG", 100);

    emit updateStatus(visiblefile + " and " + swirfile + " saved.");
}

void DemoDialog::gotSwir(QImage image)
{
    emit updateSwir(image);
    m_nHeartBeatSwir = 0;
    if(m_bRecording)
        m_pSwirEncoder->addFrame(image.copy());
}

void DemoDialog::gotVisible(QImage image)
{
    emit updateVisible(image);
    m_nHeartBeatVisible = 0;
    if(m_bRecording)
        m_pVisibleEncoder->addFrame(image);
}
