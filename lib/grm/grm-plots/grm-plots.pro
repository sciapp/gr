GRDIR = $$(GRDIR)
isEmpty(GRDIR) {
  GRDIR = /usr/local/gr
}
QT += widgets core
QMAKE_CXXLAGS += $$(EXTRA_CXXFLAGS)
QMAKE_LFLAGS += $$(EXTRA_LDFLAGS)
HEADERS += grmplots_widget.hxx grmplots_mainwindow.hxx util.hxx
SOURCES += grmplots_widget.cxx grmplots.cxx grmplots_mainwindow.cxx
INCLUDEPATH += ../include
if (macx) {
    if (exists(../libGRM.dylib)) {
      LIBS += -L.. -lGRM
    } else {
      LIBS += -L$(GRDIR)/lib -lGRM
    }
    # On macOS, the grm-plots executable is located in `$(GRDIR)/Applications/grm-plots.app/Contents/MacOS`
    # and we need to resolve `libGRM.dylib` in `$(GRDIR)/lib`
    QMAKE_RPATHDIR += ../../../../lib
} else {
    if (exists(../libGRM.so)) {
      LIBS += -L.. -lGRM -Wl,-rpath-link,../../gr -Wl,-rpath-link,../../gr3
    } else {
      LIBS += -L$(GRDIR)/lib -lGRM -Wl,-rpath-link,$(GRDIR)/lib
    }
    # On every other system, the grm-plots executable is located in `$(GRDIR)/bin`
    # and we need to resolve `libGRM.so` in `$(GRDIR)/lib`
    QMAKE_RPATHDIR += ../lib
}