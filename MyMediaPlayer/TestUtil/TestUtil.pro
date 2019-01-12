TEMPLATE = app
CONFIG += console c++11
CONFIG -= app_bundle
CONFIG -= qt

INCLUDEPATH += \
    $$PWD/../FFMediaUtilityLib \
    $$PWD/../CommonUtilites

LIBS += \
    -L$$PWD/../../build/debug \
    -lFFMediaUtilityLib \
    -lCommonUtilites

win32 {
    DESTDIR = $$PWD/../../build/debug
}

LIBS += -L$$PWD/../lib/ffmpeg/lib -lavcodec -lavformat -lavutil -lswscale -lavfilter -lswresample

SOURCES += main.cpp

HEADERS +=
