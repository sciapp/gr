GRDIR           = $$(GRDIR)
isEmpty(GRDIR) {
  GRDIR         = /usr/local/gr
}
QT             += widgets
DEFINES        += GRDIR=\\\"$(GRDIR)\\\"
TEMPLATE        = app
TARGET          = qtterm
INCLUDEPATH    += $$GRDIR/include
QMAKE_CXXLAGS  += $$(EXTRA_CXXFLAGS)
QMAKE_LFLAGS   += $$(EXTRA_LDFLAGS)
HEADERS        += main_window.h receiver_thread.h tooltip.h grm_args_t_wrapper.h
SOURCES        += main.cpp main_window.cpp receiver_thread.cpp tooltip.cpp grm_args_t_wrapper.cpp
LIBS           += -L$$GRDIR/lib -lGR3 -lGR -lGRM
RESOURCES      += resources/resources.qrc
ICON            = resources/qtterm.icns
QMAKE_RPATHDIR += $$GRDIR/lib

# For debugging
# QMAKE_LFLAGS   += -fsanitize=address
