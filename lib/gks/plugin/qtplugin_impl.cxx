
#ifndef NO_QT

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <assert.h>
#include <string.h>
#include <math.h>
#include <float.h>
#include <map>
#include <memory>
#include <stack>

#include <QtGlobal>
#include <QDebug>

#endif

#include "gks.h"
#include "gkscore.h"

#ifdef __cplusplus
extern "C" {
#endif

#ifdef _WIN32

#include <windows.h>
#ifndef DLLEXPORT
#define DLLEXPORT __declspec(dllexport)
#endif

#endif

DLLEXPORT void QT_PLUGIN_ENTRY_NAME(int fctid, int dx, int dy, int dimx, int *i_arr, int len_f_arr_1, double *f_arr_1,
                                    int len_f_arr_2, double *f_arr_2, int len_c_arr, char *c_arr, void **ptr);

#ifdef __cplusplus
}
#endif

#ifndef GKS_UNUSED
#define GKS_UNUSED(x) (void)(x)
#endif

#ifndef NO_QT

#define MAX_POINTS 2048
#define MAX_POINTS_PERFORMANCE_THRESHOLD 500
#define MAX_POLYGON 32
#define MAX_SELECTIONS 100
#define PATTERNS 120
#define HATCH_STYLE 108

#define DrawBorder 0

#define RESOLVE(arg, type, nbytes) \
  arg = (type *)(s + sp);          \
  sp += nbytes

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

#define MAX_TNR 9

#define WC_to_NDC(xw, yw, tnr, xn, yn) \
  xn = a[tnr] * (xw) + b[tnr];         \
  yn = c[tnr] * (yw) + d[tnr]

#define WC_to_NDC_rel(xw, yw, tnr, xn, yn) \
  xn = a[tnr] * (xw);                      \
  yn = c[tnr] * (yw)

#define NDC_to_DC(xn, yn, xd, yd) \
  xd = (p->a * (xn) + p->b);      \
  yd = (p->c * (yn) + p->d);

#define DC_to_NDC(xd, yd, xn, yn) \
  xn = ((xd)-p->b) / p->a;        \
  yn = ((yd)-p->d) / p->c;

#define CharXform(xrel, yrel, x, y)                  \
  x = cos(p->alpha) * (xrel)-sin(p->alpha) * (yrel); \
  y = sin(p->alpha) * (xrel) + cos(p->alpha) * (yrel);

#define nint(a) ((int)(a + 0.5))

#ifndef min
#define min(a, b) (((a) < (b)) ? (a) : (b))
#endif

#ifndef max
#define max(a, b) (((a) > (b)) ? (a) : (b))
#endif

template <typename Predicate, typename Iterator> class filter_iterator
{
public:
  using value_type = typename std::iterator_traits<Iterator>::value_type;
  using reference = typename std::iterator_traits<Iterator>::reference;
  using pointer = typename std::iterator_traits<Iterator>::pointer;
  using difference_type = typename std::iterator_traits<Iterator>::difference_type;

  filter_iterator(Predicate pred, Iterator iter, Iterator end) : pred_(pred), iter_(iter), end_(end)
  {
    advance_if_invalid();
  }

  reference operator*() const { return *iter_; }
  pointer operator->() const { return &(*iter_); }

  filter_iterator &operator++()
  {
    ++iter_;
    advance_if_invalid();
    return *this;
  }

  bool operator==(const filter_iterator &other) const { return iter_ == other.iter_; }
  bool operator!=(const filter_iterator &other) const { return !(*this == other); }

private:
  void advance_if_invalid()
  {
    while (iter_ != end_ && !pred_(*iter_))
      {
        ++iter_;
      }
  }

  Predicate pred_;
  Iterator iter_;
  Iterator end_;
};

class PartialPainters
{
public:
  struct PainterAndImage
  {
    bool active = true;
    std::unique_ptr<QImage> image; // image must come first, so the painter will be destroyed before the image
    std::unique_ptr<QPainter> painter;

    PainterAndImage(std::unique_ptr<QPainter> painter, std::unique_ptr<QImage> image)
        : painter(std::move(painter)), image(std::move(image))
    {
    }
  };

  struct PaintedRegion
  {
    unsigned int x, y, w, h; // position, width, height
    unsigned int *pixels;
  };

  using PartialPaintersIterator = filter_iterator<bool (*)(const std::pair<const unsigned int, PainterAndImage> &),
                                                  std::map<unsigned int, PainterAndImage>::iterator>;
  using PartialPaintersConstIterator = filter_iterator<bool (*)(const std::pair<const unsigned int, PainterAndImage> &),
                                                       std::map<unsigned int, PainterAndImage>::const_iterator>;

  explicit PartialPainters(const QPainter &parentPainter) : parentPainter_(parentPainter) {}

  void startPainting(unsigned int id)
  {
    auto image = std::unique_ptr<QImage>(
        new QImage(parentPainter_.device()->width(), parentPainter_.device()->height(), QImage::Format_ARGB32));
    auto painter = std::unique_ptr<QPainter>(new QPainter(image.get()));
    paintersAndImages_.emplace(id, PainterAndImage{std::move(painter), std::move(image)});
  };

  void stopPainting(unsigned int id) { paintersAndImages_.at(id).active = false; }

  const unsigned int *getPixels(unsigned int id) const
  {
    return reinterpret_cast<const uint32_t *>(paintersAndImages_.at(id).image->constBits());
  }

  std::map<unsigned int, const unsigned int *> getPixels() const
  {
    std::map<unsigned int, const unsigned int *> pixels;
    for (auto &painterAndImage : paintersAndImages_)
      {
        pixels.emplace(painterAndImage.first,
                       reinterpret_cast<const uint32_t *>(painterAndImage.second.image->constBits()));
      }
    return pixels;
  }

  PaintedRegion getPaintedCRegion(unsigned int id) const
  {
    PaintedRegion region{UINT_MAX, UINT_MAX, 0, 0, nullptr};
    auto pixels = getPixels(id);
    const auto &paintDevice = *parentPainter_.device();

    for (unsigned int y = 0; y < paintDevice.height(); y++)
      {
        for (unsigned int x = 0; x < paintDevice.width(); x++)
          {
            // Check if the current pixels is not transparent
            if ((pixels[y * paintDevice.width() + x] & 0xFF000000) != 0)
              {
                region.x = min(region.x, x);
                region.y = min(region.y, y);
                region.w = max(region.w, x);
                region.h = max(region.h, y);
              }
          }
      }
    region.w -= region.x;
    region.h -= region.y;

    region.pixels = reinterpret_cast<unsigned int *>(malloc(region.w * region.h * sizeof(unsigned int)));
    if (region.pixels == nullptr) throw std::bad_alloc();
    // Strided memcpy (one memcpy invocation for every line of the found region)
    for (unsigned int y = 0; y < region.h; y++)
      {
        memcpy(region.pixels + y * region.w, pixels + (region.y + y) * paintDevice.width() + region.x,
               region.w * sizeof(unsigned int));
      }

    return region;
  }

  std::map<unsigned int, PaintedRegion> getPaintedCRegions() const
  {
    std::map<unsigned int, PaintedRegion> regions;
    for (auto &painterAndImage : paintersAndImages_)
      {
        regions.emplace(painterAndImage.first, getPaintedCRegion(painterAndImage.first));
      }
    return regions;
  }

  PartialPaintersIterator begin()
  {
    return PartialPaintersIterator(isMapElementActive, paintersAndImages_.begin(), paintersAndImages_.end());
  }

  PartialPaintersIterator end()
  {
    return PartialPaintersIterator(isMapElementActive, paintersAndImages_.end(), paintersAndImages_.end());
  }

  PartialPaintersConstIterator begin() const
  {
    return PartialPaintersConstIterator(isMapElementActive, paintersAndImages_.begin(), paintersAndImages_.end());
  }

  PartialPaintersConstIterator end() const
  {
    return PartialPaintersConstIterator(isMapElementActive, paintersAndImages_.end(), paintersAndImages_.end());
  }

  PartialPaintersConstIterator cbegin() const noexcept
  {
    return PartialPaintersConstIterator(isMapElementActive, paintersAndImages_.cbegin(), paintersAndImages_.cend());
  }

  PartialPaintersConstIterator cend() const noexcept
  {
    return PartialPaintersConstIterator(isMapElementActive, paintersAndImages_.cend(), paintersAndImages_.cend());
  }

#define paintOnActive(method, ...)                             \
  do                                                           \
    {                                                          \
      for (auto &painterAndImage : *this)                      \
        {                                                      \
          painterAndImage.second.painter->method(__VA_ARGS__); \
        }                                                      \
    }                                                          \
  while (false)

  void setCompositionMode(QPainter::CompositionMode mode) { paintOnActive(setCompositionMode, mode); }

  void setFont(const QFont &f) { paintOnActive(setFont, f); }

  void setPen(const QColor &color) { paintOnActive(setPen, color); }
  void setPen(const QPen &pen) { paintOnActive(setPen, pen); }
  void setPen(Qt::PenStyle style) { paintOnActive(setPen, style); }

  void setBrush(const QBrush &brush) { paintOnActive(setBrush, brush); }
  void setBrush(Qt::BrushStyle style) { paintOnActive(setBrush, style); }
  void setBrush(QColor color) { paintOnActive(setBrush, color); }
  void setBrush(Qt::GlobalColor color) { paintOnActive(setBrush, color); }

  void setBackgroundMode(Qt::BGMode mode) { paintOnActive(setBackgroundMode, mode); }

  void setBrushOrigin(int x, int y) { paintOnActive(setBrushOrigin, x, y); }
  void setBrushOrigin(const QPoint &p) { paintOnActive(setBrushOrigin, p); }
  void setBrushOrigin(const QPointF &p) { paintOnActive(setBrushOrigin, p); }

  void setBackground(const QBrush &bg) { paintOnActive(setBackground, bg); }

  void setOpacity(qreal opacity) { paintOnActive(setOpacity, opacity); }

  void setClipRect(const QRectF &rect, Qt::ClipOperation op = Qt::ReplaceClip) { paintOnActive(setClipRect, rect, op); }
  void setClipRect(const QRect &rect, Qt::ClipOperation op = Qt::ReplaceClip) { paintOnActive(setClipRect, rect, op); }
  void setClipRect(int x, int y, int w, int h, Qt::ClipOperation op = Qt::ReplaceClip)
  {
    paintOnActive(setClipRect, x, y, w, h, op);
  }

  void setClipRegion(const QRegion &region, Qt::ClipOperation op = Qt::ReplaceClip)
  {
    paintOnActive(setClipRegion, region, op);
  }

  void setClipPath(const QPainterPath &path, Qt::ClipOperation op = Qt::ReplaceClip)
  {
    paintOnActive(setClipPath, path, op);
  }

  void setClipping(bool enable) { paintOnActive(setClipping, enable); }

  void save() { paintOnActive(save); }
  void restore() { paintOnActive(restore); }

  void setTransform(const QTransform &transform, bool combine = false)
  {
    paintOnActive(setTransform, transform, combine);
  }
  void resetTransform() { paintOnActive(resetTransform); }

  void setWorldTransform(const QTransform &matrix, bool combine = false)
  {
    paintOnActive(setWorldTransform, matrix, combine);
  }


  void setWorldMatrixEnabled(bool enabled) { paintOnActive(setWorldMatrixEnabled, enabled); }

  void scale(qreal sx, qreal sy) { paintOnActive(scale, sx, sy); }
  void shear(qreal sh, qreal sv) { paintOnActive(shear, sh, sv); }
  void rotate(qreal a) { paintOnActive(rotate, a); }

  void translate(const QPointF &offset) { paintOnActive(translate, offset); }
  void translate(const QPoint &offset) { paintOnActive(translate, offset); }
  void translate(qreal dx, qreal dy) { paintOnActive(translate, dx, dy); }

  void setWindow(const QRect &window) { paintOnActive(setWindow, window); }
  void setWindow(int x, int y, int w, int h) { paintOnActive(setWindow, x, y, w, h); }

  void setViewport(const QRect &viewport) { paintOnActive(setViewport, viewport); }
  void setViewport(int x, int y, int w, int h) { paintOnActive(setViewport, x, y, w, h); }

  void setViewTransformEnabled(bool enable) { paintOnActive(setViewTransformEnabled, enable); }

  void strokePath(const QPainterPath &path, const QPen &pen) { paintOnActive(strokePath, path, pen); }
  void fillPath(const QPainterPath &path, const QBrush &brush) { paintOnActive(fillPath, path, brush); }
  void drawPath(const QPainterPath &path) { paintOnActive(drawPath, path); }

  void drawPoint(const QPointF &pt) { paintOnActive(drawPoint, pt); }
  void drawPoint(const QPoint &p) { paintOnActive(drawPoint, p); }
  void drawPoint(int x, int y) { paintOnActive(drawPoint, x, y); }

  void drawPoints(const QPointF *points, int pointCount) { paintOnActive(drawPoints, points, pointCount); }
  void drawPoints(const QPolygonF &points) { paintOnActive(drawPoints, points); }
  void drawPoints(const QPoint *points, int pointCount) { paintOnActive(drawPoints, points, pointCount); }
  void drawPoints(const QPolygon &points) { paintOnActive(drawPoints, points); }

  void drawLine(const QLineF &line) { paintOnActive(drawLine, line); }
  void drawLine(const QLine &line) { paintOnActive(drawLine, line); }
  void drawLine(int x1, int y1, int x2, int y2) { paintOnActive(drawLine, x1, y1, x2, y2); }
  void drawLine(const QPoint &p1, const QPoint &p2) { paintOnActive(drawLine, p1, p2); }
  void drawLine(const QPointF &p1, const QPointF &p2) { paintOnActive(drawLine, p1, p2); }

  void drawLines(const QLineF *lines, int lineCount) { paintOnActive(drawLines, lines, lineCount); }
  void drawLines(const QPointF *pointPairs, int lineCount) { paintOnActive(drawLines, pointPairs, lineCount); }
  void drawLines(const QLine *lines, int lineCount) { paintOnActive(drawLines, lines, lineCount); }
  void drawLines(const QPoint *pointPairs, int lineCount) { paintOnActive(drawLines, pointPairs, lineCount); }
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
  void drawLines(const QList<QLineF> &lines) { paintOnActive(drawLines, lines); }
  void drawLines(const QList<QPointF> &pointPairs) { paintOnActive(drawLines, pointPairs); }
  void drawLines(const QList<QLine> &lines) { paintOnActive(drawLines, lines); }
  void drawLines(const QList<QPoint> &pointPairs) { paintOnActive(drawLines, pointPairs); }
#endif

  void drawRect(const QRectF &rect) { paintOnActive(drawRect, rect); }
  void drawRect(int x1, int y1, int w, int h) { paintOnActive(drawRect, x1, y1, w, h); }
  void drawRect(const QRect &rect) { paintOnActive(drawRect, rect); }

  void drawRects(const QRectF *rects, int rectCount) { paintOnActive(drawRects, rects, rectCount); }
  void drawRects(const QRect *rects, int rectCount) { paintOnActive(drawRects, rects, rectCount); }
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
  void drawRects(const QList<QRectF> &rectangles) { paintOnActive(drawRects, rectangles); }
  void drawRects(const QList<QRect> &rectangles) { paintOnActive(drawRects, rectangles); }
#endif

  void drawEllipse(const QRectF &r) { paintOnActive(drawEllipse, r); }
  void drawEllipse(const QRect &r) { paintOnActive(drawEllipse, r); }
  void drawEllipse(int x, int y, int w, int h) { paintOnActive(drawEllipse, x, y, w, h); }

  void drawEllipse(const QPointF &center, qreal rx, qreal ry) { paintOnActive(drawEllipse, center, rx, ry); }
  void drawEllipse(const QPoint &center, int rx, int ry) { paintOnActive(drawEllipse, center, rx, ry); }

  void drawPolyline(const QPointF *points, int pointCount) { paintOnActive(drawPolyline, points, pointCount); }
  void drawPolyline(const QPolygonF &polyline) { paintOnActive(drawPolyline, polyline); }
  void drawPolyline(const QPoint *points, int pointCount) { paintOnActive(drawPolyline, points, pointCount); }
  void drawPolyline(const QPolygon &polygon) { paintOnActive(drawPolyline, polygon); }

  void drawPolygon(const QPointF *points, int pointCount, Qt::FillRule fillRule = Qt::OddEvenFill)
  {
    paintOnActive(drawPolygon, points, pointCount, fillRule);
  }
  void drawPolygon(const QPolygonF &polygon, Qt::FillRule fillRule = Qt::OddEvenFill)
  {
    paintOnActive(drawPolygon, polygon, fillRule);
  }
  void drawPolygon(const QPoint *points, int pointCount, Qt::FillRule fillRule = Qt::OddEvenFill)
  {
    paintOnActive(drawPolygon, points, pointCount, fillRule);
  }
  void drawPolygon(const QPolygon &polygon, Qt::FillRule fillRule = Qt::OddEvenFill)
  {
    paintOnActive(drawPolygon, polygon, fillRule);
  }

  void drawConvexPolygon(const QPointF *points, int pointCount)
  {
    paintOnActive(drawConvexPolygon, points, pointCount);
  }
  void drawConvexPolygon(const QPolygonF &polygon) { paintOnActive(drawConvexPolygon, polygon); }
  void drawConvexPolygon(const QPoint *points, int pointCount) { paintOnActive(drawConvexPolygon, points, pointCount); }
  void drawConvexPolygon(const QPolygon &polygon) { paintOnActive(drawConvexPolygon, polygon); }

