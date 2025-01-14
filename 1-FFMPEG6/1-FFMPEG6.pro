TEMPLATE = app
CONFIG += console c++17
CONFIG -= app_bundle
CONFIG -= qt

SOURCES += \
        demuxthread.cpp \
        log.cpp \
        main.cpp
        win32 {
        INCLUDEPATH += $$PWD/ffmpeg-4.2.1-win32-dev/include
        LIBS += $$PWD/ffmpeg-4.2.1-win32-dev/lib/avformat.lib   \
                $$PWD/ffmpeg-4.2.1-win32-dev/lib/avcodec.lib    \
                $$PWD/ffmpeg-4.2.1-win32-dev/lib/avdevice.lib   \
                $$PWD/ffmpeg-4.2.1-win32-dev/lib/avfilter.lib   \
                $$PWD/ffmpeg-4.2.1-win32-dev/lib/avutil.lib     \
                $$PWD/ffmpeg-4.2.1-win32-dev/lib/postproc.lib   \
                $$PWD/ffmpeg-4.2.1-win32-dev/lib/swresample.lib \
                $$PWD/ffmpeg-4.2.1-win32-dev/lib/swscale.lib
        }
HEADERS += \
    demuxthread.h \
    log.h \
    thread.h
