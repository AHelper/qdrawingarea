#-------------------------------------------------
#
# Project created by QtCreator 2015-05-25T17:26:50
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = DrawingPadTest
TEMPLATE = app
CONFIG  += debug_and_release_target debug_and_release

SOURCES += main.cpp \
        testui.cpp

HEADERS  += testui.h

FORMS    += testui.ui

INCLUDEPATH += ../qdrawingarea

CONFIG(debug, debug|release) {
  LIBS += -Wl,-rpath=$$OUT_PWD/../qdrawingarea/debug/ -L../qdrawingarea/debug -lqdrawingarea
} else {
  LIBS += -Wl,-rpath=$$OUT_PWD/../qdrawingarea/release/ -L../qdrawingarea/release -lqdrawingarea
}

DISTFILES += \
    drawingarea/NOTES.txt
