QT		+= widgets network
QMAKE_CXXFLAGS	+= -DGRDIR=\\\"/usr/local/gr\\\"
INCLUDEPATH	+= ../
HEADERS		= gkswidget.h gksserver.h
SOURCES		= gksterm.cxx gkswidget.cxx gksserver.cxx \
		  font.cxx afm.cxx util.cxx dl.cxx malloc.cxx error.cxx io.cxx
mac:ICON        = gksterm.icns
