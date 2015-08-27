#-------------------------------------------------
#
# Project created by QtCreator 2015-08-25T19:43:47
#
#-------------------------------------------------

QT       += core gui widgets

TARGET = landmarker
TEMPLATE = app

SOURCES += main.cpp\
    mainwindow.cpp \
    common.cpp \
    sequence.cpp \
    annotation.cpp \
    generator.cpp

HEADERS  += \
    mainwindow.h \
    common.h \
    sequence.h \
    annotation.h \
    generator.h

FORMS    += mainwindow.ui

OPENCV = D:/opencv3/build
CONFIG(debug,debug|release): LIBS += -L$$OPENCV/x64/vc12/lib/ -lopencv_world300d
CONFIG(release,debug|release): LIBS += -L$$OPENCV/x64/vc12/lib/ -lopencv_world300
INCLUDEPATH += $$OPENCV/include
DEPENDPATH += $$OPENCV/include

SDK = D:/sensetime
LIBS += -L$$SDK/windows/x64/ -lfacesdk
INCLUDEPATH += $$SDK/include
DEPENDPATH += $$SDK/include
