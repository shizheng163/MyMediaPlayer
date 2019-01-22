QT    -= core gui
TARGET = FFMediaUtilityLib
TEMPLATE = lib
CONFIG += c++11

INCLUDEPATH += \
    $$PWD/../CommonUtilites \
    $$PWD/../include/ffmpeg/include

LIBS += \
    -L$$PWD/../lib/ffmpeg/lib -lavcodec -lavformat -lavutil -lswscale -lavfilter -lswresample \
    -L$$PWD/../../build/debug/ -lCommonUtilites

unix {
    target.path = /usr/lib
    INSTALLS += target
}

win32 {
    DESTDIR = $$PWD/../../build/debug
}
SOURCES += \
    ffdecoder.cpp \
    ffmpegutil.cpp \
    datadelaytask.cpp

HEADERS += \
    ffdecoder.h \
    ffmpegutil.h \
    datadelaytask.h
