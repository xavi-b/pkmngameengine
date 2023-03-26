#ifndef MAPPERWIDGET_H
#define MAPPERWIDGET_H

#include <QLabel>
#include <QMouseEvent>
#include <QResizeEvent>
#include <QRubberBand>
#include <QPainter>

class MapperWidget : public QWidget
{
public:
    MapperWidget(QWidget* parent = nullptr);

    void          setPixmap(const QPixmap& pixmap);
    virtual QSize sizeHint() const override;
    QSize         tileSizeInPixels() const;

protected:
    virtual void mousePressEvent(QMouseEvent* event) override;
    virtual void mouseMoveEvent(QMouseEvent* event) override;
    virtual void mouseReleaseEvent(QMouseEvent* event) override;
    virtual void resizeEvent(QResizeEvent* event) override;

    virtual void paintEvent(QPaintEvent* event) override;

private:
    QSize gridSize      = {32, 12};
    int   tilePixelSize = 32;

    QRubberBand* rubberBand;
    QPixmap      pixmap;
};

#endif // MAPPERWIDGET_H
