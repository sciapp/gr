#ifndef MAIN_WINDOW_H_INCLUDED
#define MAIN_WINDOW_H_INCLUDED

#include <QMainWindow>
#include <QPaintEvent>

class MainWindow : public QMainWindow {
    public:
        MainWindow();

    protected:
        void paintEvent(QPaintEvent *event);
        void draw();

};

#endif /* ifndef MAIN_WINDOW_H_INCLUDED */