  void drawArc(const QRectF &rect, int a, int alen) { paintOnActive(drawArc, rect, a, alen); }
  void drawArc(const QRect &rect, int a, int alen) { paintOnActive(drawArc, rect, a, alen); }
  void drawArc(int x, int y, int w, int h, int a, int alen) { paintOnActive(drawArc, x, y, w, h, a, alen); }

  void drawPie(const QRectF &rect, int a, int alen) { paintOnActive(drawPie, rect, a, alen); }
  void drawPie(int x, int y, int w, int h, int a, int alen) { paintOnActive(drawPie, x, y, w, h, a, alen); }
  void drawPie(const QRect &rect, int a, int alen) { paintOnActive(drawPie, rect, a, alen); }

  void drawChord(const QRectF &rect, int a, int alen) { paintOnActive(drawChord, rect, a, alen); }
  void drawChord(int x, int y, int w, int h, int a, int alen) { paintOnActive(drawChord, x, y, w, h, a, alen); }
  void drawChord(const QRect &rect, int a, int alen) { paintOnActive(drawChord, rect, a, alen); }

  void drawRoundedRect(const QRectF &rect, qreal xRadius, qreal yRadius, Qt::SizeMode mode = Qt::AbsoluteSize)
  {
    paintOnActive(drawRoundedRect, rect, xRadius, yRadius, mode);
  }
  void drawRoundedRect(int x, int y, int w, int h, qreal xRadius, qreal yRadius, Qt::SizeMode mode = Qt::AbsoluteSize)
  {
    paintOnActive(drawRoundedRect, x, y, w, h, xRadius, yRadius, mode);
  }
  void drawRoundedRect(const QRect &rect, qreal xRadius, qreal yRadius, Qt::SizeMode mode = Qt::AbsoluteSize)
  {
    paintOnActive(drawRoundedRect, rect, xRadius, yRadius, mode);
  }

  void drawTiledPixmap(const QRectF &rect, const QPixmap &pm, const QPointF &offset = QPointF())
  {
    paintOnActive(drawTiledPixmap, rect, pm, offset);
  }
  void drawTiledPixmap(int x, int y, int w, int h, const QPixmap &pixmap, int sx = 0, int sy = 0)
  {
    paintOnActive(drawTiledPixmap, x, y, w, h, pixmap, sx, sy);
  }
  void drawTiledPixmap(const QRect &rect, const QPixmap &pixmap, const QPoint &offset = QPoint())
  {
    paintOnActive(drawTiledPixmap, rect, pixmap, offset);
  }
  void drawPicture(const QPointF &p, const QPicture &picture) { paintOnActive(drawPicture, p, picture); }
  void drawPicture(int x, int y, const QPicture &picture) { paintOnActive(drawPicture, x, y, picture); }
  void drawPicture(const QPoint &p, const QPicture &picture) { paintOnActive(drawPicture, p, picture); }

  void drawPixmap(const QRectF &targetRect, const QPixmap &pixmap, const QRectF &sourceRect)
  {
    paintOnActive(drawPixmap, targetRect, pixmap, sourceRect);
  }
  void drawPixmap(const QRect &targetRect, const QPixmap &pixmap, const QRect &sourceRect)
  {
    paintOnActive(drawPixmap, targetRect, pixmap, sourceRect);
  }
  void drawPixmap(int x, int y, int w, int h, const QPixmap &pm, int sx, int sy, int sw, int sh)
  {
    paintOnActive(drawPixmap, x, y, w, h, pm, sx, sy, sw, sh);
  }
  void drawPixmap(int x, int y, const QPixmap &pm, int sx, int sy, int sw, int sh)
  {
    paintOnActive(drawPixmap, x, y, pm, sx, sy, sw, sh);
  }
  void drawPixmap(const QPointF &p, const QPixmap &pm, const QRectF &sr) { paintOnActive(drawPixmap, p, pm, sr); }
  void drawPixmap(const QPoint &p, const QPixmap &pm, const QRect &sr) { paintOnActive(drawPixmap, p, pm, sr); }
  void drawPixmap(const QPointF &p, const QPixmap &pm) { paintOnActive(drawPixmap, p, pm); }
  void drawPixmap(const QPoint &p, const QPixmap &pm) { paintOnActive(drawPixmap, p, pm); }
  void drawPixmap(int x, int y, const QPixmap &pm) { paintOnActive(drawPixmap, x, y, pm); }
  void drawPixmap(const QRect &r, const QPixmap &pm) { paintOnActive(drawPixmap, r, pm); }
  void drawPixmap(int x, int y, int w, int h, const QPixmap &pm) { paintOnActive(drawPixmap, x, y, w, h, pm); }

  void drawPixmapFragments(const QPainter::PixmapFragment *fragments, int fragmentCount, const QPixmap &pixmap,
                           QPainter::PixmapFragmentHints hints = QPainter::PixmapFragmentHints())
  {
    paintOnActive(drawPixmapFragments, fragments, fragmentCount, pixmap, hints);
  }

  void drawImage(const QRectF &targetRect, const QImage &image, const QRectF &sourceRect,
                 Qt::ImageConversionFlags flags = Qt::AutoColor)
  {
    paintOnActive(drawImage, targetRect, image, sourceRect, flags);
  }
  void drawImage(const QRect &targetRect, const QImage &image, const QRect &sourceRect,
                 Qt::ImageConversionFlags flags = Qt::AutoColor)
  {
    paintOnActive(drawImage, targetRect, image, sourceRect, flags);
  }
  void drawImage(const QPointF &p, const QImage &image, const QRectF &sr,
                 Qt::ImageConversionFlags flags = Qt::AutoColor)
  {
    paintOnActive(drawImage, p, image, sr, flags);
  }
  void drawImage(const QPoint &p, const QImage &image, const QRect &sr, Qt::ImageConversionFlags flags = Qt::AutoColor)
  {
    paintOnActive(drawImage, p, image, sr, flags);
  }
  void drawImage(const QRectF &r, const QImage &image) { paintOnActive(drawImage, r, image); }
  void drawImage(const QRect &r, const QImage &image) { paintOnActive(drawImage, r, image); }
  void drawImage(const QPointF &p, const QImage &image) { paintOnActive(drawImage, p, image); }
  void drawImage(const QPoint &p, const QImage &image) { paintOnActive(drawImage, p, image); }
  void drawImage(int x, int y, const QImage &image, int sx = 0, int sy = 0, int sw = -1, int sh = -1,
                 Qt::ImageConversionFlags flags = Qt::AutoColor)
  {
    paintOnActive(drawImage, x, y, image, sx, sy, sw, sh, flags);
  }

  void setLayoutDirection(Qt::LayoutDirection direction) { paintOnActive(setLayoutDirection, direction); }

  void drawGlyphRun(const QPointF &position, const QGlyphRun &glyphRun)
  {
    paintOnActive(drawGlyphRun, position, glyphRun);
  }

  void drawStaticText(const QPointF &topLeftPosition, const QStaticText &staticText)
  {
    paintOnActive(drawStaticText, topLeftPosition, staticText);
  }
  void drawStaticText(const QPoint &topLeftPosition, const QStaticText &staticText)
  {
    paintOnActive(drawStaticText, topLeftPosition, staticText);
  }
  void drawStaticText(int left, int top, const QStaticText &staticText)
  {
    paintOnActive(drawStaticText, left, top, staticText);
  }

  void drawText(const QPointF &p, const QString &s) { paintOnActive(drawText, p, s); }
  void drawText(const QPoint &p, const QString &s) { paintOnActive(drawText, p, s); }
  void drawText(int x, int y, const QString &s) { paintOnActive(drawText, x, y, s); }

  void drawText(const QPointF &p, const QString &str, int tf, int justificationPadding)
  {
    paintOnActive(drawText, p, str, tf, justificationPadding);
  }

  void drawText(const QRectF &r, int flags, const QString &text, QRectF *br = nullptr)
  {
    paintOnActive(drawText, r, flags, text, br);
  }
  void drawText(const QRect &r, int flags, const QString &text, QRect *br = nullptr)
  {
    paintOnActive(drawText, r, flags, text, br);
  }
  void drawText(int x, int y, int w, int h, int flags, const QString &text, QRect *br = nullptr)
  {
    paintOnActive(drawText, x, y, w, h, flags, text, br);
  }

  void drawText(const QRectF &r, const QString &text, const QTextOption &o = QTextOption())
  {
    paintOnActive(drawText, r, text, o);
  }

  void drawTextItem(const QPointF &p, const QTextItem &ti) { paintOnActive(drawTextItem, p, ti); }
  void drawTextItem(int x, int y, const QTextItem &ti) { paintOnActive(drawTextItem, x, y, ti); }
  void drawTextItem(const QPoint &p, const QTextItem &ti) { paintOnActive(drawTextItem, p, ti); }

  void fillRect(const QRectF &rect, const QBrush &brush) { paintOnActive(fillRect, rect, brush); }
  void fillRect(int x, int y, int w, int h, const QBrush &brush) { paintOnActive(fillRect, x, y, w, h, brush); }
  void fillRect(const QRect &rect, const QBrush &brush) { paintOnActive(fillRect, rect, brush); }

  void fillRect(const QRectF &rect, const QColor &color) { paintOnActive(fillRect, rect, color); }
  void fillRect(int x, int y, int w, int h, const QColor &color) { paintOnActive(fillRect, x, y, w, h, color); }
  void fillRect(const QRect &rect, const QColor &color) { paintOnActive(fillRect, rect, color); }

  void fillRect(int x, int y, int w, int h, Qt::GlobalColor c) { paintOnActive(fillRect, x, y, w, h, c); }
  void fillRect(const QRect &r, Qt::GlobalColor c) { paintOnActive(fillRect, r, c); }
  void fillRect(const QRectF &r, Qt::GlobalColor c) { paintOnActive(fillRect, r, c); }

  void fillRect(int x, int y, int w, int h, Qt::BrushStyle style) { paintOnActive(fillRect, x, y, w, h, style); }
  void fillRect(const QRect &r, Qt::BrushStyle style) { paintOnActive(fillRect, r, style); }
  void fillRect(const QRectF &r, Qt::BrushStyle style) { paintOnActive(fillRect, r, style); }

#if QT_VERSION >= QT_VERSION_CHECK(5, 12, 0)
  void fillRect(int x, int y, int w, int h, QGradient::Preset preset) { paintOnActive(fillRect, x, y, w, h, preset); }
  void fillRect(const QRect &r, QGradient::Preset preset) { paintOnActive(fillRect, r, preset); }
  void fillRect(const QRectF &r, QGradient::Preset preset) { paintOnActive(fillRect, r, preset); }
#endif

  void eraseRect(const QRectF &rect) { paintOnActive(eraseRect, rect); }
  void eraseRect(int x, int y, int w, int h) { paintOnActive(eraseRect, x, y, w, h); }
  void eraseRect(const QRect &rect) { paintOnActive(eraseRect, rect); }

  void setRenderHint(QPainter::RenderHint hint, bool on = true) { paintOnActive(setRenderHint, hint, on); }
  void setRenderHints(QPainter::RenderHints hints, bool on = true) { paintOnActive(setRenderHints, hints, on); }

  void beginNativePainting() { paintOnActive(beginNativePainting); }
  void endNativePainting() { paintOnActive(endNativePainting); }

#undef paintOnActive

private:
  static bool isMapElementActive(const std::pair<const unsigned int, PainterAndImage> &idToPainterAndImage)
  {
    return idToPainterAndImage.second.active;
  }

  const QPainter &parentPainter_;
  std::map<unsigned int, PainterAndImage> paintersAndImages_;
};


class GroupMask
{
public:
  // Use RGB32 instead of RGB888 to increase performance.
  GroupMask(int width, int height) : maskImage_(width, height, QImage::Format_RGB32), maskPainter_(&maskImage_)
  {
    // `white` or `0xFFFFFFFF` is no valid ID.
    maskImage_.fill(Qt::white);
    maskPainter_.setRenderHint(QPainter::Antialiasing, false);
    maskPainter_.setRenderHint(QPainter::TextAntialiasing, false);
    maskPainter_.setCompositionMode(QPainter::CompositionMode_Source);
    auto font = maskPainter_.font();
    font.setStyleStrategy(QFont::NoAntialias);
    maskPainter_.setFont(font);
  }

  void id(unsigned int id)
  {
    auto pen = maskPainter_.pen();
    pen.setColor(QColor::fromRgb(id));
    maskPainter_.setPen(pen);
    auto brush = maskPainter_.brush();
    brush.setColor(QColor::fromRgb(id));
    maskPainter_.setBrush(brush);
  }

  unsigned int id() const { return maskPainter_.pen().color().rgb() & 0x00FFFFFF; }

  const unsigned int *pixels() const { return reinterpret_cast<const uint32_t *>(this->maskImage_.constBits()); }

  unsigned int operator()(unsigned int x, unsigned int y) const
  {
    // Use `& 0x00FFFFFF` to remove the alpha channel which is always `0xFF` because of the RGB32 format.
    return pixels()[y * maskImage_.width() + x] & 0x00FFFFFF;
  }

  bool hasPixel(unsigned int x, unsigned int y) const { return this->operator()(x, y) != 0; }

  QPainter &painter() { return maskPainter_; }
  const QPainter &painter() const { return maskPainter_; }

  QImage &image() { return maskImage_; }
  const QImage &image() const { return maskImage_; }

  unsigned int width() const { return maskImage_.width(); }

  unsigned int height() const { return maskImage_.height(); }

  unsigned int *toCMask() const
  {
    unsigned int *cmask =
        reinterpret_cast<unsigned int *>(malloc(maskImage_.width() * maskImage_.height() * sizeof(unsigned int)));
    if (cmask == nullptr) throw std::bad_alloc();
    memcpy(cmask, pixels(), maskImage_.width() * maskImage_.height() * sizeof(unsigned int));
    return cmask;
  }

private:
  QImage maskImage_;
  QPainter maskPainter_;
};

class ProxyPainter
{
public:
  explicit ProxyPainter(QPainter &painter)
      : painter_(painter),
#if QT_VERSION >= QT_VERSION_CHECK(5, 6, 0)
        groupMask_(painter.device()->width() / painter.device()->devicePixelRatioF(),
                   painter.device()->height() / painter.device()->devicePixelRatioF()),
#elif QT_VERSION >= QT_VERSION_CHECK(5, 0, 0)
        groupMask_(painter.device()->width() / painter.device()->devicePixelRatio(),
                   painter.device()->height() / painter.device()->devicePixelRatio()),
#else
        groupMask_(painter.device()->width(), painter.device()->height()),
#endif
        maskPainter_(groupMask_.painter()), partialPainters_(painter_)
  {
  }

  explicit ProxyPainter(QPixmap &pixmap)
      : ownedPainter_(new QPainter(&pixmap)), painter_(*ownedPainter_),
#if QT_VERSION >= QT_VERSION_CHECK(5, 6, 0)
        groupMask_(pixmap.width() / pixmap.devicePixelRatioF(), pixmap.height() / pixmap.devicePixelRatioF()),
#elif QT_VERSION >= QT_VERSION_CHECK(5, 0, 0)
        groupMask_(pixmap.width() / pixmap.devicePixelRatio(), pixmap.height() / pixmap.devicePixelRatio()),
#else
        groupMask_(pixmap.width(), pixmap.height()),
#endif
        maskPainter_(groupMask_.painter()), partialPainters_(painter_)
  {
  }

  void beginGroup(unsigned int id)
  {
    groupMask_.id(id);
    groupStack_.push(id);
  }

  void endGroup()
  {
    groupStack_.pop();
    groupMask_.id(groupStack_.empty() ? 0 : groupStack_.top());
  }

  bool hasOpenGroup() const { return !groupStack_.empty(); }

  const GroupMask &groupMask() const { return groupMask_; }

  void beginPartial(unsigned int id) { partialPainters_.startPainting(id); }
  void endPartial(unsigned int id) { partialPainters_.stopPainting(id); }

  std::map<unsigned int, PartialPainters::PaintedRegion> partialCRegions() const
  {
    return partialPainters_.getPaintedCRegions();
  }

  // Wrapped QPainter methods start here:
#define paint(method, ...)                  \
  do                                        \
    {                                       \
      partialPainters_.method(__VA_ARGS__); \
      maskPainter_.method(__VA_ARGS__);     \
      painter_.method(__VA_ARGS__);         \
    }                                       \
  while (0)
#define paint_and_return(method, ...)      \
  do                                       \
    {                                      \
      maskPainter_.method(__VA_ARGS__);    \
      return painter_.method(__VA_ARGS__); \
    }                                      \
  while (0)


  bool begin(QPaintDevice *paintDevice) { paint_and_return(begin, paintDevice); }
  bool end() { paint_and_return(end); }
  bool isActive() const { return painter_.isActive(); }
  QPainter::CompositionMode compositionMode() { return painter_.compositionMode(); }
  void setCompositionMode(QPainter::CompositionMode mode)
  {
    painter_.setCompositionMode(mode);
    partialPainters_.setCompositionMode(mode);
  }

