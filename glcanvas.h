#ifndef GLCANVAS_H
#define GLCANVAS_H

#include <QOpenGLWidget>
#include <QOpenGLFunctions>
#include <QOpenGLBuffer>
#include <QKeyEvent>
#include <QMutex>

QT_FORWARD_DECLARE_CLASS(QOpenGLShaderProgram);
QT_FORWARD_DECLARE_CLASS(QOpenGLTexture)

class GLCanvas : public QOpenGLWidget, protected QOpenGLFunctions
{
    Q_OBJECT

public:
    explicit GLCanvas(QImage imgVisible, QImage imgSwir, QWidget *parent = nullptr);
    ~GLCanvas() override;

    QSize minimumSizeHint() const override;
    QSize sizeHint() const override;
    void setClearColor(const QColor &color);
    inline QImage getVisibleImage() { return m_imageVisible;}
    inline QImage getSwirImage() { return m_imageSwir;}
    void resizeCanvas(int width, int height);
signals:
    void getImages(QImage image1, QImage image2);
    void sendMessage(QString);

protected:
    void initializeGL() override;
    void paintGL() override;
    void resizeGL(int width, int height) override;

private:
    void makeObject();

    QColor m_clearColor;
    QImage m_imageVisible;
    QImage m_imageSwir;
    int m_nMode;
    QMutex m_mutex;
    QOpenGLTexture *m_textureSwir;
    QOpenGLTexture *m_textureVisible;
    QOpenGLShaderProgram *m_program;
    QOpenGLBuffer m_vbo;
    bool m_bMirror;
    bool m_bKeepRatio;

private slots:
    void    onTimer();
    void    nextMode();

public slots:
    void    changeMode(int);
    void    updateSwir(QImage);
    void    updateVisible(QImage);
    void    exportImages();
    void    enableMirror(bool);
    void    enableKeepRatio(bool);
};

#endif
