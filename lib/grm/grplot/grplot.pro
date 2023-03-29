GRDIR = $$(GRDIR)
isEmpty(GRDIR) {
  GRDIR = /usr/local/gr
}
QT += widgets core gui
CONFIG += c++17
DEFINES += GRDIR=\\\"$(GRDIR)\\\"
# Qt versions < 5.12 ignore `CONFIG`, so repeat the language flag here.
# Only using `QMAKE_CXXFLAGS` does not work since newer Qt installations set
# conflicting language flags themselves without setting `CONFIG`.
QMAKE_CXXFLAGS += -std=c++17 $$(EXTRA_CXXFLAGS)
QMAKE_LFLAGS += $$(EXTRA_LDFLAGS)
HEADERS += grplot_widget.hxx grplot_mainwindow.hxx util.hxx gredit/Bounding_logic.h gredit/Bounding_object.h gredit/CustomTreeWidgetItem.h gredit/TreeWidget.h
SOURCES += grplot_widget.cxx grplot.cxx grplot_mainwindow.cxx util.cxx gredit/Bounding_logic.cpp gredit/Bounding_object.cpp gredit/CustomTreeWidgetItem.cpp gredit/TreeWidget.cpp
INCLUDEPATH += ../include ../../gr
if (macx) {
    if (exists(../libGRM.dylib)) {
      LIBS += -L.. -lGRM
    } else {
      LIBS += -L$(GRDIR)/lib -lGRM
    }
    # On macOS, the grplot executable is located in `$(GRDIR)/Applications/grplot.app/Contents/MacOS`
    # and we need to resolve `libGRM.dylib` in `$(GRDIR)/lib`
    QMAKE_RPATHDIR += ../../../../lib
} else {
    if (exists(../libGRM.so)) {
      LIBS += -L.. -lGRM -Wl,-rpath-link,../../gr -Wl,-rpath-link,../../gr3
    } else {
      LIBS += -L$(GRDIR)/lib -lGRM -Wl,-rpath-link,$(GRDIR)/lib
    }
    # On every other system, the grplot executable is located in `$(GRDIR)/bin`
    # and we need to resolve `libGRM.so` in `$(GRDIR)/lib`
    QMAKE_RPATHDIR += ../lib
}