  void setFont(const QFont &f) { paint(setFont, f); }

  QFontMetrics fontMetrics() const { return painter_.fontMetrics(); }
  QFontInfo fontInfo() const { return painter_.fontInfo(); }

  const QPen &pen() const { return painter_.pen(); }
  void setPen(const QColor &color)
  {
    partialPainters_.setPen(color);
    painter_.setPen(color);
    applyPenToMask();
  }
  void setPen(const QPen &pen)
  {
    partialPainters_.setPen(pen);
    painter_.setPen(pen);
    applyPenToMask();
  }
  void setPen(Qt::PenStyle style)
  {
    partialPainters_.setPen(style);
    painter_.setPen(style);
    applyPenToMask();
  }

  const QBrush &brush() const { return painter_.brush(); }
  void setBrush(const QBrush &brush)
  {
    partialPainters_.setBrush(brush);
    painter_.setBrush(brush);
    applyBrushToMask();
  }
  void setBrush(Qt::BrushStyle style)
  {
    partialPainters_.setBrush(style);
    painter_.setBrush(style);
    applyBrushToMask();
  }
  void setBrush(QColor color)
  {
    partialPainters_.setBrush(color);
    painter_.setBrush(color);
    applyBrushToMask();
  }
  void setBrush(Qt::GlobalColor color)
  {
    partialPainters_.setBrush(color);
    painter_.setBrush(color);
    applyBrushToMask();
  }

  Qt::BGMode backgroundMode() const { return painter_.backgroundMode(); }
  void setBackgroundMode(Qt::BGMode mode) { paint(setBackgroundMode, mode); }

  QPoint brushOrigin() const { return painter_.brushOrigin(); }
  void setBrushOrigin(int x, int y) { paint(setBrushOrigin, x, y); }
  void setBrushOrigin(const QPoint &p) { paint(setBrushOrigin, p); }
  void setBrushOrigin(const QPointF &p) { paint(setBrushOrigin, p); }

  const QBrush &background() const { return painter_.background(); }
  void setBackground(const QBrush &bg)
  {
    partialPainters_.setBackground(bg);
    painter_.setBackground(bg);
    maskPainter_.setBackground(createMaskBrush(&bg));
  }

  qreal opacity() const { return painter_.opacity(); }
  void setOpacity(qreal opacity) { paint(setOpacity, opacity); }

  void setClipRect(const QRectF &rect, Qt::ClipOperation op = Qt::ReplaceClip) { paint(setClipRect, rect, op); }
  void setClipRect(const QRect &rect, Qt::ClipOperation op = Qt::ReplaceClip) { paint(setClipRect, rect, op); }
  void setClipRect(int x, int y, int w, int h, Qt::ClipOperation op = Qt::ReplaceClip)
  {
    paint(setClipRect, x, y, w, h, op);
  }

  QRegion clipRegion() const { return painter_.clipRegion(); }
  void setClipRegion(const QRegion &region, Qt::ClipOperation op = Qt::ReplaceClip)
  {
    paint(setClipRegion, region, op);
  }

  QPainterPath clipPath() const { return painter_.clipPath(); }
  void setClipPath(const QPainterPath &path, Qt::ClipOperation op = Qt::ReplaceClip) { paint(setClipPath, path, op); }

  bool hasClipping() const { return painter_.hasClipping(); }
  void setClipping(bool enable) { paint(setClipping, enable); }
  QRectF clipBoundingRect() const { return painter_.clipBoundingRect(); }

  void save() { paint(save); }
  void restore() { paint(restore); }

  const QTransform &transform() const { return painter_.transform(); }
  void setTransform(const QTransform &transform, bool combine = false) { paint(setTransform, transform, combine); }
  void resetTransform() { paint(resetTransform); }

  const QTransform &deviceTransform() const { return painter_.deviceTransform(); }
  const QTransform &worldTransform() const { return painter_.worldTransform(); }
  void setWorldTransform(const QTransform &matrix, bool combine = false) { paint(setWorldTransform, matrix, combine); }

  QTransform combinedTransform() const { return painter_.combinedTransform(); }

  bool worldMatrixEnabled() const { return painter_.worldMatrixEnabled(); }
  void setWorldMatrixEnabled(bool enabled) { paint(setWorldMatrixEnabled, enabled); }

  void scale(qreal sx, qreal sy) { paint(scale, sx, sy); }
  void shear(qreal sh, qreal sv) { paint(shear, sh, sv); }
  void rotate(qreal a) { paint(rotate, a); }

  void translate(const QPointF &offset) { paint(translate, offset); }
  void translate(const QPoint &offset) { paint(translate, offset); }
  void translate(qreal dx, qreal dy) { paint(translate, dx, dy); }

  QRect window() const { return painter_.window(); }
  void setWindow(const QRect &window) { paint(setWindow, window); }
  void setWindow(int x, int y, int w, int h) { paint(setWindow, x, y, w, h); }

  QRect viewport() const { return painter_.viewport(); }
  void setViewport(const QRect &viewport) { paint(setViewport, viewport); }
  void setViewport(int x, int y, int w, int h) { paint(setViewport, x, y, w, h); }

  bool viewTransformEnabled() const { return painter_.viewTransformEnabled(); }
  void setViewTransformEnabled(bool enable) { paint(setViewTransformEnabled, enable); }

  void strokePath(const QPainterPath &path, const QPen &pen)
  {
    partialPainters_.strokePath(path, pen);
    painter_.strokePath(path, pen);
    maskPainter_.strokePath(path, createMaskPen(&pen));
  }
  void fillPath(const QPainterPath &path, const QBrush &brush)
  {
    partialPainters_.fillPath(path, brush);
    painter_.fillPath(path, brush);
    maskPainter_.fillPath(path, createMaskBrush(&brush));
  }
  void drawPath(const QPainterPath &path) { paint(drawPath, path); }

  void drawPoint(const QPointF &pt) { paint(drawPoint, pt); }
  void drawPoint(const QPoint &p) { paint(drawPoint, p); }
  void drawPoint(int x, int y) { paint(drawPoint, x, y); }

  void drawPoints(const QPointF *points, int pointCount) { paint(drawPoints, points, pointCount); }
  void drawPoints(const QPolygonF &points) { paint(drawPoints, points); }
  void drawPoints(const QPoint *points, int pointCount) { paint(drawPoints, points, pointCount); }
  void drawPoints(const QPolygon &points) { paint(drawPoints, points); }

  void drawLine(const QLineF &line) { paint(drawLine, line); }
  void drawLine(const QLine &line) { paint(drawLine, line); }
  void drawLine(int x1, int y1, int x2, int y2) { paint(drawLine, x1, y1, x2, y2); }
  void drawLine(const QPoint &p1, const QPoint &p2) { paint(drawLine, p1, p2); }
  void drawLine(const QPointF &p1, const QPointF &p2) { paint(drawLine, p1, p2); }

  void drawLines(const QLineF *lines, int lineCount) { paint(drawLines, lines, lineCount); }
  void drawLines(const QPointF *pointPairs, int lineCount) { paint(drawLines, pointPairs, lineCount); }
  void drawLines(const QLine *lines, int lineCount) { paint(drawLines, lines, lineCount); }
  void drawLines(const QPoint *pointPairs, int lineCount) { paint(drawLines, pointPairs, lineCount); }
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
  void drawLines(const QList<QLineF> &lines) { paint(drawLines, lines); }
  void drawLines(const QList<QPointF> &pointPairs) { paint(drawLines, pointPairs); }
  void drawLines(const QList<QLine> &lines) { paint(drawLines, lines); }
  void drawLines(const QList<QPoint> &pointPairs) { paint(drawLines, pointPairs); }
#endif

  void drawRect(const QRectF &rect) { paint(drawRect, rect); }
  void drawRect(int x1, int y1, int w, int h) { paint(drawRect, x1, y1, w, h); }
  void drawRect(const QRect &rect) { paint(drawRect, rect); }

  void drawRects(const QRectF *rects, int rectCount) { paint(drawRects, rects, rectCount); }
  void drawRects(const QRect *rects, int rectCount) { paint(drawRects, rects, rectCount); }
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
  void drawRects(const QList<QRectF> &rectangles) { paint(drawRects, rectangles); }
  void drawRects(const QList<QRect> &rectangles) { paint(drawRects, rectangles); }
#endif

  void drawEllipse(const QRectF &r) { paint(drawEllipse, r); }
  void drawEllipse(const QRect &r) { paint(drawEllipse, r); }
  void drawEllipse(int x, int y, int w, int h) { paint(drawEllipse, x, y, w, h); }

  void drawEllipse(const QPointF &center, qreal rx, qreal ry) { paint(drawEllipse, center, rx, ry); }
  void drawEllipse(const QPoint &center, int rx, int ry) { paint(drawEllipse, center, rx, ry); }

  void drawPolyline(const QPointF *points, int pointCount) { paint(drawPolyline, points, pointCount); }
  void drawPolyline(const QPolygonF &polyline) { paint(drawPolyline, polyline); }
  void drawPolyline(const QPoint *points, int pointCount) { paint(drawPolyline, points, pointCount); }
  void drawPolyline(const QPolygon &polygon) { paint(drawPolyline, polygon); }

  void drawPolygon(const QPointF *points, int pointCount, Qt::FillRule fillRule = Qt::OddEvenFill)
  {
    paint(drawPolygon, points, pointCount, fillRule);
  }
  void drawPolygon(const QPolygonF &polygon, Qt::FillRule fillRule = Qt::OddEvenFill)
  {
    paint(drawPolygon, polygon, fillRule);
  }
  void drawPolygon(const QPoint *points, int pointCount, Qt::FillRule fillRule = Qt::OddEvenFill)
  {
    paint(drawPolygon, points, pointCount, fillRule);
  }
  void drawPolygon(const QPolygon &polygon, Qt::FillRule fillRule = Qt::OddEvenFill)
  {
    paint(drawPolygon, polygon, fillRule);
  }

  void drawConvexPolygon(const QPointF *points, int pointCount) { paint(drawConvexPolygon, points, pointCount); }
  void drawConvexPolygon(const QPolygonF &polygon) { paint(drawConvexPolygon, polygon); }
  void drawConvexPolygon(const QPoint *points, int pointCount) { paint(drawConvexPolygon, points, pointCount); }
  void drawConvexPolygon(const QPolygon &polygon) { paint(drawConvexPolygon, polygon); }

  void drawArc(const QRectF &rect, int a, int alen) { paint(drawArc, rect, a, alen); }
  void drawArc(const QRect &rect, int a, int alen) { paint(drawArc, rect, a, alen); }
  void drawArc(int x, int y, int w, int h, int a, int alen) { paint(drawArc, x, y, w, h, a, alen); }

  void drawPie(const QRectF &rect, int a, int alen) { paint(drawPie, rect, a, alen); }
  void drawPie(int x, int y, int w, int h, int a, int alen) { paint(drawPie, x, y, w, h, a, alen); }
  void drawPie(const QRect &rect, int a, int alen) { paint(drawPie, rect, a, alen); }

  void drawChord(const QRectF &rect, int a, int alen) { paint(drawChord, rect, a, alen); }
  void drawChord(int x, int y, int w, int h, int a, int alen) { paint(drawChord, x, y, w, h, a, alen); }
  void drawChord(const QRect &rect, int a, int alen) { paint(drawChord, rect, a, alen); }

  void drawRoundedRect(const QRectF &rect, qreal xRadius, qreal yRadius, Qt::SizeMode mode = Qt::AbsoluteSize)
  {
    paint(drawRoundedRect, rect, xRadius, yRadius, mode);
  }
  void drawRoundedRect(int x, int y, int w, int h, qreal xRadius, qreal yRadius, Qt::SizeMode mode = Qt::AbsoluteSize)
  {
    paint(drawRoundedRect, x, y, w, h, xRadius, yRadius, mode);
  }
  void drawRoundedRect(const QRect &rect, qreal xRadius, qreal yRadius, Qt::SizeMode mode = Qt::AbsoluteSize)
  {
    paint(drawRoundedRect, rect, xRadius, yRadius, mode);
  }

  void drawTiledPixmap(const QRectF &rect, const QPixmap &pm, const QPointF &offset = QPointF())
  {
    partialPainters_.drawTiledPixmap(rect, pm, offset);
    painter_.drawTiledPixmap(rect, pm, offset);
    maskPainter_.fillRect(rect, maskPainter_.brush().color());
  }
  void drawTiledPixmap(int x, int y, int w, int h, const QPixmap &pixmap, int sx = 0, int sy = 0)
  {
    partialPainters_.drawTiledPixmap(x, y, w, h, pixmap, sx, sy);
    painter_.drawTiledPixmap(x, y, w, h, pixmap, sx, sy);
    maskPainter_.fillRect(x, y, w, h, maskPainter_.brush().color());
  }
  void drawTiledPixmap(const QRect &rect, const QPixmap &pixmap, const QPoint &offset = QPoint())
  {
    partialPainters_.drawTiledPixmap(rect, pixmap, offset);
    painter_.drawTiledPixmap(rect, pixmap, offset);
    maskPainter_.fillRect(rect, maskPainter_.brush().color());
  }
  void drawPicture(const QPointF &p, const QPicture &picture)
  {
    partialPainters_.drawPicture(p, picture);
    painter_.drawPicture(p, picture);
    maskPainter_.fillRect(QRectF(p, picture.boundingRect().size()), maskPainter_.brush().color());
  }
  void drawPicture(int x, int y, const QPicture &picture)
  {
    partialPainters_.drawPicture(x, y, picture);
    painter_.drawPicture(x, y, picture);
    maskPainter_.fillRect(QRect(QPoint(x, y), picture.boundingRect().size()), maskPainter_.brush().color());
  }
  void drawPicture(const QPoint &p, const QPicture &picture)
  {
    partialPainters_.drawPicture(p, picture);
    painter_.drawPicture(p, picture);
    maskPainter_.fillRect(QRect(p, picture.boundingRect().size()), maskPainter_.brush().color());
  }

  void drawPixmap(const QRectF &targetRect, const QPixmap &pixmap, const QRectF &sourceRect)
  {
    partialPainters_.drawPixmap(targetRect, pixmap, sourceRect);
    painter_.drawPixmap(targetRect, pixmap, sourceRect);
    drawMaskForPixmap(targetRect, pixmap, sourceRect);
  }
  void drawPixmap(const QRect &targetRect, const QPixmap &pixmap, const QRect &sourceRect)
  {
    partialPainters_.drawPixmap(targetRect, pixmap, sourceRect);
    painter_.drawPixmap(targetRect, pixmap, sourceRect);
    drawMaskForPixmap(targetRect, pixmap, sourceRect);
  }
  void drawPixmap(int x, int y, int w, int h, const QPixmap &pm, int sx, int sy, int sw, int sh)
  {
    partialPainters_.drawPixmap(x, y, w, h, pm, sx, sy, sw, sh);
    painter_.drawPixmap(x, y, w, h, pm, sx, sy, sw, sh);
    drawMaskForPixmap(QRect(x, y, w, h), pm, QRect(sx, sy, sw, sh));
  }
  void drawPixmap(int x, int y, const QPixmap &pm, int sx, int sy, int sw, int sh)
  {
    partialPainters_.drawPixmap(x, y, pm, sx, sy, sw, sh);
    painter_.drawPixmap(x, y, pm, sx, sy, sw, sh);
    drawMaskForPixmap(QRect(QPoint(x, y), pm.size()), pm, QRect(sx, sy, sw, sh));
  }
  void drawPixmap(const QPointF &p, const QPixmap &pm, const QRectF &sr)
  {
    partialPainters_.drawPixmap(p, pm, sr);
    painter_.drawPixmap(p, pm, sr);
    drawMaskForPixmap(QRectF(p, pm.size()), pm, sr);
  }
  void drawPixmap(const QPoint &p, const QPixmap &pm, const QRect &sr)
  {
    partialPainters_.drawPixmap(p, pm, sr);
    painter_.drawPixmap(p, pm, sr);
    drawMaskForPixmap(QRect(p, pm.size()), pm, sr);
  }
  void drawPixmap(const QPointF &p, const QPixmap &pm)
  {
    partialPainters_.drawPixmap(p, pm);
    painter_.drawPixmap(p, pm);
    drawMaskForPixmap(QRectF(p, pm.size()), pm);
  }
  void drawPixmap(const QPoint &p, const QPixmap &pm)
  {
    partialPainters_.drawPixmap(p, pm);
    painter_.drawPixmap(p, pm);
    drawMaskForPixmap(QRect(p, pm.size()), pm);
  }
  void drawPixmap(int x, int y, const QPixmap &pm)
  {
    partialPainters_.drawPixmap(x, y, pm);
    painter_.drawPixmap(x, y, pm);
    drawMaskForPixmap(QRect(QPoint(x, y), pm.size()), pm);
  }
  void drawPixmap(const QRect &r, const QPixmap &pm)
  {
    partialPainters_.drawPixmap(r, pm);
    painter_.drawPixmap(r, pm);
    drawMaskForPixmap(r, pm);
  }
  void drawPixmap(int x, int y, int w, int h, const QPixmap &pm)
  {
    partialPainters_.drawPixmap(x, y, w, h, pm);
    painter_.drawPixmap(x, y, w, h, pm);
    drawMaskForPixmap(QRect(x, y, w, h), pm);
  }

