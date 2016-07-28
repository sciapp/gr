#ifndef GRWIDGET_H_INCLUDED
#define GRWIDGET_H_INCLUDED

#include <QWidget>

#ifdef _WIN32
#ifdef COMPILING_DLL
#define DLL __declspec(dllexport)
#else
#define DLL __declspec(dllimport)
#endif
#else
#define DLL
#endif


class DLL GRWidget : public QWidget {
    public:
        GRWidget(QWidget *parent = 0);

    protected:
        void paintEvent(QPaintEvent *event);
        virtual void draw() = 0;

    private:
        void init_gks();
};

#undef DLL
#endif /* ifndef GRWIDGET_H_INCLUDED */
