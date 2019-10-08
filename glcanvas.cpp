#include "glcanvas.h"
#include <QOpenGLShaderProgram>
#include <QOpenGLTexture>
#include <QMouseEvent>
#include <QMenu>
#include <QTimer>

GLCanvas::GLCanvas(QImage imgVisible, QImage imgSwir, QWidget *parent)
    : QOpenGLWidget(parent)
      , m_clearColor(Qt::black)
      , m_imageVisible(imgVisible)
      , m_imageSwir(imgSwir)
      , m_nMode(2)
      , m_textureSwir(nullptr)
      , m_textureVisible(nullptr)
      , m_program(nullptr)
      , m_bMirror(false)
      , m_bKeepRatio(false)
{
    QTimer *timer = new QTimer(this);
    connect(timer, SIGNAL(timeout()), this, SLOT(onTimer()));
    timer->start(30);
}

GLCanvas::~GLCanvas()
{
    makeCurrent();
    if(m_textureSwir)
        delete m_textureSwir;
    if(m_textureVisible)
        delete m_textureVisible;
    m_vbo.destroy();
    delete m_program;
    doneCurrent();
}

void GLCanvas::onTimer()
{
    update();
}

QSize GLCanvas::minimumSizeHint() const
{
    return QSize(50, 50);
}

QSize GLCanvas::sizeHint() const
{
    return QSize(200, 200);
}

void GLCanvas::setClearColor(const QColor &color)
{
    m_clearColor = color;
}

void GLCanvas::nextMode()
{
    m_mutex.lock();
    if(++m_nMode >= 4)
        m_nMode = 0;
    m_mutex.unlock();
}

void GLCanvas::changeMode(int nMode)
{
    m_mutex.lock();
    m_nMode = nMode;
    m_mutex.unlock();
}

void GLCanvas::initializeGL()
{
    initializeOpenGLFunctions();

    makeObject();
    m_textureSwir = new QOpenGLTexture(m_imageSwir);
    m_textureVisible = new QOpenGLTexture(m_imageVisible);

    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
    glEnable(GL_BLEND);

#define PROGRAM_VERTEX_ATTRIBUTE 0
#define PROGRAM_TEXCOORD_ATTRIBUTE 1

    QOpenGLShader *vshader = new QOpenGLShader(QOpenGLShader::Vertex, this);
    const char *vsrc =
        "attribute highp vec4 vertex;\n"
        "attribute mediump vec4 texCoord;\n"
        "varying mediump vec4 texc;\n"
        "uniform mediump mat4 matrix;\n"
        "void main(void)\n"
        "{\n"
        "    gl_Position = matrix * vertex;\n"
        "    texc = texCoord;\n"
        "}\n";
    vshader->compileSourceCode(vsrc);

    QOpenGLShader *fshader = new QOpenGLShader(QOpenGLShader::Fragment, this);
    const char *fsrc =
        "uniform sampler2D texture;\n"
        "varying mediump vec4 texc;\n"
        "void main(void)\n"
        "{\n"
        "    gl_FragColor = texture2D(texture, texc.st);\n"
        "}\n";
    fshader->compileSourceCode(fsrc);

    m_program = new QOpenGLShaderProgram;
    m_program->addShader(vshader);
    m_program->addShader(fshader);
    m_program->bindAttributeLocation("vertex", PROGRAM_VERTEX_ATTRIBUTE);
    m_program->bindAttributeLocation("texCoord", PROGRAM_TEXCOORD_ATTRIBUTE);
    m_program->link();

    m_program->bind();
    m_program->setUniformValue("texture", 0);
}

void GLCanvas::paintGL()
{
    glClearColor(m_clearColor.redF(), m_clearColor.greenF(),
                 m_clearColor.blueF(), m_clearColor.alphaF());
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    QMatrix4x4 m;
    m.ortho(-0.5f, +0.5f, +0.5f, -0.5f, 2.0f, 10.0f);
    m.translate(0.0f, 0.0f, -5.0f);

    m_program->setUniformValue("matrix", m);
    m_program->enableAttributeArray(PROGRAM_VERTEX_ATTRIBUTE);
    m_program->enableAttributeArray(PROGRAM_TEXCOORD_ATTRIBUTE);
    m_program->setAttributeBuffer(PROGRAM_VERTEX_ATTRIBUTE, GL_FLOAT, 0, 3, 5 * sizeof(GLfloat));
    m_program->setAttributeBuffer(PROGRAM_TEXCOORD_ATTRIBUTE, GL_FLOAT, 3 * sizeof(GLfloat), 2, 5 * sizeof(GLfloat));

    m_mutex.lock();
    int swiridx = 16 + m_nMode;
    int visibleidx = m_nMode;

    if(m_bMirror)
    {
        swiridx += 8;
        visibleidx += 8;
    }
    if(m_bKeepRatio)
    {
        swiridx += 4;
        visibleidx += 4;
    }

    m_textureSwir->bind();
    glDrawArrays(GL_TRIANGLE_FAN, swiridx * 4, 4);
    m_textureVisible->bind();
    glDrawArrays(GL_TRIANGLE_FAN, visibleidx * 4, 4);

    m_mutex.unlock();
}
void GLCanvas::resizeGL(int width, int height)
{
    int side = qMin(width, height);
    glViewport((width - side) / 2, (height - side) / 2, side, side);
//    QString msg = tr("(") + QString::number(width) + tr(",") + QString::number(height) + tr(")");
//    emit sendMessage(msg);
}