  void drawPixmapFragments(const QPainter::PixmapFragment *fragments, int fragmentCount, const QPixmap &pixmap,
                           QPainter::PixmapFragmentHints hints = QPainter::PixmapFragmentHints())
  {
    partialPainters_.drawPixmapFragments(fragments, fragmentCount, pixmap, hints);
    painter_.drawPixmapFragments(fragments, fragmentCount, pixmap, hints);
    // TODO: Use rectangles to represent fragments in the mask image
  }

  void drawImage(const QRectF &targetRect, const QImage &image, const QRectF &sourceRect,
                 Qt::ImageConversionFlags flags = Qt::AutoColor)
  {
    partialPainters_.drawImage(targetRect, image, sourceRect, flags);
    painter_.drawImage(targetRect, image, sourceRect, flags);
    drawMaskForImage(targetRect, image, sourceRect);
  }
  void drawImage(const QRect &targetRect, const QImage &image, const QRect &sourceRect,
                 Qt::ImageConversionFlags flags = Qt::AutoColor)
  {
    partialPainters_.drawImage(targetRect, image, sourceRect, flags);
    painter_.drawImage(targetRect, image, sourceRect, flags);
    drawMaskForImage(targetRect, image, sourceRect);
  }
  void drawImage(const QPointF &p, const QImage &image, const QRectF &sr,
                 Qt::ImageConversionFlags flags = Qt::AutoColor)
  {
    partialPainters_.drawImage(p, image, sr, flags);
    painter_.drawImage(p, image, sr, flags);
    drawMaskForImage(QRectF(p, image.size()), image, sr);
  }
  void drawImage(const QPoint &p, const QImage &image, const QRect &sr, Qt::ImageConversionFlags flags = Qt::AutoColor)
  {
    partialPainters_.drawImage(p, image, sr, flags);
    painter_.drawImage(p, image, sr, flags);
    drawMaskForImage(QRectF(p, image.size()), image, sr);
  }
  void drawImage(const QRectF &r, const QImage &image)
  {
    partialPainters_.drawImage(r, image);
    painter_.drawImage(r, image);
    drawMaskForImage(r, image);
  }
  void drawImage(const QRect &r, const QImage &image)
  {
    partialPainters_.drawImage(r, image);
    painter_.drawImage(r, image);
    drawMaskForImage(r, image);
  }
  void drawImage(const QPointF &p, const QImage &image)
  {
    partialPainters_.drawImage(p, image);
    painter_.drawImage(p, image);
    drawMaskForImage(QRectF(p, image.size()), image);
  }
  void drawImage(const QPoint &p, const QImage &image)
  {
    partialPainters_.drawImage(p, image);
    painter_.drawImage(p, image);
    drawMaskForImage(QRectF(p, image.size()), image);
  }
  void drawImage(int x, int y, const QImage &image, int sx = 0, int sy = 0, int sw = -1, int sh = -1,
                 Qt::ImageConversionFlags flags = Qt::AutoColor)
  {
    partialPainters_.drawImage(x, y, image, sx, sy, sw, sh, flags);
    painter_.drawImage(x, y, image, sx, sy, sw, sh, flags);
    drawMaskForImage(QRect(QPoint(x, y), image.size()), image);
  }

  void setLayoutDirection(Qt::LayoutDirection direction) { paint(setLayoutDirection, direction); }

  void drawGlyphRun(const QPointF &position, const QGlyphRun &glyphRun) { paint(drawGlyphRun, position, glyphRun); }

  void drawStaticText(const QPointF &topLeftPosition, const QStaticText &staticText)
  {
    paint(drawStaticText, topLeftPosition, staticText);
  }
  void drawStaticText(const QPoint &topLeftPosition, const QStaticText &staticText)
  {
    paint(drawStaticText, topLeftPosition, staticText);
  }
  void drawStaticText(int left, int top, const QStaticText &staticText)
  {
    paint(drawStaticText, left, top, staticText);
  }

  void drawText(const QPointF &p, const QString &s) { paint(drawText, p, s); }
  void drawText(const QPoint &p, const QString &s) { paint(drawText, p, s); }
  void drawText(int x, int y, const QString &s) { paint(drawText, x, y, s); }

  void drawText(const QPointF &p, const QString &str, int tf, int justificationPadding)
  {
    paint(drawText, p, str, tf, justificationPadding);
  }

  void drawText(const QRectF &r, int flags, const QString &text, QRectF *br = nullptr)
  {
    paint(drawText, r, flags, text, br);
  }
  void drawText(const QRect &r, int flags, const QString &text, QRect *br = nullptr)
  {
    paint(drawText, r, flags, text, br);
  }
  void drawText(int x, int y, int w, int h, int flags, const QString &text, QRect *br = nullptr)
  {
    paint(drawText, x, y, w, h, flags, text, br);
  }

  void drawText(const QRectF &r, const QString &text, const QTextOption &o = QTextOption())
  {
    paint(drawText, r, text, o);
  }

  QRectF boundingRect(const QRectF &rect, int flags, const QString &text)
  {
    paint_and_return(boundingRect, rect, flags, text);
  }
  QRect boundingRect(const QRect &rect, int flags, const QString &text)
  {
    paint_and_return(boundingRect, rect, flags, text);
  }
  QRect boundingRect(int x, int y, int w, int h, int flags, const QString &text)
  {
    paint_and_return(boundingRect, x, y, w, h, flags, text);
  }

  QRectF boundingRect(const QRectF &rect, const QString &text, const QTextOption &o = QTextOption())
  {
    paint_and_return(boundingRect, rect, text, o);
  }

  void drawTextItem(const QPointF &p, const QTextItem &ti) { paint(drawTextItem, p, ti); }
  void drawTextItem(int x, int y, const QTextItem &ti) { paint(drawTextItem, x, y, ti); }
  void drawTextItem(const QPoint &p, const QTextItem &ti) { paint(drawTextItem, p, ti); }

  void fillRect(const QRectF &rect, const QBrush &brush)
  {
    partialPainters_.fillRect(rect, brush);
    painter_.fillRect(rect, brush);
    maskPainter_.fillRect(rect, createMaskBrush(&brush));
  }
  void fillRect(int x, int y, int w, int h, const QBrush &brush)
  {
    partialPainters_.fillRect(x, y, w, h, brush);
    painter_.fillRect(x, y, w, h, brush);
    maskPainter_.fillRect(x, y, w, h, createMaskBrush(&brush));
  }
  void fillRect(const QRect &rect, const QBrush &brush)
  {
    partialPainters_.fillRect(rect, brush);
    painter_.fillRect(rect, brush);
    maskPainter_.fillRect(rect, createMaskBrush(&brush));
  }

  void fillRect(const QRectF &rect, const QColor &color)
  {
    partialPainters_.fillRect(rect, color);
    painter_.fillRect(rect, color);
    maskPainter_.fillRect(rect, maskPainter_.brush().color());
  }
  void fillRect(int x, int y, int w, int h, const QColor &color)
  {
    partialPainters_.fillRect(x, y, w, h, color);
    painter_.fillRect(x, y, w, h, color);
    maskPainter_.fillRect(x, y, w, h, maskPainter_.brush().color());
  }
  void fillRect(const QRect &rect, const QColor &color)
  {
    partialPainters_.fillRect(rect, color);
    painter_.fillRect(rect, color);
    maskPainter_.fillRect(rect, maskPainter_.brush().color());
  }

  void fillRect(int x, int y, int w, int h, Qt::GlobalColor c)
  {
    partialPainters_.fillRect(x, y, w, h, c);
    painter_.fillRect(x, y, w, h, c);
    maskPainter_.fillRect(x, y, w, h, maskPainter_.brush().color());
  }
  void fillRect(const QRect &r, Qt::GlobalColor c)
  {
    partialPainters_.fillRect(r, c);
    painter_.fillRect(r, c);
    maskPainter_.fillRect(r, maskPainter_.brush().color());
  }
  void fillRect(const QRectF &r, Qt::GlobalColor c)
  {
    partialPainters_.fillRect(r, c);
    painter_.fillRect(r, c);
    maskPainter_.fillRect(r, maskPainter_.brush().color());
  }

  void fillRect(int x, int y, int w, int h, Qt::BrushStyle style) { paint(fillRect, x, y, w, h, style); }
  void fillRect(const QRect &r, Qt::BrushStyle style) { paint(fillRect, r, style); }
  void fillRect(const QRectF &r, Qt::BrushStyle style) { paint(fillRect, r, style); }

#if QT_VERSION >= QT_VERSION_CHECK(5, 12, 0)
  void fillRect(int x, int y, int w, int h, QGradient::Preset preset)
  {
    partialPainters_.fillRect(x, y, w, h, preset);
    painter_.fillRect(x, y, w, h, preset);
    maskPainter_.fillRect(x, y, w, h, maskPainter_.brush().color());
  }
  void fillRect(const QRect &r, QGradient::Preset preset)
  {
    partialPainters_.fillRect(r, preset);
    painter_.fillRect(r, preset);
    maskPainter_.fillRect(r, maskPainter_.brush().color());
  }
  void fillRect(const QRectF &r, QGradient::Preset preset)
  {
    partialPainters_.fillRect(r, preset);
    painter_.fillRect(r, preset);
    maskPainter_.fillRect(r, maskPainter_.brush().color());
  }
#endif

  void eraseRect(const QRectF &rect) { paint(eraseRect, rect); }
  void eraseRect(int x, int y, int w, int h) { paint(eraseRect, x, y, w, h); }
  void eraseRect(const QRect &rect) { paint(eraseRect, rect); }

  void setRenderHint(QPainter::RenderHint hint, bool on = true)
  {
    partialPainters_.setRenderHint(hint, on);
    painter_.setRenderHint(hint, on);
  }
  void setRenderHints(QPainter::RenderHints hints, bool on = true)
  {
    partialPainters_.setRenderHints(hints, on);
    painter_.setRenderHints(hints, on);
  }
  bool testRenderHint(QPainter::RenderHint hint) const { paint_and_return(testRenderHint, hint); }

  void beginNativePainting() { paint(beginNativePainting, ); }
  void endNativePainting() { paint(endNativePainting, ); }

  Qt::LayoutDirection layoutDirection() const { return painter_.layoutDirection(); }
  QPainter::RenderHints renderHints() const { return painter_.renderHints(); }
  QPaintEngine *paintEngine() const { return painter_.paintEngine(); }

#undef paint
#undef paint_and_return

protected:
  QBrush createMaskBrush(const QBrush *brush = nullptr)
  {
    // TODO: Use a reference instead of pointer for brush
    if (!brush) brush = &painter_.brush();
    auto brush_ = QBrush(*brush);
    brush_.setColor(maskPainter_.brush().color());
    return brush_;
  }

  void applyBrushToMask() { maskPainter_.setBrush(createMaskBrush()); }

  QPen createMaskPen(const QPen *pen = nullptr)
  {
    if (!pen) pen = &painter_.pen();
    auto pen_ = QPen(*pen);
    pen_.setColor(maskPainter_.pen().color());
    if (pen_.widthF() < 1.0) pen_.setWidth(1);
    return pen_;
  }

  void applyPenToMask() { maskPainter_.setPen(createMaskPen()); }

  void drawMaskForImage(const QRectF &targetRect, const QImage &image)
  {
    drawMaskForImage(targetRect, image, QRectF(0, 0, image.width(), image.height()));
  }

  void drawMaskForImage(const QRectF &targetRect, const QImage &image, const QRectF &sourceRect)
  {
#if QT_VERSION >= QT_VERSION_CHECK(5, 6, 0)
    double imageDevicePixelRatio = image.devicePixelRatioF();
#elif QT_VERSION >= QT_VERSION_CHECK(5, 1, 0)
    double imageDevicePixelRatio = image.devicePixelRatio();
#else
    double imageDevicePixelRatio = 1.0;
#endif
    QTransform transform;
    transform.translate(targetRect.x(), targetRect.y());
    transform.scale((double)targetRect.width() / image.width() / imageDevicePixelRatio,
                    (double)targetRect.height() / image.height() / imageDevicePixelRatio);
    transform.scale((double)image.width() / sourceRect.width(), (double)image.height() / sourceRect.height());
    transform.translate(-sourceRect.x(), -sourceRect.y());
    transform *= maskPainter_.transform();
    transform = transform.inverted();
    auto transformedTargetRect = maskPainter_.transform().mapRect(targetRect);
    auto &groupImage = groupMask_.image();
    for (int j = transformedTargetRect.top(); j <= transformedTargetRect.bottom(); ++j)
      {
        for (int i = transformedTargetRect.left(); i <= transformedTargetRect.right(); ++i)
          {
            qreal x, y;
            transform.map(i, j, &x, &y);
            if (x < sourceRect.left() || x >= sourceRect.right() || y < sourceRect.top() || y >= sourceRect.bottom())
              continue;
            if ((image.pixel((int)x, (int)y) & 0xFF000000) == 0) continue;
            groupImage.setPixel(i, j, maskPainter_.brush().color().rgb());
          }
      }
  }

  void drawMaskForPixmap(const QRectF &targetRect, const QPixmap &pixmap)
  {
    drawMaskForPixmap(targetRect, pixmap, QRectF(0, 0, pixmap.width(), pixmap.height()));
  }

  void drawMaskForPixmap(const QRectF &targetRect, const QPixmap &pixmap, const QRectF &sourceRect)
  {
    drawMaskForImage(targetRect, pixmap.toImage(), sourceRect);
  }

private:
  std::unique_ptr<QPainter> ownedPainter_;
  QPainter &painter_;
  GroupMask groupMask_;
  QPainter &maskPainter_;
  std::stack<unsigned int> groupStack_;
  PartialPainters partialPainters_;
};

static gks_state_list_t gkss_, *gkss = &gkss_;

static double a[MAX_TNR], b[MAX_TNR], c[MAX_TNR], d[MAX_TNR];

struct bounding_struct
{
  double x_min, x_max;
  double y_min, y_max;
  void (*fun_call)(int, double, double, double, double);
  int item_id;
};

typedef struct ws_state_list_t
{
  gks_display_list_t dl;
  QWidget *widget;
  QPixmap *pixmap;
  QPixmap *bg;
  QPixmap *selection;
  std::unique_ptr<ProxyPainter> painter;
  int state, wtype;
  int device_dpi_x, device_dpi_y;
  bool has_user_defined_device_pixel_ratio;
  double device_pixel_ratio;
  double mwidth, mheight;
  int width, height;
  double a, b, c, d;
  double window[4], viewport[4];
  double nominal_size;
  QRectF rect[MAX_TNR];
  QColor rgb[MAX_COLOR + 1];
  int transparency;
  QPolygonF *points;
  int npoints, max_points;
  QPolygonF *polygon;
  int max_polygon;
  QFont *font;
  int family, capheight;
  double alpha, angle;
  QPixmap *pattern[PATTERNS];
  int pcolor[PATTERNS];
  bool empty;
  bool prevent_resize_by_dl;
  bool window_stays_on_top;
  bool interp_was_called;

  void (*memory_plugin)(int, int, int, int, int *, int, double *, int, double *, int, char *, void **);
  bool memory_plugin_initialised;
  int memory_plugin_wstype;
  void *memory_plugin_ws_state_list;
  int *memory_plugin_mem_ptr;
  char *memory_plugin_mem_path;
  std::stack<bounding_struct> bounding_stack;
  void (*mask_callback)(unsigned int, unsigned int, unsigned int *);
  void (*partial_drawing_callback)(int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int *);
} ws_state_list;

static ws_state_list p_, *p = &p_;

static int fontfile = 0;

static const char *fonts[] = {
    "Times",          "Arial",       "Courier", "Open Symbol", "Bookman Old Style", "Century Schoolbook",
    "Century Gothic", "Book Antiqua"};

static double capheights[29] = {0.662, 0.660, 0.681, 0.662, 0.729, 0.729, 0.729, 0.729, 0.583, 0.583,
                                0.583, 0.583, 0.667, 0.681, 0.681, 0.681, 0.681, 0.722, 0.722, 0.722,
                                0.722, 0.739, 0.739, 0.739, 0.739, 0.694, 0.693, 0.683, 0.683};

static int map[32] = {22, 9,  5, 14, 18, 26, 13, 1, 24, 11, 7, 16, 20, 28, 13, 3,
                      23, 10, 6, 15, 19, 27, 13, 2, 25, 12, 8, 17, 21, 29, 13, 4};

