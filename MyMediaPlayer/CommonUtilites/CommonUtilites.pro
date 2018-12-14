QT    -= core gui
TARGET = CommonUtilites
TEMPLATE = lib
CONFIG += c++11

unix {
    target.path = /usr/lib
    INSTALLS += target
}

win32 {
DESTDIR = $$PWD/../../debug/
}

SOURCES += \
    logutil.cpp \
    fileutil.cpp

HEADERS += \
    logutil.h \
    fileutil.h

