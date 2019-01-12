QT       += core gui
QT       += opengl

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = MyMediaPlayer
TEMPLATE = app

DEFINES += QT_DEPRECATED_WARNINGS


SOURCES += \
    main.cpp\
    mainwindow.cpp \
    videoglwidget.cpp

HEADERS  += \
    mainwindow.h \
    videoglwidget.h

FORMS    += mainwindow.ui

INCLUDEPATH += \
    $$PWD/../CommonUtilites

LIBS += \
    -L$$PWD/../../build/debug/ -lCommonUtilites

win32 {
DESTDIR = $$PWD/../../build/debug/
}
