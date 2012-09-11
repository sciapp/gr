#ifndef GR3WIDGET_H
#define GR3WIDGET_H

#include <QtGui>
#include <QtOpenGL>
#include <gr3.h>

class PlottableFunction {
public:
    virtual void transform(float nx, float ny, float& fx, float& fy) = 0;
    virtual float f(float fx, float fy, float t = 0) = 0;
    virtual float dfdx(float fx, float fy, float t = 0) = 0;
    virtual float dfdy(float fx, float fy, float t = 0) = 0;
};

class ExampleFunction : public PlottableFunction {
    void transform(float nx, float ny, float &fx, float &fy) {
        fx = nx*2-1;
        fx*= 100;
        fy = ny*2-1;
        fy*= 100;
    }
    virtual float f(float fx, float fy, float t = 0) {
        return sin(fx*0.3+t*5) + 5*sin(t)*sin(fy*0.1)*2;
    }
    virtual float dfdx(float fx, float fy, float t = 0) {
        return cos(fx*0.3+t*5)*0.3;
    }
    virtual float dfdy(float fx, float fy, float t = 0) {
        return 5*sin(t)*cos(fy*0.1)*0.1*2;
    }
};



class GR3Widget: public QGLWidget {
    Q_OBJECT

public:
    GR3Widget(QWidget* parent) : QGLWidget(QGLFormat(QGL::SampleBuffers), parent), gr3_initialized(false), gr3_mesh(0)
    {
        func = new ExampleFunction();
        timer.setInterval(16);
        connect(&timer, SIGNAL(timeout()), this,SLOT(updateGL()));
        timer.start();
        start_time = QDateTime::currentDateTime();
        tilt = 0;
        rotation = 0;
        zoom = 100;
        setFocusPolicy(Qt::StrongFocus);
        grabKeyboard();
    }

    ~GR3Widget()
    {
        delete func;
        if (gr3_initialized) {
            gr3_terminate();
            gr3_initialized = false;
        }
    }

    void keyPressEvent(QKeyEvent *event) {
        switch (event->key()) {
        case Qt::Key_Up:
            zoom-=3;
            event->accept();
            break;
        case Qt::Key_Down:
            zoom+=3;
            event->accept();
            break;
        default:
            break;
        }
        if (zoom > 180) zoom = 180;
        if (zoom < 10) zoom = 10;
    }

    void mousePressEvent(QMouseEvent *event) {
        last_x = event->x();
        last_y = event->y();
        event->accept();
    }

    void mouseReleaseEvent(QMouseEvent *event) {
        last_x = -1;
        last_y = -1;
        event->accept();
    }

    void mouseMoveEvent(QMouseEvent *event) {
        event->accept();
        if (last_x < 0 || last_y < 0) {
            return;
        }
        int dx = last_x - event->x();
        int dy = last_y - event->y();
        last_x = event->x();
        last_y = event->y();
        float ndx = 1.0f*dx/width();
        float ndy = 1.0f*dy/height();
        rotation += ndx;
        tilt -= ndy;
        if (tilt < 0) tilt = 0;
        if (tilt > M_PI/2-0.1) tilt = M_PI/2-0.1;
    }

    void updateScene(float t) {
        float camera_x = zoom*cos(rotation)*cos(tilt);
        float camera_y = zoom*sin(rotation)*cos(tilt);
        float camera_z = zoom*sin(tilt);
        gr3_cameralookat(camera_x,camera_y,camera_z,0,0,0,0,0,1);
        gr3_deletemesh(gr3_mesh);
        gr3_clear();
        {
            int xdim = 100;
            int ydim = 100;
            float *vertices = new float[xdim*ydim*18];
            float *normals = new float[xdim*ydim*18];
            float *colors = new float[xdim*ydim*18];
            int i, x, y;
            int dx[] = {0,1,1, 1,0,0};
            int dy[] = {0,0,1, 1,0,1};
            for (x = 0; x < xdim; x++) {
                for (y = 0; y < ydim; y++) {
                    for (i = 0; i < 6; i++) {
                        // normalize x and y to [0;1]
                        float nx = 1.0f*(x+dx[i])/xdim;
                        float ny = 1.0f*(y+dy[i])/xdim;
                        float fx, fy;
                        // transform x and y
                        func->transform(nx, ny, fx, fy);

                        vertices[((y*xdim+x)*6+i)*3+0] = fx;
                        vertices[((y*xdim+x)*6+i)*3+1] = fy;
                        vertices[((y*xdim+x)*6+i)*3+2] = func->f(fx, fy, t);

                        normals[((y*xdim+x)*6+i)*3+0] = -func->dfdx(fx, fy, t);
                        normals[((y*xdim+x)*6+i)*3+1] = -func->dfdy(fx, fy, t);
                        normals[((y*xdim+x)*6+i)*3+2] = 1;
                        normalize(normals + ((y*xdim+x)*6+i)*3+0);

                        colors[((y*xdim+x)*6+i)*3+0] = 1;
                        colors[((y*xdim+x)*6+i)*3+1] = 0;
                        colors[((y*xdim+x)*6+i)*3+2] = 0;
                    }
                }
            }

            gr3_createmesh(&gr3_mesh, xdim*ydim*6, vertices, normals, colors);
            delete[] vertices;
            delete[] normals;
            delete[] colors;
        }
        {
            float positions[] = {0,0,0};
            float directions[] = {0,0,1};
            float ups[] = {0,1,0};
            float colors[] = {1,1,1};
            float scales[] = {0.5,0.5,1};
            gr3_drawmesh(gr3_mesh, 1, positions, directions, ups, colors, scales);
        }
    }

    void initializeGL()
    {
        if (!gr3_initialized) {
            int attrib_list[] = {GR3_IA_END_OF_LIST};
            gr3_init(attrib_list);
            gr3_setcameraprojectionparameters(45,1,400);
            gr3_setbackgroundcolor(1,1,1,1);
            gr3_initialized = true;
            qDebug() << "GR3 info:" << gr3_getrenderpathstring() << endl;
        }
    }

    void resizeGL(int width, int height)
    {
        glViewport(0, 0, (GLint)width, (GLint)height);
    }

    void  paintGL()
    {
        int time = start_time.msecsTo(QDateTime::currentDateTime());
        updateScene(time/1000.0);
        gr3_drawimage(0, this->geometry().width(), 0, this->geometry().height(), this->geometry().width(), this->geometry().height(), GR3_DRAWABLE_OPENGL);
    }

    void repaint()
    {
        paintGL();
    }

    void normalize(float *vec) {
        float tmp = 0;
        int i;
        for (i = 0; i < 3; i++) {
            tmp += vec[i]*vec[i];
        }
        tmp = sqrt(tmp);
        for (i = 0; i < 3; i++) {
            vec[i]/=tmp;
        }
    }

private:
    bool gr3_initialized;
    int gr3_mesh;
    PlottableFunction* func;
    QTimer timer;
    QDateTime start_time;
    int last_x;
    int last_y;
    float rotation;
    float tilt;
    int zoom;
};

#endif // GR3WIDGET_H
