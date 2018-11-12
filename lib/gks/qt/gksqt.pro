GRDIR           = /usr/local/gr

QT		+= widgets network
DEFINES	        += GRDIR=\\\"$(GRDIR)\\\"
INCLUDEPATH	+= ../
HEADERS		= gkswidget.h gksserver.h
SOURCES		= gksqt.cxx gkswidget.cxx gksserver.cxx
LIBS		+= ../libGKS.a
mac:ICON        = gksqt.icns
win32:RC_ICONS  = gksqt.ico
win32:QMAKE_CXXFLAGS	+= -D_CRT_SECURE_NO_WARNINGS -D_ALLOW_MSC_VER_MISMATCH
RESOURCES       = gksqt.qrc
