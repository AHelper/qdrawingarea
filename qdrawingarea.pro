TEMPLATE = subdirs

qdrawingarea.subdir = qdrawingarea

testui.subdir = testui
testui.depends = qdrawingarea

SUBDIRS += qdrawingarea testui
