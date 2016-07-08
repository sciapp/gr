#include <QPainter>
#include <cstdlib>
#include <cstdio>
#include "gr.h"

#define COMPILING_DLL
#include "grwidget.h"

GRWidget::GRWidget(QWidget *parent) : QWidget(parent) {
    init_gks();
}

void GRWidget::init_gks() {
#ifdef _WIN32
    putenv("GKS_WSTYPE=381");
    putenv("GKS_DOUBLE_BUF=True");
#else
    setenv("GKS_WSTYPE", "381", 1);
    setenv("GKS_DOUBLE_BUF", "True", 1);
#endif
}

void GRWidget::paintEvent(QPaintEvent *event) {
    QPainter painter;
    char buf[100];

#ifdef _WIN32
    sprintf(buf, "GKS_CONID=%p!%p", this, &painter);
    putenv(buf);
#else
    sprintf(buf, "%p!%p", this, &painter);
    setenv("GKS_CONID", buf, 1);
#endif

    painter.begin(this);
    painter.fillRect(0, 0, width(), height(), QColor("white"));

    gr_clearws();
    draw();
    gr_updatews();

    painter.end();
}
