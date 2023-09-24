#ifndef MAPPERWIDGET_H
#define MAPPERWIDGET_H

#include "map.h"

#include <QInputDialog>
#include <QLabel>
#include <QMouseEvent>
#include <QPainter>
#include <QResizeEvent>
#include <QRubberBand>

class MapperWidget : public QWidget
{
    Q_OBJECT

public:
    MapperWidget(QWidget* parent = nullptr);

    void          setSelectionPixmap(QPair<QString, QRect> const& data);
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
    void setEventLayerEdition(bool edit);

    std::unique_ptr<Map> const& getMap() const;
    void                        swapMap(std::unique_ptr<Map>&& newMap);

    void setMapWidth(size_t v);
    void setMapHeight(size_t v);

signals:
    void workingLayerIndexChanged(int index);
    void workingLevelIndexChanged(int index);

    void layerVisibleChanged(int index, bool visible);
    void levelVisibleChanged(int index, bool visible);

    void levelAdded();
    void levelRemoved(int index);

    void reset();

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
    static constexpr int TilePixelSize = 32;

    QPair<QString, QRect>  data;
    QPixmap                overlayPixmap;
    QMap<QString, QPixmap> pixmaps;
    QPoint                 origin;
    bool                   pressed = false;

    std::unique_ptr<Map> map;
    int                  workingLayerIndex  = 0;
    int                  workingLevelIndex  = 0;
    bool                 belowLevelsOpacity = false;
    bool                 eventLayerEdition  = false;
};

#endif // MAPPERWIDGET_H
