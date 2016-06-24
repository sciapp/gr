#if defined(NO_QT5)
#define NO_QT
#endif

#ifndef NO_QT

#include <QtWidgets/QWidget>
#include <QtGui/QPainter>
#include <QtGui/QImage>

#endif

#define QT_NAME_STRING "Qt5"
#define QT_PLUGIN_ENTRY_NAME gks_qt5plugin

#include "qtplugin_impl.cxx"
