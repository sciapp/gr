#if defined(NO_QT4)
#define NO_QT
#endif

#ifndef NO_QT

#include <QtGui/QWidget>
#include <QtGui/QPainter>
#include <QtGui/QImage>
#include <QtGui/QPicture>
#include <QtGui/QApplication>
#include <QtGui/QDesktopWidget>

#endif

#define QT_NAME_STRING "Qt4"
#define QT_PLUGIN_ENTRY_NAME gks_qtplugin
#define QT_PLUGIN_USED_AS_PLUGIN_CODE 1

#include "qtplugin_impl.cxx"
