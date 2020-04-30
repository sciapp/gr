#if defined(NO_QT4)
#define NO_QT
#endif

#ifndef NO_QT

#include <QtGui/QWidget>
#include <QtGui/QPainter>
#include <QtGui/QImage>
#include <QtGui/QApplication>
#include <QtGui/QDesktopWidget>

#endif

#define QT_NAME_STRING "Qt4"
#define QT_PLUGIN_ENTRY_NAME gks_qtplugin

#include "qtplugin_impl.cxx"
