QT       += core gui
QT       += opengl

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets
RC_ICONS = ../../images/VideoIcon.ico
TARGET = MyMediaPlayer
TEMPLATE = app

DEFINES += QT_DEPRECATED_WARNINGS


win32 {
    DESTDIR = $$PWD/../../build/debug
}

FORMS    += mainwindow.ui

INCLUDEPATH += \
    $$PWD/../CommonUtilites \
    $$PWD/../FFMediaUtilityLib

LIBS += \
    -L$$PWD/../../build/debug/ -lCommonUtilites -lFFMediaUtilityLib

LIBS += -L$$PWD/../lib/ffmpeg/lib -lavcodec -lavformat -lavutil -lswscale -lavfilter -lswresample

SOURCES += \
    main.cpp\
    mainwindow.cpp \
    videoglwidget.cpp

HEADERS  += \
    mainwindow.h \
    videoglwidget.h