int symbol2utf[256] = {
    0,    1,    2,    3,    4,    5,    6,    7,    8,    9,    10,   11,   12,   13,    14,   15,   16,    17,   18,
    18,   20,   21,   22,   23,   24,   25,   26,   27,   28,   29,   30,   31,   32,    33,   8704, 35,    8707, 37,
    38,   8715, 40,   41,   42,   43,   44,   45,   46,   47,   48,   49,   50,   51,    52,   53,   54,    55,   56,
    57,   58,   59,   60,   61,   62,   63,   8773, 913,  914,  935,  916,  917,  934,   915,  919,  921,   977,  922,
    923,  924,  925,  927,  928,  920,  929,  931,  932,  933,  962,  937,  926,  936,   918,  91,   8756,  93,   8869,
    95,   8254, 945,  946,  967,  948,  949,  966,  947,  951,  953,  981,  954,  955,   956,  957,  959,   960,  952,
    961,  963,  964,  965,  982,  969,  958,  968,  950,  123,  124,  125,  126,  127,   128,  129,  130,   131,  132,
    133,  134,  135,  136,  137,  138,  139,  140,  141,  142,  143,  144,  145,  146,   147,  148,  149,   150,  151,
    152,  153,  154,  155,  156,  157,  158,  159,  160,  978,  8242, 8804, 8260, 8734,  402,  9827, 9830,  9829, 9824,
    8596, 8592, 8593, 8594, 8595, 176,  177,  8243, 8805, 215,  8733, 8706, 8226, 247,   8800, 8801, 8776,  8230, 9116,
    9135, 8629, 8501, 8465, 8476, 8472, 8855, 8853, 8709, 8745, 8746, 8835, 8839, 8836,  8834, 8838, 8712,  8713, 8736,
    8711, 174,  169,  8482, 8719, 8730, 183,  172,  8743, 8744, 8660, 8656, 8657, 8658,  8659, 9674, 12296, 174,  169,
    8482, 8721, 9115, 9116, 9117, 9121, 9116, 9123, 9127, 9128, 9129, 9116, 240,  12297, 8747, 9127, 9116,  9133, 9131,
    9130, 9120, 9124, 9130, 9126, 9131, 9132, 9133, 255};

static double xfac[4] = {0, 0, -0.5, -1};

static double yfac[6] = {0, -1.2, -1, -0.5, 0, 0.2};

static int predef_font[] = {1, 1, 1, -2, -3, -4};

static int predef_prec[] = {0, 1, 2, 2, 2, 2};

static int predef_ints[] = {0, 1, 3, 3, 3};

static int predef_styli[] = {1, 1, 1, 2, 3};

static void set_norm_xform(int tnr, double *wn, double *vp)
{
  double xp1, yp1, xp2, yp2;

  a[tnr] = (vp[1] - vp[0]) / (wn[1] - wn[0]);
  b[tnr] = vp[0] - wn[0] * a[tnr];
  c[tnr] = (vp[3] - vp[2]) / (wn[3] - wn[2]);
  d[tnr] = vp[2] - wn[2] * c[tnr];

  NDC_to_DC(vp[0], vp[3], xp1, yp1);
  NDC_to_DC(vp[1], vp[2], xp2, yp2);

  p->rect[tnr].setCoords(xp1, yp1, xp2, yp2);
}

static void init_norm_xform(void)
{
  int tnr;

  for (tnr = 0; tnr < MAX_TNR; tnr++) set_norm_xform(tnr, gkss->window[tnr], gkss->viewport[tnr]);
}

static void resize_window(void)
{
  p->mwidth = p->viewport[1] - p->viewport[0];
  p->width = nint(p->device_dpi_x * p->mwidth / 0.0254);
  if (p->width < 2)
    {
      p->width = 2;
      p->mwidth = (double)p->width / p->device_dpi_x * 0.0254;
    }

  p->mheight = p->viewport[3] - p->viewport[2];
  p->height = nint(p->device_dpi_y * p->mheight / 0.0254);
  if (p->height < 2)
    {
      p->height = 2;
      p->mheight = (double)p->height / p->device_dpi_y * 0.0254;
    }
  p->nominal_size = min(p->width, p->height) / 500.0;
  if (gkss->nominal_size > 0) p->nominal_size *= gkss->nominal_size;

#ifndef QT_PLUGIN_USED_AS_PLUGIN_CODE
  if (p->pixmap)
    {
      if (fabs(p->width * p->device_pixel_ratio - p->pixmap->size().width()) > FEPS ||
          fabs(p->height * p->device_pixel_ratio - p->pixmap->size().height()) > FEPS)
        {
          p->painter.release();
          delete p->pixmap;

          p->pixmap = new QPixmap(p->width * p->device_pixel_ratio, p->height * p->device_pixel_ratio);
#if QT_VERSION >= QT_VERSION_CHECK(5, 1, 0)
          p->pixmap->setDevicePixelRatio(p->device_pixel_ratio);
#endif
          p->pixmap->fill(Qt::white);
          if (p->bg)
            {
              delete p->bg;
              p->bg = new QPixmap(*p->pixmap);
            }

          p->painter = std::unique_ptr<ProxyPainter>(new ProxyPainter(*p->pixmap));
          p->painter->setClipRect(0, 0, p->width, p->height);
        }
    }
#endif
}

static void set_xform(void)
{
  double ratio, w, h, x, y;

  ratio = (p->window[1] - p->window[0]) / (p->window[3] - p->window[2]) * (1.0 * p->device_dpi_x / p->device_dpi_y);

  if (p->width > p->height * ratio)
    {
      w = p->height * ratio;
      h = p->height;
      x = 0.5 * (p->width - w);
      y = h;
    }
  else
    {
      w = p->width;
      h = p->width / ratio;
      x = 0;
      y = h + 0.5 * (p->height - h);
    }

  p->a = w / (p->window[1] - p->window[0]);
  p->b = x - p->window[0] * p->a;
  p->c = h / (p->window[2] - p->window[3]);
  p->d = y + p->window[2] * p->c;

  p->nominal_size = min(p->width, p->height) / 500.0;
  if (gkss->nominal_size > 0) p->nominal_size *= gkss->nominal_size;
}

static void seg_xform(double *x, double *y)
{
  double xx;

  xx = *x * gkss->mat[0][0] + *y * gkss->mat[0][1] + gkss->mat[2][0];
  *y = *x * gkss->mat[1][0] + *y * gkss->mat[1][1] + gkss->mat[2][1];
  *x = xx;
}

static void seg_xform_rel(double *x, double *y)
{
  double xx;

  xx = *x * gkss->mat[0][0] + *y * gkss->mat[0][1];
  *y = *x * gkss->mat[1][0] + *y * gkss->mat[1][1];
  *x = xx;
}

static void set_clip_rect(int tnr)
{
  if (gkss->clip_tnr != 0)
    tnr = gkss->clip_tnr;
  else if (gkss->clip == GKS_K_NOCLIP)
    tnr = 0;

  if (gkss->clip_region == GKS_K_REGION_ELLIPSE && tnr != 0)
    {
      if (gkss->clip_start_angle > 0 || gkss->clip_end_angle < 360)
        {
          QPainterPath path;
          path.moveTo(p->rect[tnr].center());
          path.arcTo(p->rect[tnr].toRect(), gkss->clip_start_angle, gkss->clip_end_angle - gkss->clip_start_angle);
          p->painter->setClipPath(path);
        }
      else
        p->painter->setClipRegion(QRegion(p->rect[tnr].toRect(), QRegion::Ellipse));
    }
  else
    {
      p->painter->setClipRect(p->rect[tnr]);
    }
}

static void set_color_rep(int color, double red, double green, double blue)
{
  int i;

  if (color >= 0 && color < MAX_COLOR)
    {
      p->rgb[color].setRgb(nint(red * 255), nint(green * 255), nint(blue * 255));
      for (i = 0; i < PATTERNS; i++)
        {
          if (p->pcolor[i] == color) p->pcolor[i] = -1;
        }
    }
}

static void init_colors(void)
{
  int color;
  double red, green, blue;

  for (color = 0; color < MAX_COLOR; color++)
    {
      gks_inq_rgb(color, &red, &green, &blue);
      set_color_rep(color, red, green, blue);
    }
}

static void set_color(int color)
{
  QColor transparent_color(p->rgb[color]);
  transparent_color.setAlpha(p->transparency);
  p->painter->setPen(transparent_color);
  p->painter->setBrush(transparent_color);
}

static QPixmap *create_pattern(int pattern, int color)
{
  int parray[33];
  int i, j;
  QPixmap *pm;

  gks_inq_pattern_array(pattern, parray);

  QImage img(8, 8, QImage::Format_Mono);
  img.setColor(0, qRgb(255, 255, 255));
  img.setColor(1, p->rgb[color].rgb());
  for (i = 0; i < 8; i++)
    for (j = 0; j < 8; j++) img.setPixel(i, j, (parray[(j % parray[0]) + 1] >> i) & 0x01 ? 0 : 1);

  pm = new QPixmap(8, 8);
  QPixmap tmp = QPixmap::fromImage(img);
  *pm = tmp;

  return pm;
}

static void line_routine(int n, double *px, double *py, int linetype, int tnr)
{
  double x, y, x0, y0, xi, yi, xim1, yim1;
  int i;

  WC_to_NDC(px[0], py[0], tnr, x, y);
  seg_xform(&x, &y);
  NDC_to_DC(x, y, x0, y0);

  p->npoints = 0;
  (*p->points)[p->npoints++] = QPointF(x0, y0);

  xim1 = x0;
  yim1 = y0;
  for (i = 1; i < n; i++)
    {
      WC_to_NDC(px[i], py[i], tnr, x, y);
      seg_xform(&x, &y);
      NDC_to_DC(x, y, xi, yi);

      if (i == 1 || xi != xim1 || yi != yim1)
        {
          (*p->points)[p->npoints++] = QPointF(xi, yi);
          xim1 = xi;
          yim1 = yi;
        }
    }
  if (linetype == 0) (*p->points)[p->npoints++] = QPointF(x0, y0);

  if (p->npoints > MAX_POINTS_PERFORMANCE_THRESHOLD)
    {
      /*
       * Qt drawPolyline() is slow on calculating line joins for a large list of points.
       * Drawing each of the line segments using drawLine() does remove the problem.
       */
      for (i = 1; i < p->npoints; i++) p->painter->drawLine((*p->points)[i - 1], (*p->points)[i]);
    }
  else
    {
      p->painter->drawPolyline(p->points->constData(), p->npoints);
    }
  if (!p->bounding_stack.empty())
    {
      double point_x, point_y;
      for (i = 0; i < p->npoints; i++)
        {
          point_x = p->points->constData()[i].x();
          point_y = p->points->constData()[i].y();
          if (p->bounding_stack.top().x_max < point_x) p->bounding_stack.top().x_max = point_x;
          if (p->bounding_stack.top().x_min > point_x) p->bounding_stack.top().x_min = point_x;
          if (p->bounding_stack.top().y_max < point_y) p->bounding_stack.top().y_max = point_y;
          if (p->bounding_stack.top().y_min > point_y) p->bounding_stack.top().y_min = point_y;
        }

      /* A vertical or horizontal polyline needs a bigger bounding box inside gredit so that the user can click on it */
      double min_bbox_size = 8;
      if (p->bounding_stack.top().x_max - p->bounding_stack.top().x_min < min_bbox_size)
        {
          p->bounding_stack.top().x_min -= min_bbox_size / 2;
          p->bounding_stack.top().x_max += min_bbox_size / 2;
        }
      if (p->bounding_stack.top().y_max - p->bounding_stack.top().y_min < min_bbox_size)
        {
          p->bounding_stack.top().y_min -= min_bbox_size / 2;
          p->bounding_stack.top().y_max += min_bbox_size / 2;
        }
    }
}

static void polyline(int n, double *px, double *py)
{
  int ln_type, ln_color;
  double ln_width;
  int i, list[10];

  if (n > p->max_points)
    {
      p->points->resize(n);
      p->max_points = n;
    }
  ln_type = gkss->asf[0] ? gkss->ltype : gkss->lindex;
  ln_color = gkss->asf[2] ? gkss->plcoli : 1;
  if (ln_color < 0 || ln_color >= MAX_COLOR) ln_color = 1;
  ln_width = gkss->asf[1] ? gkss->lwidth : 1;

  ln_width *= p->nominal_size;
  /* line widths < 0.1 no longer provide meaningful results */
  if (ln_width < 0.1) ln_width = 0.1;

  p->painter->save();
  p->painter->setRenderHint(QPainter::Antialiasing);
  QColor transparent_color(p->rgb[ln_color]);
  transparent_color.setAlpha(p->transparency);

  if (ln_type != GKS_K_LINETYPE_SOLID)
    {
      gks_get_dash_list(ln_type, 1.0, list);
      QVector<qreal> dashPattern(list[0]);
      for (i = 0; i < list[0]; i++) dashPattern[i] = (double)list[i + 1];

      QPen pen(QPen(transparent_color, ln_width, Qt::CustomDashLine, Qt::FlatCap, Qt::RoundJoin));
      pen.setDashPattern(dashPattern);
      p->painter->setPen(pen);
    }
  else
    p->painter->setPen(QPen(transparent_color, ln_width, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));

  line_routine(n, px, py, ln_type, gkss->cntnr);

  p->painter->restore();
}

static void draw_marker(double xn, double yn, int mtype, double mscale, int mcolor)
{
  double x, y;
  int i;
  int pc, op;
  double r, d, scale, xr, yr;
  QPolygonF *points;

#include "marker.h"

  QColor marker_color(p->rgb[mcolor]);
  marker_color.setAlpha(p->transparency);
  QColor border_color(p->rgb[gkss->bcoli]);
  border_color.setAlpha(p->transparency);

  mscale *= p->nominal_size;
  r = 3 * mscale;
  d = 2 * r;
  scale = 0.01 * mscale / 3.0;

  xr = r;
  yr = 0;
  seg_xform_rel(&xr, &yr);
  r = sqrt(xr * xr + yr * yr);

  NDC_to_DC(xn, yn, x, y);

  pc = 0;
  mtype = (d > 0) ? mtype + marker_off : marker_off + 1;

  do
    {
      op = marker[mtype][pc];
      switch (op)
        {

        case 1: /* point */
          p->painter->setPen(QPen(marker_color, p->nominal_size, Qt::SolidLine, Qt::FlatCap));
          p->painter->drawPoint(QPointF(x, y));
          break;

        case 2: /* line */
          for (i = 0; i < 2; i++)
            {
              xr = scale * marker[mtype][pc + 2 * i + 1];
              yr = -scale * marker[mtype][pc + 2 * i + 2];
              seg_xform_rel(&xr, &yr);
              (*p->points)[i] = QPointF(x - xr, y + yr);
            }
          p->painter->setPen(
              QPen(marker_color, max(gkss->bwidth, gkss->lwidth) * p->nominal_size, Qt::SolidLine, Qt::FlatCap));
          p->painter->drawPolyline(p->points->constData(), 2);
          pc += 4;
          break;

        case 3: /* polygon */
        case 9: /* border polygon */
          if (op == 3 || gkss->bwidth > 0)
            {
              points = new QPolygonF(marker[mtype][pc + 1]);
              for (i = 0; i < marker[mtype][pc + 1]; i++)
                {
                  xr = scale * marker[mtype][pc + 2 + 2 * i];
                  yr = -scale * marker[mtype][pc + 3 + 2 * i];
                  seg_xform_rel(&xr, &yr);
                  (*points)[i] = QPointF(x - xr, y + yr);
                }
              p->painter->setPen(QPen(op == 3 ? marker_color : border_color, gkss->bwidth * p->nominal_size,
                                      Qt::SolidLine, Qt::FlatCap, Qt::RoundJoin));
              p->painter->drawPolyline(points->constData(), marker[mtype][pc + 1]);
              delete points;
            }
          pc += 1 + 2 * marker[mtype][pc + 1];
          break;

        case 4: /* filled polygon */
        case 5: /* hollow polygon */
          points = new QPolygonF(marker[mtype][pc + 1]);
          if (op == 4)
            {
              p->painter->setBrush(QBrush(marker_color, Qt::SolidPattern));
              if (gkss->bcoli != gkss->pmcoli && gkss->bwidth > 0)
                p->painter->setPen(
                    QPen(border_color, gkss->bwidth * p->nominal_size, Qt::SolidLine, Qt::FlatCap, Qt::RoundJoin));
              else
                p->painter->setPen(Qt::NoPen);
            }
          else
            set_color(0);
          for (i = 0; i < marker[mtype][pc + 1]; i++)
            {
              xr = scale * marker[mtype][pc + 2 + 2 * i];
              yr = -scale * marker[mtype][pc + 3 + 2 * i];
              seg_xform_rel(&xr, &yr);
              (*points)[i] = QPointF(x - xr, y + yr);
            }
          p->painter->drawPolygon(points->constData(), marker[mtype][pc + 1]);
          pc += 1 + 2 * marker[mtype][pc + 1];
          delete points;
          break;

        case 6: /* arc */
          p->painter->setPen(
              QPen(marker_color, max(gkss->bwidth, gkss->lwidth) * p->nominal_size, Qt::SolidLine, Qt::FlatCap));
          p->painter->drawArc(QRectF(x - r, y - r, d, d), 0, 360 * 16);
          break;

        case 7: /* filled arc */
        case 8: /* hollow arc */
          if (op == 7)
            {
              p->painter->setBrush(QBrush(marker_color, Qt::SolidPattern));
              if (gkss->bcoli != gkss->pmcoli && gkss->bwidth > 0)
                p->painter->setPen(QPen(border_color, gkss->bwidth * p->nominal_size, Qt::SolidLine, Qt::FlatCap));
              else
                p->painter->setPen(Qt::NoPen);
            }
          else
            set_color(0);
          p->painter->drawChord(QRectF(x - r, y - r, d, d), 0, 360 * 16);
          break;
        }
      if (!p->bounding_stack.empty())
        {
          double point_x, point_y;
          point_x = x;
          point_y = y;
          if (p->bounding_stack.top().x_max <= point_x) p->bounding_stack.top().x_max = point_x;
          if (p->bounding_stack.top().x_min >= point_x) p->bounding_stack.top().x_min = point_x;
          if (p->bounding_stack.top().y_max <= point_y) p->bounding_stack.top().y_max = point_y;
          if (p->bounding_stack.top().y_min >= point_y) p->bounding_stack.top().y_min = point_y;
        }
      pc++;
    }
  while (op != 0);
}

