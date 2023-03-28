#ifndef MAPPERWIDGET_H
#define MAPPERWIDGET_H

#include <QLabel>
#include <QMouseEvent>
#include <QResizeEvent>
#include <QRubberBand>
#include <QPainter>
#include "map.h"

class MapperWidget : public QWidget
{
    Q_OBJECT

public:
    MapperWidget(QWidget* parent = nullptr);

    void          setSelectionPixmap(const QPair<QString, QRect>& data);
    virtual QSize sizeHint() const override;
    QSize         tileSizeInPixels() const;

    Map* getMap() const;
    int  getWorkingLayerIndex() const;
    void setWorkingLayerIndex(int index);

    bool isLayerVisible(int index) const;
    void setLayerVisible(int index, bool visible);

signals:
    void workingLayerIndexChanged(int index);

    void layerVisibleChanged(int index, bool visible);

protected:
    virtual void mousePressEvent(QMouseEvent* event) override;
    virtual void mouseMoveEvent(QMouseEvent* event) override;
    virtual void mouseReleaseEvent(QMouseEvent* event) override;
    virtual void resizeEvent(QResizeEvent* event) override;

    virtual void paintEvent(QPaintEvent* event) override;

private:
    QSize gridSize      = {5, 5};
    int   tilePixelSize = 32;

    QPair<QString, QRect> data;
    QPixmap               overlayPixmap;
    QPoint                origin;

    std::unique_ptr<Map> map;
    std::vector<bool>    visibleLayers;
    int                  workingLayerIndex = 0;
};

#endif // MAPPERWIDGET_H
