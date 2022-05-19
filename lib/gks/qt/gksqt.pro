GRDIR                 = /usr/local/gr
QT                   += widgets network
DEFINES              += GRDIR=\\\"$(GRDIR)\\\"
INCLUDEPATH          += ../
QMAKE_CXXLAGS        += $$(EXTRA_CXXFLAGS)
QMAKE_LFLAGS         += $$(EXTRA_LDFLAGS)
HEADERS               = gkswidget.h gksserver.h
SOURCES               = gksqt.cxx gkswidget.cxx gksserver.cxx
exists( ../libGKS.a ) {
  LIBS               += ../libGKS.a
} else {
  LIBS               += $$GRDIR/lib/libGKS.a
}
LIBS += -ldl
mac:ICON              = gksqt.icns
win32:RC_ICONS        = gksqt.ico
win32:QMAKE_CXXFLAGS += -D_CRT_SECURE_NO_WARNINGS -D_ALLOW_MSC_VER_MISMATCH
RESOURCES             = gksqt.qrc
