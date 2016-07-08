#ifndef MAIN_WINDOW_H_INCLUDED
#define MAIN_WINDOW_H_INCLUDED

#include "grwidget.h"

class MainWindow : public GRWidget {
    public:
        MainWindow();

    protected:
        virtual void draw();

};

#endif /* ifndef MAIN_WINDOW_H_INCLUDED */
