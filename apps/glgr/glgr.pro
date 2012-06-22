TEMPLATE	= app
TARGET		= glgr

CONFIG		+= qt opengl warn_on release
CONFIG		-= dlopen_opengl
!mac:unix:LIBS  += -lm

REQUIRES	= opengl

HEADERS		= glgr.h rect.h vec.h glgrserver.h
SOURCES		= main.cpp glgr.cpp rect.cpp vec.cpp glgrserver.cpp
QT		+= network xml opengl
