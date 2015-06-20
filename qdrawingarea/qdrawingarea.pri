QT	+= core gui
greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET	= qdrawingarea
TEMPLATE = lib
CONFIG  += debug_and_release_target debug_and_release

SOURCES += qdrawingarea.cpp

HEADERS += qdrawingarea.h \
	qdrawingarea_p.h
