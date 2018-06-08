#-------------------------------------------------
#
# Project created by QtCreator 2018-01-31T19:44:05
#
#-------------------------------------------------

QT       += core gui network serialbus serialport charts

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = StationAcq
TEMPLATE = app

# The following define makes your compiler emit warnings if you use
# any feature of Qt which as been marked as deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if you use deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0


SOURCES += \
        main.cpp \
        stationacq.cpp \
    chartview.cpp \
    configdialog.cpp \
    logdialog.cpp \
    uploaddialog.cpp \
    sleepThread.cpp \
    configmanagement.cpp \
    def.cpp \
    pumpcontroldlg.cpp \
    serialthread.cpp \
    assignment.cpp \
    content.cpp \
    DataContent.cpp \
    PartContent.cpp \
    StringDataReader.cpp \
    logindialog.cpp

HEADERS += \
        stationacq.h \
    chartview.h \
    configdialog.h \
    logdialog.h \
    uploaddialog.h \
    sleepThread.h \
    configmanagement.h \
    def.h \
    pumpcontroldlg.h \
    serialthread.h \
    assignment.h \
    cluster.h \
    content.h \
    DataContent.h \
    PartContent.h \
    StringDataReader.h \
    logindialog.h

FORMS += \
        stationacq.ui \
    configdialog.ui \
    logdialog.ui \
    uploaddialog.ui \
    pumpcontroldlg.ui \
    logindialog.ui

INCLUDEPATH += D:\qtaddon\AnalogWidgets\analogwidgets\analogwidgets
INCLUDEPATH += c:\opencv\build\include
LIBS += libanalogwidgets

RESOURCES += pixmaps/pixmaps.qrc
RC_FILE = maininterface.rc

CONFIG(debug, debug|release): {
LIBS += -LC:/opencv/build/x86/vc14/lib \
-lopencv_core2413d \
-lopencv_imgproc2413d \
-lopencv_highgui2413d \
-lopencv_ml2413d \
-lopencv_video2413d \
-lopencv_features2d2413d \
-lopencv_calib3d2413d \
-lopencv_objdetect2413d \
-lopencv_contrib2413d \
-lopencv_legacy2413d \
-lopencv_flann2413d
} else:CONFIG(release, debug|release): {
LIBS += -LC:/opencv/build/x86/vc14/lib \
-lopencv_core2413 \
-lopencv_imgproc2413 \
-lopencv_highgui2413 \
-lopencv_ml2413 \
-lopencv_video2413 \
-lopencv_features2d2413 \
-lopencv_calib3d2413 \
-lopencv_objdetect2413 \
-lopencv_contrib2413 \
-lopencv_legacy2413 \
-lopencv_flann2413
}