static void marker_routine(int n, double *px, double *py, int mtype, double mscale, int mcolor)
{
  double x, y;
  double *clrt = gkss->viewport[gkss->cntnr];
  int i, draw;

  for (i = 0; i < n; i++)
    {
      WC_to_NDC(px[i], py[i], gkss->cntnr, x, y);
      seg_xform(&x, &y);

      if (gkss->clip == GKS_K_CLIP)
        draw = (x >= clrt[0] && x <= clrt[1] && y >= clrt[2] && y <= clrt[3]);
      else
        draw = 1;

      if (draw) draw_marker(x, y, mtype, mscale, mcolor);
    }
}

static void polymarker(int n, double *px, double *py)
{
  int mk_type, mk_color;
  double mk_size;

  mk_type = gkss->asf[3] ? gkss->mtype : gkss->mindex;
  mk_size = gkss->asf[4] ? gkss->mszsc : 1;
  mk_color = gkss->asf[5] ? gkss->pmcoli : 1;

  if (mk_color < 0 || mk_color >= MAX_COLOR) mk_color = 1;

  p->painter->save();
  p->painter->setRenderHint(QPainter::Antialiasing);

  marker_routine(n, px, py, mk_type, mk_size, mk_color);

  p->painter->restore();
}

static void text_routine(double x, double y, int nchars, char *chars)
{
  int i, ch, width;
  double xrel, yrel, xstart, ystart, ax, ay;
  QFontMetrics fm = QFontMetrics(*p->font);
  QString s = QString("");
  if (p->family == 3)
    {
      /* Open Symbol maps codepoints to glyphs correctly, but GKS expects mappings like a to alpha, which we need to
       * revert */
      for (i = 0; i < nchars; i++)
        {
          ch = chars[i];
          if (ch < 0) ch += 256;
          ch = symbol2utf[ch];
          s.append(QChar(ch));
        }
    }
  else
    {
      s = QString::fromUtf8(chars);
    }

  (void)nchars;

  NDC_to_DC(x, y, xstart, ystart);

#if QT_VERSION >= QT_VERSION_CHECK(5, 11, 0)
  width = fm.horizontalAdvance(s);
#else
  width = fm.width(s);
#endif
  xrel = width * xfac[gkss->txal[0]];
  yrel = p->capheight * yfac[gkss->txal[1]];
  CharXform(xrel, yrel, ax, ay);
  xstart += ax;
  ystart -= ay;

  if (fabs(p->angle) > FEPS)
    {
      p->painter->save();
      p->painter->translate(xstart, ystart);
      p->painter->rotate(-p->angle);
      p->painter->drawText(0, 0, s);
      p->painter->restore();
    }
  else
    p->painter->drawText(xstart, ystart, s);

  if (!p->bounding_stack.empty())
    {
      p->bounding_stack.top().x_max = xstart + xrel;
      p->bounding_stack.top().x_min = xstart;
      p->bounding_stack.top().y_max = ystart + yrel;
      p->bounding_stack.top().y_min = ystart;
    }
}

static void set_font(int font)
{
  double scale, ux, uy;
  int fontNum, size, bold, italic;
  double width, height, capheight;

  font = abs(font);
  if (font >= 101 && font <= 129)
    font -= 100;
  else if (font >= 1 && font <= 32)
    font = map[font - 1];
  else
    font = 9;

  WC_to_NDC_rel(gkss->chup[0], gkss->chup[1], gkss->cntnr, ux, uy);
  seg_xform_rel(&ux, &uy);

  p->alpha = -atan2(ux, uy);
  p->angle = p->alpha * 180 / M_PI;
  if (p->angle < 0) p->angle += 360;

  scale = sqrt(gkss->chup[0] * gkss->chup[0] + gkss->chup[1] * gkss->chup[1]);
  ux = gkss->chup[0] / scale * gkss->chh;
  uy = gkss->chup[1] / scale * gkss->chh;
  WC_to_NDC_rel(ux, uy, gkss->cntnr, ux, uy);

  width = 0;
  height = sqrt(ux * ux + uy * uy);
  seg_xform_rel(&width, &height);

  height = sqrt(width * width + height * height);
  capheight = height * (fabs(p->c) + 1);
  p->capheight = nint(capheight);

  fontNum = font - 1;
  size = nint(p->capheight / capheights[fontNum]);
  if (size < 1) size = 1;
  if (font > 13) font += 3;
  p->family = (font - 1) / 4;
  bold = (font % 4 == 1 || font % 4 == 2) ? 0 : 1;
  italic = (font % 4 == 2 || font % 4 == 0);

  p->font->setFamily(fonts[p->family]);
  p->font->setBold(bold);
  p->font->setItalic(italic);
  p->font->setPixelSize(size);

  p->painter->setFont(*p->font);
}

static void fill_routine(int n, double *px, double *py, int tnr);

static void text(double px, double py, int nchars, char *chars)
{
  int tx_font, tx_prec, tx_color;
  double x, y;

  tx_font = gkss->asf[6] ? gkss->txfont : predef_font[gkss->tindex - 1];
  tx_prec = gkss->asf[6] ? gkss->txprec : predef_prec[gkss->tindex - 1];
  tx_color = gkss->asf[9] ? gkss->txcoli : 1;

  if (tx_color < 0 || tx_color >= MAX_COLOR) tx_color = 1;

  p->painter->save();
  p->painter->setRenderHint(QPainter::Antialiasing);
  QColor transparent_color(p->rgb[tx_color]);
  transparent_color.setAlpha(p->transparency);
  p->painter->setPen(QPen(transparent_color, p->nominal_size, Qt::SolidLine, Qt::FlatCap, Qt::RoundJoin));

  if (tx_prec == GKS_K_TEXT_PRECISION_STRING)
    {
      set_font(tx_font);

      WC_to_NDC(px, py, gkss->cntnr, x, y);
      seg_xform(&x, &y);

      text_routine(x, y, nchars, chars);
    }
  else
    {
      if ((tx_prec == GKS_K_TEXT_PRECISION_STROKE || tx_prec == GKS_K_TEXT_PRECISION_CHAR) && fontfile == 0)
        {
          fontfile = gks_open_font();
          gkss->fontfile = fontfile;
        }
      gks_emul_text(px, py, nchars, chars, line_routine, fill_routine);
    }

  p->painter->restore();
}

static void fill_routine(int n, double *px, double *py, int tnr)
{
  int i;
  double x, y, xi, yi;
  QPolygonF *points;

  points = new QPolygonF();
  for (i = 0; i < n; i++)
    {
      if (isnan(px[i]) || isnan(py[i]))
        {
          NDC_to_DC(0, 0, xi, yi);
          points->append(QPointF(xi, yi));
          continue;
        }
      WC_to_NDC(px[i], py[i], tnr, x, y);
      seg_xform(&x, &y);
      NDC_to_DC(x, y, xi, yi);
      points->append(QPointF(xi, yi));
    }

  if (points->size() >= 2)
    {
      p->painter->drawPolygon(points->constData(), points->size());
    }

  if (!p->bounding_stack.empty())
    {
      double point_x, point_y;
      for (i = 0; i < points->size() - 1; i++)
        {
          point_x = points->constData()[i].x();
          point_y = points->constData()[i].y();
          if (p->bounding_stack.top().x_max < point_x) p->bounding_stack.top().x_max = point_x;
          if (p->bounding_stack.top().x_min > point_x) p->bounding_stack.top().x_min = point_x;
          if (p->bounding_stack.top().y_max < point_y) p->bounding_stack.top().y_max = point_y;
          if (p->bounding_stack.top().y_min > point_y) p->bounding_stack.top().y_min = point_y;
        }
    }
  delete points;
}

static void fillarea(int n, double *px, double *py)
{
  int fl_inter, fl_style, fl_color;

  fl_inter = gkss->asf[10] ? gkss->ints : predef_ints[gkss->findex - 1];
  fl_style = gkss->asf[11] ? gkss->styli : predef_styli[gkss->findex - 1];
  fl_color = gkss->asf[12] ? gkss->facoli : 1;

  if (fl_color < 0 || fl_color >= MAX_COLOR) fl_color = 1;

  p->painter->save();
  p->painter->setRenderHint(QPainter::Antialiasing);
  QColor transparent_color(p->rgb[fl_color]);
  transparent_color.setAlpha(p->transparency);

  if (fl_inter == GKS_K_INTSTYLE_HOLLOW)
    {
      p->painter->setPen(
          QPen(transparent_color, gkss->bwidth * p->nominal_size, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));
      line_routine(n, px, py, DrawBorder, gkss->cntnr);
    }
  else if (fl_inter == GKS_K_INTSTYLE_SOLID)
    {
      p->painter->setPen(Qt::NoPen);
      p->painter->setBrush(QBrush(transparent_color, Qt::SolidPattern));
      fill_routine(n, px, py, gkss->cntnr);
    }
  else if (fl_inter == GKS_K_INTSTYLE_PATTERN || fl_inter == GKS_K_INTSTYLE_HATCH)
    {
      if (fl_inter == GKS_K_INTSTYLE_HATCH) fl_style += HATCH_STYLE;
      if (fl_style >= PATTERNS) fl_style = 1;
      if (p->pattern[fl_style] == NULL || p->pcolor[fl_style] != fl_color)
        {
          if (p->pattern[fl_style] != NULL) free(p->pattern[fl_style]);
          p->pattern[fl_style] = create_pattern(fl_style, fl_color);
          p->pcolor[fl_style] = fl_color;
        }
      p->painter->setPen(Qt::NoPen);
      p->painter->setBrush(QBrush(transparent_color, *p->pattern[fl_style]));
      fill_routine(n, px, py, gkss->cntnr);
    }

  p->painter->restore();
}

static void cellarray(double xmin, double xmax, double ymin, double ymax, int dx, int dy, int dimx, int *colia,
                      int true_color)
{
  double x1, y1, x2, y2;
  double xi1, xi2, yi1, yi2;
  double x, y;
  int width, height;
  int i, j, ix, iy, ind;
  int swapx, swapy;

  WC_to_NDC(xmin, ymax, gkss->cntnr, x1, y1);
  seg_xform(&x1, &y1);
  NDC_to_DC(x1, y1, xi1, yi1);

  WC_to_NDC(xmax, ymin, gkss->cntnr, x2, y2);
  seg_xform(&x2, &y2);
  NDC_to_DC(x2, y2, xi2, yi2);

  width = nint(abs(xi2 - xi1) * p->device_pixel_ratio);
  height = nint(abs(yi2 - yi1) * p->device_pixel_ratio);
  if (width == 0 || height == 0) return;
  x = min(xi1, xi2);
  y = min(yi1, yi2);

  swapx = xi1 > xi2;
  swapy = yi1 < yi2;

  if (!p->bounding_stack.empty())
    {
      p->bounding_stack.top().x_max = xi2;
      p->bounding_stack.top().x_min = xi1;
      if (swapy)
        {
          p->bounding_stack.top().y_max = yi2;
          p->bounding_stack.top().y_min = yi1;
        }
      else
        {
          p->bounding_stack.top().y_max = yi1;
          p->bounding_stack.top().y_min = yi2;
        }
    }

  if (!true_color)
    {
      QImage img = QImage(width, height, QImage::Format_ARGB32);
#if QT_VERSION >= QT_VERSION_CHECK(5, 1, 0)
      img.setDevicePixelRatio(p->device_pixel_ratio);
#endif
      for (j = 0; j < height; j++)
        {
          iy = dy * j / height;
          if (swapy) iy = dy - 1 - iy;
          for (i = 0; i < width; i++)
            {
              ix = dx * i / width;
              if (swapx) ix = dx - 1 - ix;
              ind = colia[iy * dimx + ix];
              ind = FIX_COLORIND(ind);
              QColor transparent_color(p->rgb[ind]);
              transparent_color.setAlpha(p->transparency);
              img.setPixel(i, j, transparent_color.rgba());
            }
        }
      p->painter->drawPixmap(QPointF(x, y), QPixmap::fromImage(img));
    }
  else
    {
      unsigned char *pixels = (unsigned char *)gks_malloc(width * height * 4);
      gks_resample((const unsigned char *)colia, pixels, dx, dy, width, height, dimx, swapx, swapy,
                   gkss->resample_method);
      /* TODO: Use QImage::Format_RGBA8888 once GR stops supporting Qt4? */
      for (j = 0; j < height; j++)
        {
          for (i = 0; i < width; i++)
            {
              unsigned char red = pixels[(j * width + i) * 4 + 0];
              unsigned char green = pixels[(j * width + i) * 4 + 1];
              unsigned char blue = pixels[(j * width + i) * 4 + 2];
              unsigned char alpha = (unsigned char)(pixels[(j * width + i) * 4 + 3] * gkss->alpha);

              ((unsigned int *)pixels)[j * width + i] = (alpha << 24u) + (red << 16u) + (green << 8u) + (blue << 0u);
            }
        }
      QImage img = QImage(pixels, width, height, QImage::Format_ARGB32);
#if QT_VERSION >= QT_VERSION_CHECK(5, 1, 0)
      img.setDevicePixelRatio(p->device_pixel_ratio);
#endif
      p->painter->drawPixmap(QPointF(x, y), QPixmap::fromImage(img));
      gks_free(pixels);
    }
}

static void to_DC(int n, double *x, double *y)
{
  int i;
  double xn, yn;

  for (i = 0; i < n; i++)
    {
      WC_to_NDC(x[i], y[i], gkss->cntnr, xn, yn);
      seg_xform(&xn, &yn);
      x[i] = p->a * xn + p->b;
      y[i] = p->c * yn + p->d;
    }
}

static void draw_path(int n, double *px, double *py, int nc, int *codes)
{
  int i, j;
  double x[3], y[3], w, h, a1, a2;
  double cur_x = 0, cur_y = 0;
  double start_x = 0, start_y = 0;
  QPainterPath path;

  p->painter->save();
  p->painter->setRenderHint(QPainter::Antialiasing);

  QColor stroke_color(p->rgb[gkss->bcoli]);
  stroke_color.setAlpha(p->transparency);
  QColor fill_color(p->rgb[gkss->facoli]);
  fill_color.setAlpha(p->transparency);

  j = 0;
  for (i = 0; i < nc; ++i)
    {
      assert(j <= n);
      switch (codes[i])
        {
        case 'M':
        case 'm':
          x[0] = px[j];
          y[0] = py[j];
          if (codes[i] == 'm')
            {
              x[0] += cur_x;
              y[0] += cur_y;
            }
          cur_x = start_x = x[0];
          cur_y = start_y = y[0];
          to_DC(1, x, y);
          path.moveTo(x[0], y[0]);
          j += 1;
          break;
        case 'L':
        case 'l':
          x[0] = px[j];
          y[0] = py[j];
          if (codes[i] == 'l')
            {
              x[0] += cur_x;
              y[0] += cur_y;
            }
          cur_x = x[0];
          cur_y = y[0];
          to_DC(1, x, y);
          path.lineTo(x[0], y[0]);
          j += 1;
          break;
        case 'Q':
        case 'q':
          x[0] = px[j];
          y[0] = py[j];
          if (codes[i] == 'q')
            {
              x[0] += cur_x;
              y[0] += cur_y;
            }
          x[1] = px[j + 1];
          y[1] = py[j + 1];
          if (codes[i] == 'q')
            {
              x[1] += cur_x;
              y[1] += cur_y;
            }
          cur_x = x[1];
          cur_y = y[1];
          to_DC(2, x, y);
          path.quadTo(x[0], y[0], x[1], y[1]);
          j += 2;
          break;
        case 'C':
        case 'c':
          x[0] = px[j];
          y[0] = py[j];
          if (codes[i] == 'c')
            {
              x[0] += cur_x;
              y[0] += cur_y;
            }
          x[1] = px[j + 1];
          y[1] = py[j + 1];
          if (codes[i] == 'c')
            {
              x[1] += cur_x;
              y[1] += cur_y;
            }
          x[2] = px[j + 2];
          y[2] = py[j + 2];
          if (codes[i] == 'c')
            {
              x[2] += cur_x;
              y[2] += cur_y;
            }
          cur_x = x[2];
          cur_y = y[2];
          to_DC(3, x, y);
          path.cubicTo(x[0], y[0], x[1], y[1], x[2], y[2]);
          j += 3;
          break;
        case 'A':
        case 'a':
          {
            double rx, ry, cx, cy;
            rx = fabs(px[j]);
            ry = fabs(py[j]);
            a1 = px[j + 1];
            a2 = py[j + 1];
            cx = cur_x - rx * cos(a1);
            cy = cur_y - ry * sin(a1);
            x[0] = cx - rx;
            y[0] = cy - ry;
            x[1] = cx + rx;
            y[1] = cy + ry;
            cur_x = cx + rx * cos(a2);
            cur_y = cy + ry * sin(a2);
          }
          to_DC(2, x, y);
          w = x[1] - x[0];
          h = y[1] - y[0];
          a1 *= -180 / M_PI;
          a2 *= -180 / M_PI;
          while (fabs(a2 - a1) > 360)
            {
              if (a1 > a2)
                {
                  path.arcTo(x[0], y[0], w, h, a1, -180);
                  a1 -= 180;
                }
              else
                {
                  path.arcTo(x[0], y[0], w, h, a1, 180);
                  a1 += 180;
                }
            }
          path.arcTo(x[0], y[0], w, h, a1, (a2 - a1));
          j += 3;
          break;
        case 's': /* close and stroke */
          path.closeSubpath();
          cur_x = start_x;
          cur_y = start_y;
          p->painter->strokePath(
              path, QPen(stroke_color, gkss->bwidth * p->nominal_size, Qt::SolidLine, Qt::FlatCap, Qt::RoundJoin));
          break;
        case 'S': /* stroke */
          p->painter->strokePath(
              path, QPen(stroke_color, gkss->bwidth * p->nominal_size, Qt::SolidLine, Qt::FlatCap, Qt::RoundJoin));
          break;
        case 'F': /* fill (even-odd) and stroke */
        case 'G': /* fill (winding) and stroke */
          path.closeSubpath();
          cur_x = start_x;
          cur_y = start_y;
          path.setFillRule(codes[i] == 'F' ? Qt::OddEvenFill : Qt::WindingFill);
          p->painter->fillPath(path, fill_color);
          p->painter->strokePath(
              path, QPen(stroke_color, gkss->bwidth * p->nominal_size, Qt::SolidLine, Qt::FlatCap, Qt::RoundJoin));
          break;
        case 'f': /* fill (even-odd) */
        case 'g': /* fill (winding) */
          path.closeSubpath();
          cur_x = start_x;
          cur_y = start_y;
          path.setFillRule(codes[i] == 'f' ? Qt::OddEvenFill : Qt::WindingFill);
          p->painter->fillPath(path, fill_color);
          break;
        case 'Z': /* closepath */
          path.closeSubpath();
          cur_x = start_x;
          cur_y = start_y;
          break;
        case '\0':
          break;
        default:
          gks_perror("invalid path code ('%c')", codes[i]);
          exit(1);
        }
    }

  if (!p->bounding_stack.empty())
    {
      if (p->bounding_stack.top().x_max < path.boundingRect().x() + path.boundingRect().width())
        p->bounding_stack.top().x_max = path.boundingRect().x() + path.boundingRect().width();
      if (p->bounding_stack.top().x_min > path.boundingRect().x())
        p->bounding_stack.top().x_min = path.boundingRect().x();
      if (p->bounding_stack.top().y_max < path.boundingRect().y() + path.boundingRect().height())
        p->bounding_stack.top().y_max = path.boundingRect().y() + path.boundingRect().height();
      if (p->bounding_stack.top().y_min > path.boundingRect().y())
        p->bounding_stack.top().y_min = path.boundingRect().y();
      if (p->bounding_stack.top().y_max < p->bounding_stack.top().y_min)
        {
          double tmp = p->bounding_stack.top().y_max;
          p->bounding_stack.top().y_max = p->bounding_stack.top().y_min;
          p->bounding_stack.top().y_min = tmp;
        }
    }
  p->painter->restore();
}

