#if defined(NO_QT6)
#define NO_QT
#endif

#ifndef NO_QT

#include <QtWidgets/QWidget>
#include <QtGui/QPainter>
#include <QtGui/QPainterPath>
#include <QtGui/QImage>
#include <QtGui/QGuiApplication>
#include <QtGui/QScreen>

#endif

#define QT_NAME_STRING "Qt6"
#define QT_PLUGIN_ENTRY_NAME gks_qt6plugin

#include "qtplugin_impl.cxx"
