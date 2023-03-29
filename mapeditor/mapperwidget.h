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

    std::unique_ptr<TileLayer>&              getWorkingLayer();
    std::unique_ptr<Level>&                  getWorkingLevel();
    int                                      getWorkingLayerIndex() const;
    void                                     setWorkingLayerIndex(int index);
    int                                      getWorkingLevelIndex() const;
    void                                     setWorkingLevelIndex(int index);
    std::vector<std::unique_ptr<Level>>&     getLevels();
    std::vector<std::unique_ptr<TileLayer>>& getWorkingLevelLayers();

    bool isLayerVisible(int index) const;
    void setLayerVisible(int index, bool visible);
    bool isLevelVisible(int index) const;
    void setLevelVisible(int index, bool visible);

    void addLevel();
    void removeLevel(int index);

    void setBelowLevelsOpacity(bool opacity);

signals:
    void workingLayerIndexChanged(int index);
    void workingLevelIndexChanged(int index);

    void layerVisibleChanged(int index, bool visible);
    void levelVisibleChanged(int index, bool visible);

    void levelAdded();
    void levelRemoved(int index);

    void entered();

protected:
    virtual void mousePressEvent(QMouseEvent* event) override;
    virtual void mouseMoveEvent(QMouseEvent* event) override;
    virtual void mouseReleaseEvent(QMouseEvent* event) override;
    virtual void resizeEvent(QResizeEvent* event) override;

    virtual void paintEvent(QPaintEvent* event) override;

    virtual void enterEvent(QEnterEvent* event) override;
    virtual void leaveEvent(QEvent* event) override;

private:
    QSize gridSize      = {5, 5};
    int   tilePixelSize = 32;

    QPair<QString, QRect> data;
    QPixmap               overlayPixmap;
    QPoint                origin;

    std::unique_ptr<Map> map;
    int                  workingLayerIndex  = 0;
    int                  workingLevelIndex  = 0;
    bool                 belowLevelsOpacity = false;
};

#endif // MAPPERWIDGET_H
