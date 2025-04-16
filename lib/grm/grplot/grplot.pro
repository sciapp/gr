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
QMAKE_CXXFLAGS += -std=c++17 $$(XERCESCDEFS) $$(EXTRA_CXXFLAGS)
QMAKE_LFLAGS += $$(EXTRA_LDFLAGS)
QMAKE_MACOSX_DEPLOYMENT_TARGET = 10.15
HEADERS += grplotWidget.hxx grplotMainwindow.hxx util.hxx gredit/BoundingLogic.hxx gredit/BoundingObject.hxx gredit/CustomTreeWidgetItem.hxx gredit/TreeWidget.hxx gredit/AddElementWidget.hxx gredit/TableWidget.hxx gredit/EditElementWidget.hxx qtterm/ArgsWrapper.hxx qtterm/Receiver.hxx
SOURCES += grplotWidget.cxx grplot.cxx grplotMainwindow.cxx util.cxx gredit/BoundingLogic.cpp gredit/BoundingObject.cpp gredit/CustomTreeWidgetItem.cpp gredit/TreeWidget.cpp gredit/AddElementWidget.cpp gredit/TableWidget.cpp gredit/EditElementWidget.cpp qtterm/ArgsWrapper.cpp qtterm/Receiver.cpp
INCLUDEPATH += ../include ../../gr
if (macx) {
    if (exists(../libGRM.dylib)) {
      LIBS += -L.. -L../../gr -lGRM -lGR
    } else {
      LIBS += -L$(GRDIR)/lib -lGRM -lGR
    }
    # On macOS, the grplot executable is located in `$(GRDIR)/Applications/grplot.app/Contents/MacOS`
    # and we need to resolve `libGRM.dylib` in `$(GRDIR)/lib`
    QMAKE_RPATHDIR += ../../../../lib
} else {
    if (exists(../libGRM.so)) {
      LIBS += -L.. -L../../gr -lGRM -lGR -Wl,-rpath-link,../../gr -Wl,-rpath-link,../../gr3
    } else {
      LIBS += -L$(GRDIR)/lib -lGRM -lGR -Wl,-rpath-link,$(GRDIR)/lib
    }
    if (exists($$THIRDPARTYDIR/lib/libicuuc.a)) {
      LIBS += $$THIRDPARTYDIR/lib/libicuuc.a $$THIRDPARTYDIR/lib/libicudata.a -ldl
    }
    # On every other system, the grplot executable is located in `$(GRDIR)/bin`
    # and we need to resolve `libGRM.so` in `$(GRDIR)/lib`
    QMAKE_RPATHDIR += ../lib
}