void GLCanvas::enableMirror(bool bEnable)
{
    m_bMirror = bEnable;
}

void GLCanvas::enableKeepRatio(bool bEnable)
{
    m_bKeepRatio = bEnable;
}

void GLCanvas::makeObject()
{
    static const float coords[32][4][5] = {
        // visible base = 0
        {{ -0.6f, +0.6f, 0, 1, 0 }, { -1, +0.6f, 0, 0, 0 }, { -1, +1, 0, 0, 1 }, { -0.6f, +1, 0, 1, 1 }},
        {{ +1, -1, -1, 1, 0 }, { -1, -1, -1, 0, 0 }, { -1, +1, -1, 0, 1 }, { +1, +1, -1, 1, 1 }},
        {{ +0.2f, -0.2f, 0, 1, 0 }, { -1, -0.2f, 0, 0, 0 }, { -1, +1, 0, 0, 1 }, { +0.2f, +1, 0, 1, 1 }},
        {{ +1, -1, -1, 1, 0 }, { -0.2f, -1, -1, 0, 0 }, { -0.2f, +0.2f, -1, 0, 1 }, { +1, +0.2f, -1, 1, 1}},
        // keep aspe ratio + 4
        {{ -0.28f, +0.52f, 0, 1, 0 }, { -1, +0.52f, 0, 0, 0 }, { -1, +1, 0, 0, 1 }, { -0.28f, +1, 0, 1, 1 }},
        {{ +1, -1, -1, 1, 0 }, { -1, -1, -1, 0, 0 }, { -1, +0.7325f, -1, 0, 1 }, { +1, +0.7325f, -1, 1, 1 }},
        {{ +0.2f, -0.0395f, 0, 1, 0 }, { -1, -0.0395f, 0, 0, 0 }, { -1, +1, 0, 0, 1 }, { +0.2f, +1, 0, 1, 1 }},
        {{ +1, -1, -1, 1, 0 }, { -0.2f, -1, -1, 0, 0 }, { -0.2f, +0.0395f, -1, 0, 1 }, { +1, +0.0395f, -1, 1, 1 }},
        //  + 8
        {{ -0.6f, +0.6f, 0, 0, 0 }, { -1, +0.6f, 0, 1, 0 }, { -1, +1, 0, 1, 1 }, { -0.6f, +1, 0, 0, 1 }},
        {{ +1, -1, -1, 0, 0 }, { -1, -1, -1, 1, 0 }, { -1, +1, -1, 1, 1 }, { +1, +1, -1, 0, 1 }},
        {{ +0.2f, -0.2f, 0, 0, 0 }, { -1, -0.2f, 0, 1, 0 }, { -1, +1, 0, 1, 1 }, { +0.2f, +1, 0, 0, 1 }},
        {{ +1, -1, -1, 0, 0 }, { -0.2f, -1, -1, 1, 0 }, { -0.2f, +0.2f, -1, 1, 1 }, { +1, +0.2f, -1, 0, 1}},

        {{ -0.28f, +0.52f, 0, 0, 0 }, { -1, +0.52f, 0, 1, 0 }, { -1, +1, 0, 1, 1 }, { -0.28f, +1, 0, 0, 1 }},
        {{ +1, -1, -1, 0, 0 }, { -1, -1, -1, 1, 0 }, { -1, +0.7325f, -1, 1, 1 }, { +1, +0.7325f, -1, 0, 1 }},
        {{ +0.2f, -0.0395f, 0, 0, 0 }, { -1, -0.0395f, 0, 1, 0 }, { -1, +1, 0, 1, 1 }, { +0.2f, +1, 0, 0, 1 }},
        {{ +1, -1, -1, 0, 0 }, { -0.2f, -1, -1, 1, 0 }, { -0.2f, +0.0395f, -1, 1, 1 }, { +1, +0.0395f, -1, 0, 1 }},
        // swir base = 16
        {{ +1, -1, -1, 1, 0 }, { -1, -1, -1, 0, 0 }, { -1, +1, -1, 0, 1 }, { +1, +1, -1, 1, 1 }},
        {{ -0.6f, +0.6f, 0, 1, 0 }, { -1, +0.6f, 0, 0, 0 }, { -1, +1, 0, 0, 1 }, { -0.6f, +1, 0, 1, 1 }},
        {{ +1, -1, -1, 1, 0 }, { -0.2f, -1, -1, 0, 0 }, { -0.2f, +0.2f, -1, 0, 1 }, { +1, +0.2f, -1, 1, 1}},
        {{ +0.2f, -0.2f, 0, 1, 0 }, { -1, -0.2f, 0, 0, 0 }, { -1, +1, 0, 0, 1 }, { +0.2f, +1, 0, 1, 1 }},

        {{ +1, -1, -1, 1, 0 }, { -0.63f, -1, -1, 0, 0 }, { -0.63f, +1, -1, 0, 1 }, { +1, +1, -1, 1, 1 }},
        {{ -0.28f, +0.113f, 0, 1, 0 }, { -1, +0.113f, 0, 0, 0 }, { -1, +1, 0, 0, 1 }, { -0.28f, +1, 0, 1, 1 }},
        {{ +1, -1, -1, 1, 0 }, { -0.2f, -1, -1, 0, 0 }, { -0.2f, +0.4784f, -1, 0, 1 }, { +1, +0.4784f, -1, 1, 1 }},
        {{ +0.2f, -0.4784f, 0, 1, 0 }, { -1, -0.4784f, 0, 0, 0 }, { -1, +1, 0, 0, 1 }, { +0.2f, +1, 0, 1, 1 }},

        {{ +1, -1, -1, 0, 0 }, { -1, -1, -1, 1, 0 }, { -1, +1, -1, 1, 1 }, { +1, +1, -1, 0, 1 }},
        {{ -0.6f, +0.6f, 0, 0, 0 }, { -1, +0.6f, 0, 1, 0 }, { -1, +1, 0, 1, 1 }, { -0.6f, +1, 0, 0, 1 }},
        {{ +1, -1, -1, 0, 0 }, { -0.2f, -1, -1, 1, 0 }, { -0.2f, +0.2f, -1, 1, 1 }, { +1, +0.2f, -1, 0, 1}},
        {{ +0.2f, -0.2f, 0, 0, 0 }, { -1, -0.2f, 0, 1, 0 }, { -1, +1, 0, 1, 1 }, { +0.2f, +1, 0, 0, 1 }},

        {{ +1, -1, -1, 0, 0 }, { -0.63f, -1, -1, 1, 0 }, { -0.63f, +1, -1, 1, 1 }, { +1, +1, -1, 0, 1 }},
        {{ -0.28f, +0.113f, 0, 0, 0 }, { -1, +0.113f, 0, 1, 0 }, { -1, +1, 0, 1, 1 }, { -0.28f, +1, 0, 0, 1 }},
        {{ +1, -1, -1, 0, 0 }, { -0.2f, -1, -1, 1, 0 }, { -0.2f, +0.4784f, -1, 1, 1 }, { +1, +0.4784f, -1, 0, 1 }},
        {{ +0.2f, -0.4784f, 0, 0, 0 }, { -1, -0.4784f, 0, 1, 0 }, { -1, +1, 0, 1, 1 }, { +0.2f, +1, 0, 0, 1 }},
    };
    QVector<GLfloat> vertData;
    for(int i = 0; i < 32; i++)
    {
        for (int j = 0; j < 4; ++j)
        {
            // vertex position
            vertData.append(0.5f * coords[i][j][0]);
            vertData.append(0.5f * coords[i][j][1]);
            vertData.append(0.5f * coords[i][j][2]);

            vertData.append(coords[i][j][3]);
            vertData.append(coords[i][j][4]);
        }
    }

    m_vbo.create();
    m_vbo.bind();
    m_vbo.allocate(vertData.constData(), vertData.count() * sizeof(GLfloat));
}

void GLCanvas::resizeCanvas(int width, int height)
{
    resizeGL(width, height);
}


void GLCanvas::updateSwir(QImage image)
{
    m_mutex.lock();
    m_imageSwir = image;
    if(m_textureSwir)
        delete m_textureSwir;
    m_textureSwir = new QOpenGLTexture(image);
    m_mutex.unlock();
}

void GLCanvas::updateVisible(QImage image)
{
    m_mutex.lock();
    m_imageVisible = image;
    if(m_textureVisible)
        delete m_textureVisible;
    m_textureVisible = new QOpenGLTexture(image);
    m_mutex.unlock();
}

void GLCanvas::exportImages()
{
    m_mutex.lock();
    emit getImages(m_imageVisible, m_imageSwir);
    m_mutex.unlock();
}
