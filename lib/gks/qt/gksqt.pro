contains(QT_VERSION, ^5\\..*) {
QT += widgets printsupport gui network
}
INCLUDEPATH	+= ../
HEADERS		= gkswidget.h gksserver.h
SOURCES		= gksqt.cpp gkswidget.cpp gksserver.cpp \
		  font.cpp afm.cpp util.cpp malloc.cpp error.cpp io.cpp
RESOURCES      += gksqt.qrc

mac:ICON        = ./images/gksqt.icns

QT		+= network
DEFINES		= GRDIR=\\\"/usr/local/gr\\\"

