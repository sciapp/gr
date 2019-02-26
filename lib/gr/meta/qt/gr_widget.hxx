#include <QWidget>
#include <QRubberBand>
#include <gr.h>

class GRWidget : public QWidget
{
  Q_OBJECT

public:
  GRWidget(QWidget *parent);
  virtual ~GRWidget() override;

signals:
  void mouse_pos_changed(const QPoint &);

protected:
  virtual void init_env_vars();
  virtual void init_plot_data();
  virtual void init_ui();
  virtual void draw();
  void keyPressEvent(QKeyEvent *event) override;
  void mouseMoveEvent(QMouseEvent *event) override;
  void mousePressEvent(QMouseEvent *event) override;
  void mouseReleaseEvent(QMouseEvent *event) override;
  void paintEvent(QPaintEvent *event) override;
  void resizeEvent(QResizeEvent *event) override;
  void wheelEvent(QWheelEvent *event) override;

private:
  class MouseState
  {
  public:
    enum class Mode
    {
      normal,
      pan,
      zoom,
      boxzoom
    };

    MouseState();
    virtual ~MouseState();

    Mode mode() const;
    void mode(const Mode &mode);

    QPoint mode_start_pos() const;
    void mode_start_pos(const QPoint &point);

  private:
    Mode mode_;
    QPoint mode_start_pos_;
  };


  gr_meta_args_t *args_;
  MouseState mouse_state_;
  QRubberBand *box_zoom_rubberband_;
};
