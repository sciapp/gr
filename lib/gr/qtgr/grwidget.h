#ifndef GRWIDGET_H_INCLUDED
#define GRWIDGET_H_INCLUDED

#include <QWidget>

#ifdef _WIN32
#ifdef COMPILING_DLL
#define DLLEXPORT __declspec(dllexport)
#else
#define DLLEXPORT __declspec(dllimport)
#endif
#else
#define DLLEXPORT
#endif


class DLLEXPORT GRWidget : public QWidget {
    public:
        GRWidget(QWidget *parent = 0);

    protected:
        void paintEvent(QPaintEvent *event);
        virtual void draw() = 0;

    private:
        void init_gks();
};

#undef DLLEXPORT
#endif /* ifndef GRWIDGET_H_INCLUDED */
