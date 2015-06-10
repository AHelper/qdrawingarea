QT	+= core gui
greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET	= qdrawingarea
TEMPLATE = lib

SOURCES += qdrawingarea.cpp

HEADERS += qdrawingarea.h \
	qdrawingarea_p.h