static void draw_lines(int n, double *px, double *py, int *attributes)
{
  int i, j = 0, rgba, line_color = MAX_COLOR;
  double x, y, xim1, yim1, xi, yi;
  float line_width, red, green, blue;

  p->painter->save();
  p->painter->setRenderHint(QPainter::Antialiasing);

  WC_to_NDC(px[0], py[0], gkss->cntnr, x, y);
  seg_xform(&x, &y);
  NDC_to_DC(x, y, xi, yi);

  for (i = 1; i < n; i++)
    {
      xim1 = xi;
      yim1 = yi;
      WC_to_NDC(px[i], py[i], gkss->cntnr, x, y);
      seg_xform(&x, &y);
      NDC_to_DC(x, y, xi, yi);

      line_width = 0.001 * attributes[j++];
      rgba = attributes[j++];
      red = rgba & 0xff;
      green = (rgba >> 8) & 0xff;
      blue = (rgba >> 16) & 0xff;
      p->rgb[line_color].setRgb(red, green, blue);
      p->rgb[line_color].setAlpha(p->transparency);

      p->painter->setPen(QPen(p->rgb[line_color], line_width * p->nominal_size, Qt::SolidLine, Qt::RoundCap));
      p->painter->drawLine(xim1, yim1, xi, yi);
    }

  p->painter->restore();
}

static void draw_markers(int n, double *px, double *py, int *attributes)
{
  int mk_type, mk_color = MAX_COLOR;
  double x, y, mk_size;
  double *clrt = gkss->viewport[gkss->cntnr];
  int i, j = 0, rgba, red, green, blue, draw;

  mk_type = gkss->asf[3] ? gkss->mtype : gkss->mindex;

  p->painter->save();
  p->painter->setRenderHint(QPainter::Antialiasing);

  for (i = 0; i < n; i++)
    {
      WC_to_NDC(px[i], py[i], gkss->cntnr, x, y);
      seg_xform(&x, &y);

      if (gkss->clip == GKS_K_CLIP)
        draw = (x >= clrt[0] && x <= clrt[1] && y >= clrt[2] && y <= clrt[3]);
      else
        draw = 1;

      mk_size = 0.001 * attributes[j++];
      rgba = attributes[j++];
      red = rgba & 0xff;
      green = (rgba >> 8) & 0xff;
      blue = (rgba >> 16) & 0xff;
      p->rgb[mk_color].setRgb(red, green, blue);
      p->rgb[mk_color].setAlpha(p->transparency);

      if (draw) draw_marker(x, y, mk_type, mk_size, mk_color);
    }

  p->painter->restore();
}

static void draw_triangles(int n, double *px, double *py, int ntri, int *tri)
{
  double x, y, xi, yi;
  int i, j, k, rgba, line_color = MAX_COLOR;
  int red, green, blue;
  QPolygonF *triangle;

  p->painter->save();
  p->painter->setRenderHint(QPainter::Antialiasing);

  if (n > p->max_points)
    {
      p->points->resize(n);
      p->max_points = n;
    }

  for (i = 0; i < n; ++i)
    {
      WC_to_NDC(px[i], py[i], gkss->cntnr, x, y);
      seg_xform(&x, &y);
      NDC_to_DC(x, y, xi, yi);
      (*p->points)[i] = QPointF(xi, yi);
      if (!p->bounding_stack.empty())
        {
          double point_x, point_y;
          point_x = xi;
          point_y = yi;
          if (p->bounding_stack.top().x_max <= point_x) p->bounding_stack.top().x_max = point_x;
          if (p->bounding_stack.top().x_min >= point_x) p->bounding_stack.top().x_min = point_x;
          if (p->bounding_stack.top().y_max <= point_y) p->bounding_stack.top().y_max = point_y;
          if (p->bounding_stack.top().y_min >= point_y) p->bounding_stack.top().y_min = point_y;
        }
    }

  triangle = new QPolygonF(3);
  j = 0;
  for (i = 0; i < ntri / 4; ++i)
    {
      for (k = 0; k < 3; ++k)
        {
          (*triangle)[k] = (*p->points)[tri[j] - 1];
          j++;
        }

      rgba = tri[j++];
      red = rgba & 0xff;
      green = (rgba >> 8) & 0xff;
      blue = (rgba >> 16) & 0xff;
      p->rgb[line_color].setRgb(red, green, blue);
      p->rgb[line_color].setAlpha(p->transparency);

      p->painter->setPen(
          QPen(p->rgb[line_color], gkss->lwidth * p->nominal_size, Qt::SolidLine, Qt::FlatCap, Qt::RoundJoin));

      p->painter->drawPolygon(triangle->constData(), 3);
    }
  delete triangle;

  p->painter->restore();
}

static void fill_polygons(int n, double *px, double *py, int nply, int *ply)
{
  double x, y, xi, yi;
  int i, j, k, rgba, len;
  int red, green, blue, alpha;
  QColor fill_color;

  p->painter->save();
  p->painter->setRenderHint(QPainter::Antialiasing);

  QColor border_color(p->rgb[gkss->bcoli]);
  border_color.setAlpha(p->transparency);

  if (n > p->max_points)
    {
      p->points->resize(n);
      p->max_points = n;
    }

  for (i = 0; i < n; ++i)
    {
      WC_to_NDC(px[i], py[i], gkss->cntnr, x, y);
      seg_xform(&x, &y);
      NDC_to_DC(x, y, xi, yi);
      (*p->points)[i] = QPointF(xi, yi);
      if (!p->bounding_stack.empty())
        {
          double point_x, point_y;
          point_x = xi;
          point_y = yi;
          if (p->bounding_stack.top().x_max <= point_x) p->bounding_stack.top().x_max = point_x;
          if (p->bounding_stack.top().x_min >= point_x) p->bounding_stack.top().x_min = point_x;
          if (p->bounding_stack.top().y_max <= point_y) p->bounding_stack.top().y_max = point_y;
          if (p->bounding_stack.top().y_min >= point_y) p->bounding_stack.top().y_min = point_y;
        }
    }

  j = 0;
  while (j < nply)
    {
      len = ply[j++];
      if (len > p->max_polygon)
        {
          p->polygon->resize(len);
          p->max_polygon = len;
        }
      for (k = 0; k < len; ++k)
        {
          (*p->polygon)[k] = (*p->points)[ply[j] - 1];
          j++;
        }

      rgba = (unsigned int)ply[j++];
      red = rgba & 0xff;
      green = (rgba >> 8) & 0xff;
      blue = (rgba >> 16) & 0xff;
      alpha = (rgba >> 24) & 0xff;
      fill_color.setRgb(red, green, blue);
      fill_color.setAlpha(alpha);

      p->painter->setBrush(QBrush(fill_color, Qt::SolidPattern));
      p->painter->setPen(QPen(border_color, gkss->bwidth * p->nominal_size, Qt::SolidLine, Qt::FlatCap, Qt::RoundJoin));
      p->painter->drawPolygon(p->polygon->constData(), len);
    }

  p->painter->restore();
}

static void gdp(int n, double *px, double *py, int primid, int nc, int *codes)
{
  switch (primid)
    {
    case GKS_K_GDP_DRAW_PATH:
      draw_path(n, px, py, nc, codes);
      break;
    case GKS_K_GDP_DRAW_LINES:
      draw_lines(n, px, py, codes);
      break;
    case GKS_K_GDP_DRAW_MARKERS:
      draw_markers(n, px, py, codes);
      break;
    case GKS_K_GDP_DRAW_TRIANGLES:
      draw_triangles(n, px, py, nc, codes);
      break;
    case GKS_K_GDP_FILL_POLYGONS:
      fill_polygons(n, px, py, nc, codes);
      break;
    default:
      gks_perror("invalid drawing primitive ('%d')", primid);
      exit(1);
    }
}

static void memory_plugin_dl_render(int fctid, int dx, int dy, int dimx, int *ia, int lr1, double *r1, int lr2,
                                    double *r2, int lc, char *chars, void **ptr)
{
  double ratio, w, h;

  switch (fctid)
    {
    case 2:
      ratio = (p->window[1] - p->window[0]) / (p->window[3] - p->window[2]);

      if (p->width > p->height * ratio)
        {
          w = p->height * ratio;
          h = p->height;
        }
      else
        {
          w = p->width;
          h = p->width / ratio;
        }
      if (!p->memory_plugin_initialised)
        {
          int memory_plugin_init_ia[3];
          if (!p->prevent_resize_by_dl)
            {
              p->window[0] = p->window[2] = 0.0;
              p->window[1] = p->window[3] = 1.0;
            }

          p->memory_plugin_mem_path = (char *)gks_malloc(1024);
          p->memory_plugin_mem_ptr = (int *)gks_malloc(3 * sizeof(int) + sizeof(unsigned char *));

          p->memory_plugin_mem_ptr[0] = w;
          p->memory_plugin_mem_ptr[1] = h;
          p->memory_plugin_mem_ptr[2] = p->device_dpi_x * p->device_pixel_ratio;
          *((unsigned char **)(p->memory_plugin_mem_ptr + 3)) = NULL;

          snprintf(p->memory_plugin_mem_path, 1024, "!resizable@%p.mem:r", (void *)p->memory_plugin_mem_ptr);
          chars = p->memory_plugin_mem_path;
          /* set wstype for cairo or agg png in memory */
          memory_plugin_init_ia[2] = p->memory_plugin_wstype;
          p->memory_plugin_initialised = true;
          p->memory_plugin_ws_state_list = *ptr;
          p->memory_plugin(fctid, 0, 0, 3, memory_plugin_init_ia, 0, NULL, 0, NULL, strlen(chars), chars,
                           (&p->memory_plugin_ws_state_list));
          /* activate cairo or agg workstation */
          p->memory_plugin(4, 0, 0, 0, NULL, 0, NULL, 0, NULL, 0, NULL, (&p->memory_plugin_ws_state_list));
        }
      else
        {
          double vp_size[4] = {0};
          /* clear cairo or agg workstation */
          p->memory_plugin(6, 0, 0, 0, NULL, 0, NULL, 0, NULL, 0, NULL, (&p->memory_plugin_ws_state_list));
          /* resize cairo or agg workstation to Qt window */
          vp_size[1] = w * 2.54 / 100 / p->device_dpi_x;
          vp_size[3] = h * 2.54 / 100 / p->device_dpi_y;
          p->memory_plugin(55, 0, 0, 0, NULL, 0, vp_size, 0, vp_size + 2, 0, NULL,
                           (void **)(&p->memory_plugin_ws_state_list));
        }
      return;
    case 54:
      if (!p->prevent_resize_by_dl || !p->interp_was_called)
        {
          p->window[0] = r1[0];
          p->window[1] = r1[1];
          p->window[2] = r2[0];
          p->window[3] = r2[1];
        }
      break;
    case 55:
      if (!p->prevent_resize_by_dl)
        {
          p->viewport[0] = r1[0];
          p->viewport[1] = r1[1];
          p->viewport[2] = r2[0];
          p->viewport[3] = r2[1];
        }
      break;
    case 109:
      p->nominal_size = min(p->width, p->height) / 500.0;
      if (gkss->nominal_size > 0) p->nominal_size *= gkss->nominal_size;
      break;
    }
  if (p->memory_plugin_initialised)
    {
      p->memory_plugin(fctid, dx, dy, dimx, ia, lr1, r1, lr2, r2, lc, chars, (&p->memory_plugin_ws_state_list));
    }
}

static void gks_memory_plugin_write_page()
{
  int width, height;
  unsigned char *mem;
  int ia[2] = {0, GKS_K_WRITE_PAGE_FLAG};
  p->memory_plugin(8, 0, 0, 0, ia, 0, NULL, 0, NULL, 0, NULL, (void **)(&p->memory_plugin_ws_state_list));

  width = p->memory_plugin_mem_ptr[0];
  height = p->memory_plugin_mem_ptr[1];
  mem = *((unsigned char **)(p->memory_plugin_mem_ptr + 3));

  QImage img = QImage(mem, width, height, QImage::Format_ARGB32_Premultiplied);
#if QT_VERSION >= QT_VERSION_CHECK(5, 1, 0)
  img.setDevicePixelRatio(p->device_pixel_ratio);
#endif
  width /= p->device_pixel_ratio;
  height /= p->device_pixel_ratio;

  p->painter->drawPixmap((p->width - width) / 2, (p->height - height) / 2, QPixmap::fromImage(img));
}

