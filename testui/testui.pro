#-------------------------------------------------
#
# Project created by QtCreator 2015-05-25T17:26:50
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = DrawingPadTest
TEMPLATE = app


SOURCES += main.cpp \
        testui.cpp

HEADERS  += testui.h

FORMS    += testui.ui

INCLUDEPATH += ../qdrawingarea

LIBS += -Wl,-rpath=$$OUT_PWD/../qdrawingarea -L../qdrawingarea -lqdrawingarea

DISTFILES += \
    drawingarea/NOTES.txt
