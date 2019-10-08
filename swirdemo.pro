#-------------------------------------------------
#
# Project created by QtCreator 2019-03-22T21:29:42
#
#-------------------------------------------------

QT       += core gui network opengl

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = swirdemo
TEMPLATE = app

# The following define makes your compiler emit warnings if you use
# any feature of Qt which has been marked as deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if you use deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

CONFIG += c++11

INCLUDEPATH += $$_PRO_FILE_PWD_/include \
    $$QtDir/include \

LIBS += -L$$_PRO_FILE_PWD_/lib \
    -L$$QtDir/lib \
    -lIPACDevNetSDK \
    -lavcodec\
    -lavutil\
    -lavformat\
    -lswscale\
    -lswresample \

CONFIG(debug, debug|release) {
    LIBS += -lopencv_world341d
} else {
    LIBS += -lopencv_world341
}


SOURCES += \
        main.cpp \
        demodialog.cpp \
    glcanvas.cpp \
    swirprocessor.cpp \
    utils.cpp \
    swircapturer.cpp \
    rtspcapturer.cpp \
    videoencoder.cpp \
    controlpanel.cpp

HEADERS += \
        demodialog.h \
    glcanvas.h \
    global.h \
    rtspcapturer.h \
    swircapturer.h \
    swirprocessor.h \
    videoencoder.h \
    controlpanel.h

FORMS += \
    controlpanel.ui

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

RESOURCES += \
    demo.qrc