static void qt_dl_render(int fctid, int dx, int dy, int dimx, int *ia, int lr1, double *r1, int lr2, double *r2, int lc,
                         char *chars, void **ptr)
{
  GKS_UNUSED(lr1);
  GKS_UNUSED(lr2);
  GKS_UNUSED(lc);
  static gks_state_list_t saved_gkss;
  int true_color;
  int cur_id;
  bounding_struct *top;

  switch (fctid)
    {
    case 2:
      memmove(&saved_gkss, gkss, sizeof(gks_state_list_t));
      memmove(gkss, *ptr, sizeof(gks_state_list_t));

      if (!p->prevent_resize_by_dl)
        {
          p->window[0] = p->window[2] = 0.0;
          p->window[1] = p->window[3] = 1.0;
        }

      p->viewport[0] = p->viewport[2] = 0.0;
      p->viewport[1] = p->mwidth;
      p->viewport[3] = p->mheight;

      set_xform();
      init_norm_xform();
      init_colors();

      gkss->fontfile = fontfile;
      gks_init_core(gkss);
      break;

    case 12:
      polyline(ia[0], r1, r2);
      break;

    case 13:
      polymarker(ia[0], r1, r2);
      break;

    case 14:
      text(r1[0], r2[0], strlen(chars), chars);
      break;

    case 15:
      fillarea(ia[0], r1, r2);
      break;

    case 16:
    case 201:
      true_color = fctid == DRAW_IMAGE;
      cellarray(r1[0], r1[1], r2[0], r2[1], dx, dy, dimx, ia, true_color);
      break;

    case 17:
      gdp(ia[0], r1, r2, ia[1], ia[2], ia + 3);
      break;

    case 48:
      set_color_rep(ia[1], r1[0], r1[1], r1[2]);
      break;

    case 49:
      set_norm_xform(*ia, gkss->window[*ia], gkss->viewport[*ia]);
      gks_set_norm_xform(*ia, gkss->window[*ia], gkss->viewport[*ia]);
      break;

    case 50:
      set_norm_xform(*ia, gkss->window[*ia], gkss->viewport[*ia]);
      gks_set_norm_xform(*ia, gkss->window[*ia], gkss->viewport[*ia]);

      if (*ia == gkss->cntnr) set_clip_rect(*ia);
      break;

    case 52:
      set_clip_rect(gkss->cntnr);
      break;

    case 53:
      set_clip_rect(gkss->cntnr);
      break;

    case 54:
      if (!p->prevent_resize_by_dl || !p->interp_was_called)
        {
          /*
           * In floating window managers, there is always a paint event before a user-generated resize event. Thus,
           * in floating windows managers the first interpretation of the display list always initializes the
           * wswindow (`p->prevent_resize_by_dl` is `false`). In tiling window managers, the first user-generated
           * resize event is queued before the first paint event (the wm fits the window into a tile). In this case,
           * wswindow would never be set. Therefore, always initialize wswindow if this is the first interpretation
           * of a display list.
           */
          p->window[0] = r1[0];
          p->window[1] = r1[1];
          p->window[2] = r2[0];
          p->window[3] = r2[1];
        }

      set_xform();
      init_norm_xform();
      break;

    case 55:
      if (!p->prevent_resize_by_dl)
        {
          p->viewport[0] = r1[0];
          p->viewport[1] = r1[1];
          p->viewport[2] = r2[0];
          p->viewport[3] = r2[1];
        }

      resize_window();
      set_xform();
      init_norm_xform();
      break;

    case 109:
      p->nominal_size = min(p->width, p->height) / 500.0;
      if (gkss->nominal_size > 0) p->nominal_size *= gkss->nominal_size;
      break;

    case 203:
      p->transparency = (int)(r1[0] * 255);
      break;

    case BEGIN_SELECTION:
      p->painter.release();
      if (p->selection == NULL)
        {
          p->selection = new QPixmap(p->width * p->device_pixel_ratio, p->height * p->device_pixel_ratio);
#if QT_VERSION >= QT_VERSION_CHECK(5, 1, 0)
          p->selection->setDevicePixelRatio(p->device_pixel_ratio);
#endif
          p->selection->fill(Qt::white);
        }
      p->painter = std::unique_ptr<ProxyPainter>(new ProxyPainter(*p->selection));
      break;

    case END_SELECTION:
      p->painter = std::unique_ptr<ProxyPainter>(new ProxyPainter(*p->pixmap));
      break;

    case MOVE_SELECTION:
      if (p->selection != NULL)
        {
          int x_offset = (int)(p->a * r1[0] + 0.5);
          int y_offset = (int)(p->c * r2[0] + 0.5);
          QPainter::CompositionMode lastMode = p->painter->compositionMode();
          p->painter->drawPixmap(QPoint(0, 0), *p->pixmap);
          p->painter->setCompositionMode(QPainter::RasterOp_NotSourceXorDestination);
          p->painter->drawPixmap(QPoint(x_offset, y_offset), *p->selection);
          p->painter->setCompositionMode(lastMode);
        }
      break;

    case GKS_SET_BBOX_CALLBACK: /* 260 */
      cur_id = ia[0];
#ifdef _WIN32
      p->bounding_stack.push(
          {DBL_MAX, -DBL_MAX, DBL_MAX, -DBL_MAX, (void (*)(int, double, double, double, double))r1, cur_id});
#else
      p->bounding_stack.push((bounding_struct){DBL_MAX, -DBL_MAX, DBL_MAX, -DBL_MAX,
                                               (void (*)(int, double, double, double, double))r1, cur_id});
#endif
      p->mask_callback = (void (*)(unsigned int, unsigned int, unsigned int *))r2;
      p->painter->beginGroup(cur_id);
      break;

    case GKS_CANCEL_BBOX_CALLBACK: /* 261 */
      p->painter->endGroup();
      assert(!p->bounding_stack.empty());
      top = &p->bounding_stack.top();
      top->fun_call(top->item_id, top->x_min, top->x_max, top->y_min, top->y_max);
      p->bounding_stack.pop();
      break;

    case SET_BACKGROUND:
      if (p->pixmap)
        {
          if (p->bg) delete p->bg;
          p->bg = new QPixmap(*p->pixmap);
        }
      break;

    case CLEAR_BACKGROUND:
      if (p->bg)
        {
          delete p->bg;
          p->bg = NULL;
        }
      break;

    case GKS_BEGIN_PARTIAL:
      cur_id = ia[0];
      p->partial_drawing_callback =
          (void (*)(int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int *))r1;
      p->painter->beginPartial(cur_id);
      break;

    case GKS_END_PARTIAL:
      cur_id = ia[0];
      p->painter->endPartial(cur_id);
      break;
    }
}

static void dl_render_function(int fctid, int dx, int dy, int dimx, int *ia, int lr1, double *r1, int lr2, double *r2,
                               int lc, char *chars, void **ptr)
{
  if (fctid == 2)
    {
      if (ia[2] == 412)
        {
          p->memory_plugin_wstype = 143;
          p->memory_plugin = gks_cairo_plugin;
        }
      else if (ia[2] == 413)
        {
          p->memory_plugin_wstype = 173;
          p->memory_plugin = gks_agg_plugin;
        }
      else
        {
          p->memory_plugin_wstype = 0;
        }
    }
  if (p->memory_plugin_wstype)
    {
      memory_plugin_dl_render(fctid, dx, dy, dimx, ia, lr1, r1, lr2, r2, lc, chars, ptr);
    }
  else
    {
      qt_dl_render(fctid, dx, dy, dimx, ia, lr1, r1, lr2, r2, lc, chars, ptr);
    }
}

static bool dl_contains_only_background_fctid(const char *s)
{
  int sp = 0, *len = NULL, *fctid = NULL;
  // see `purge` in `dl.c` for preserved fctids (+ `2` for openws)
  const std::vector<int> leading_ignored_fctids = {2, 48, 54, 55};

  while (true)
    {
      RESOLVE(len, int, sizeof(int));
      if (*len == 0) break;
      RESOLVE(fctid, int, sizeof(int));
      if (std::find(leading_ignored_fctids.begin(), leading_ignored_fctids.end(), *fctid) ==
          leading_ignored_fctids.end())
        break;
      sp += *len - 2 * sizeof(int);
    }

  return *fctid == CLEAR_BACKGROUND || *fctid == SET_BACKGROUND;
}

static void interp(char *str)
{
  char *s;
  int sp = 0, *len = NULL;

  s = str;

  if (p->bg && !dl_contains_only_background_fctid(s))
    {
      if (gkss->cntnr != 0) set_clip_rect(0);
      p->painter->drawPixmap(QPoint(0, 0), *p->bg);
      if (gkss->cntnr != 0) set_clip_rect(gkss->cntnr);
    }

  RESOLVE(len, int, sizeof(int));
  while (*len)
    {
      sp += gks_dl_read_item(s + sp, &gkss, dl_render_function);
      RESOLVE(len, int, sizeof(int));
    }

  if (p->memory_plugin_wstype && p->memory_plugin_initialised)
    {
      gks_memory_plugin_write_page();
    }

  if (p->mask_callback != NULL)
    {
      const auto &group_mask = p->painter->groupMask();
      p->mask_callback(group_mask.width(), group_mask.height(), group_mask.toCMask());
    }

  if (p->partial_drawing_callback != NULL)
    {
      for (const auto &[id, region] : p->painter->partialCRegions())
        {
          p->partial_drawing_callback(id, region.x, region.y, region.w, region.h, region.pixels);
        }
    }

  p->interp_was_called = true;
}

static void initialize_data()
{
  int i;

  p->pixmap = p->bg = p->selection = NULL;
  p->font = new QFont();

  p->points = new QPolygonF(MAX_POINTS);
  p->npoints = 0;
  p->max_points = MAX_POINTS;

  p->polygon = new QPolygonF(MAX_POLYGON);
  p->max_polygon = MAX_POLYGON;

  for (i = 0; i < PATTERNS; i++)
    {
      p->pattern[i] = NULL;
      p->pcolor[i] = -1;
    }

  p->empty = true;

  p->memory_plugin_initialised = false;
  p->prevent_resize_by_dl = false;
  p->window_stays_on_top = false;
  p->interp_was_called = false;

  p->window[0] = 0.0;
  p->window[1] = 1.0;
  p->window[2] = 0.0;
  p->window[3] = 1.0;

  p->transparency = 255;

  p->mask_callback = NULL;
  p->partial_drawing_callback = NULL;
}

static void release_data()
{
  int i;

  for (i = 0; i < PATTERNS; i++)
    if (p->pattern[i] != NULL) free(p->pattern[i]);

  delete p->polygon;
  delete p->points;
  delete p->font;
#ifndef QT_PLUGIN_USED_AS_PLUGIN_CODE
  /* The pixmap is only owned if the code is not compiled as plugin. In plugin mode, the pixmap is only a non-owning
   * pointer to the underlying paint device of the painter object. */
  if (p->pixmap) delete p->pixmap;
#endif
  if (p->bg) delete p->bg;
  delete p;
}

static void update_metrics(const QPaintDevice *device)
{
  p->width = device->width();
  p->height = device->height();
  if (p->has_user_defined_device_pixel_ratio)
    {
      /* This case was introduced to work around broken HiDPI support in `QQuickPaintedItem`. QML uses phyiscal instead
       * of logical pixels, but this plugin works with logical pixels. Therefore, convert the physical `width` and
       * `height` to logical pixels by dividing by `p->device_pixel_ratio` given by the user. Multiply with the
       * `device_pixel_ratio` set on the paint device to support logical pixels in QML in the future. */
#if QT_VERSION >= QT_VERSION_CHECK(5, 6, 0)
      p->width *= device->devicePixelRatioF() / p->device_pixel_ratio;
      p->height *= device->devicePixelRatioF() / p->device_pixel_ratio;
#elif QT_VERSION >= QT_VERSION_CHECK(5, 1, 0)
      p->width *= device->devicePixelRatio() / p->device_pixel_ratio;
      p->height *= device->devicePixelRatio() / p->device_pixel_ratio;
#else
      p->width /= p->device_pixel_ratio;
      p->height /= p->device_pixel_ratio;
#endif
    }
  else
    {
#if QT_VERSION >= QT_VERSION_CHECK(5, 6, 0)
      p->device_pixel_ratio = device->devicePixelRatioF();
#elif QT_VERSION >= QT_VERSION_CHECK(5, 1, 0)
      p->device_pixel_ratio = device->devicePixelRatio();
#else
      p->device_pixel_ratio = 1.0;
#endif
    }
  p->device_dpi_x = device->physicalDpiX();
  p->device_dpi_y = device->physicalDpiY();
  p->mwidth = (double)p->width / p->device_dpi_x * 0.0254;
  p->mheight = (double)p->height / p->device_dpi_y * 0.0254;
  p->nominal_size = min(p->width, p->height) / 500.0;
  if (gkss->nominal_size > 0) p->nominal_size *= gkss->nominal_size;
}

static int get_paint_device(void)
{
  char *env;
  QPaintDevice *device;

  env = (char *)gks_getenv("GKS_CONID");
  if (!env) env = (char *)gks_getenv("GKSconid");

  if (env != NULL)
    {
      bool has_exclamation_mark = strchr(env, '!');
      bool has_hash_mark = strchr(env, '#');
      QPainter *painter;
      p->has_user_defined_device_pixel_ratio = has_hash_mark;
      if (has_exclamation_mark && has_hash_mark)
        {
          sscanf(env, "%p!%p#%lf", (void **)&p->widget, (void **)&painter, &p->device_pixel_ratio);
          device = p->widget;
        }
      else if (has_exclamation_mark)
        {
          sscanf(env, "%p!%p", (void **)&p->widget, (void **)&painter);
          device = p->widget;
        }
      else if (has_hash_mark)
        {
          sscanf(env, "%p#%lf", (void **)&painter, &p->device_pixel_ratio);
          p->widget = NULL;
          device = painter->device();
        }
      else
        {
          sscanf(env, "%p", (void **)&painter);
          p->widget = NULL;
          device = painter->device();
        }
#ifdef QT_PLUGIN_USED_AS_PLUGIN_CODE
      QPixmap *pixmap = dynamic_cast<QPixmap *>(painter->device());
      if (pixmap != NULL)
        {
          p->pixmap = pixmap;
        }
#endif
      p->painter = std::unique_ptr<ProxyPainter>(new ProxyPainter(*painter));
    }
  else
    {
      return 1;
    }

  update_metrics(device);

  return 0;
}

static void inqdspsize(double *mwidth, double *mheight, int *width, int *height)
{
#if QT_VERSION >= 0x050000
  QScreen *screen = QGuiApplication::primaryScreen();
  if (screen)
    {
      *mwidth = screen->physicalSize().width() * 0.001;
      *mheight = screen->physicalSize().height() * 0.001;
      *width = screen->size().width();
      *height = screen->size().height();
    }
  else
    {
      *mwidth = 0;
      *mheight = 0;
      *width = 0;
      *height = 0;
    }
#else
  {
    QWidget *screen = QApplication::desktop()->screen();
    *mwidth = screen->widthMM() * 0.001;
    *mheight = screen->heightMM() * 0.001;
    *width = screen->width();
    *height = screen->height();
  }
#endif
}

#ifdef QT_PLUGIN_USED_AS_PLUGIN_CODE
void QT_PLUGIN_ENTRY_NAME(int fctid, int dx, int dy, int dimx, int *i_arr, int len_f_arr_1, double *f_arr_1,
                          int len_f_arr_2, double *f_arr_2, int len_c_arr, char *c_arr, void **ptr)
{
  double aspect_ratio;

  p = (ws_state_list *)*ptr;

  switch (fctid)
    {
    case 2:
      gkss = (gks_state_list_t *)*ptr;
      p = new ws_state_list;

      p->width = p->height = 500;
      p->nominal_size = 1;

      initialize_data();

      if (get_paint_device() == 0)
        {
          f_arr_1[0] = p->mwidth;
          f_arr_2[0] = p->mheight;
          i_arr[0] = p->width;
          i_arr[1] = p->height;
        }
      else
        {
          inqdspsize(&f_arr_1[0], &f_arr_2[0], &i_arr[0], &i_arr[1]);
        }

      *ptr = p;
      break;

    case 3:
      if (fontfile > 0)
        {
          gks_close_font(fontfile);
          gkss->fontfile = fontfile = 0;
        }
      release_data();

      p = NULL;
      break;

    case 8:
      if (i_arr[1] & GKS_K_PERFORM_FLAG)
        {
          if (get_paint_device() == 0)
            interp(p->dl.buffer);
          else if (!p->empty)
            gks_perror("can't obtain Qt drawable");
        }
      break;

    case 12:  /* polyline */
    case 13:  /* polymarker */
    case 14:  /* text */
    case 15:  /* fill area */
    case 16:  /* cell array */
    case 201: /* draw image */
      p->empty = false;
      break;

    case 205: /* configure ws */
      if (p->widget != NULL) update_metrics(p->widget);
      f_arr_1[0] = p->mwidth;
      f_arr_2[0] = p->mheight;
      i_arr[0] = p->width;
      i_arr[1] = p->height;
      return;

    case 209: /* inq_ws_state */
      aspect_ratio =
          (p->window[1] - p->window[0]) / (p->window[3] - p->window[2]) * (1.0 * p->device_dpi_x / p->device_dpi_y);
      ;
      get_paint_device();
      if (p->width > p->height * aspect_ratio)
        {
          i_arr[0] = nint(p->height * aspect_ratio);
          i_arr[1] = p->height;
        }
      else
        {
          i_arr[0] = p->width;
          i_arr[1] = nint(p->width / aspect_ratio);
        }
      f_arr_1[0] = p->device_pixel_ratio;
      return;

    default:;
    }

  if (p != NULL)
    gks_dl_write_item(&p->dl, fctid, dx, dy, dimx, i_arr, len_f_arr_1, f_arr_1, len_f_arr_2, f_arr_2, len_c_arr, c_arr,
                      gkss);
}
#endif

#else

#ifdef QT_PLUGIN_USED_AS_PLUGIN_CODE
#define QT_NAME_STRING "Qt"

void QT_PLUGIN_ENTRY_NAME(int fctid, int dx, int dy, int dimx, int *i_arr, int len_f_arr_1, double *f_arr_1,
                          int len_f_arr_2, double *f_arr_2, int len_c_arr, char *c_arr, void **ptr)
{
  GKS_UNUSED(dx);
  GKS_UNUSED(dy);
  GKS_UNUSED(dimx);
  GKS_UNUSED(i_arr);
  GKS_UNUSED(len_f_arr_1);
  GKS_UNUSED(f_arr_1);
  GKS_UNUSED(len_f_arr_2);
  GKS_UNUSED(f_arr_2);
  GKS_UNUSED(len_c_arr);
  GKS_UNUSED(c_arr);
  GKS_UNUSED(ptr);

  if (fctid == 2)
    {
      gks_perror(QT_NAME_STRING " support not compiled in");
      i_arr[0] = 0;
      f_arr_1[0] = 0;
      f_arr_2[0] = 0;
      i_arr[0] = 0;
      i_arr[1] = 0;
      if (c_arr != nullptr) c_arr[0] = '\0';
    }
}

#endif

#endif
