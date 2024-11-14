#ifndef MAPPERWIDGET_H
#define MAPPERWIDGET_H

#include "commands/mappercommand.h"
#include "map.h"

#include <QInputDialog>
#include <QLabel>
#include <QMouseEvent>
#include <QPainter>
#include <QResizeEvent>
#include <QRubberBand>
#include <stack>

class MapperWidget : public QWidget
{
    Q_OBJECT
    Q_PROPERTY(bool gridVisible READ isGridVisible WRITE setGridVisible NOTIFY gridVisibleChanged FINAL)

public:
    enum class LayerType
    {
        TILES,
        EVENTS,
        SPECIAL_TILE
    };

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
    void setNight(bool newNight);
    void setLayerType(LayerType type);
    void setCurrentSpecialTileType(SpecialTileType type);

    std::unique_ptr<Map> const& getMap() const;
    void                        swapMap(std::unique_ptr<Map>&& newMap);

    void setMapWidth(size_t v);
    void setMapHeight(size_t v);

    bool isGridVisible() const;
    void setGridVisible(bool newGridVisible);

    void showCoordinates(bool show);

    size_t getCommandsIndex() const;
    size_t getCommandsHistorySize() const;

    void moveContentUp();
    void moveContentDown();
    void moveContentLeft();
    void moveContentRight();

    void commandExecute(std::unique_ptr<MapperCommand> cmd);
    void commandUndo();
    void commandRedo();
    void commandClear();

signals:
    void workingLayerIndexChanged(int index);
    void workingLevelIndexChanged(int index);

    void layerVisibleChanged(int index, bool visible);
    void levelVisibleChanged(int index, bool visible);

    void levelAdded();
    void levelRemoved(int index);

    void reset();

    void gridVisibleChanged();

    void commandsIndexChanged();

protected:
    virtual void mousePressEvent(QMouseEvent* event) override;
    virtual void mouseMoveEvent(QMouseEvent* event) override;
    virtual void mouseReleaseEvent(QMouseEvent* event) override;

    void processMouseEvent(QMouseEvent* event);

    virtual void resizeEvent(QResizeEvent* event) override;

    virtual void paintEvent(QPaintEvent* event) override;

    virtual void enterEvent(QEnterEvent* event) override;
    virtual void leaveEvent(QEvent* event) override;

private:
    void moveContent(int x, int y);

    static constexpr int TilePixelSize = 32;

    QPair<QString, QRect>  data;
    QPixmap                overlayPixmap;
    QMap<QString, QPixmap> pixmaps;
    QMap<QString, QPixmap> nightPixmaps;
    QPoint                 origin;
    bool                   pressed             = false;
    bool                   showSelectionPixmap = false;

    std::unique_ptr<Map> map;
    int                  workingLayerIndex   = 0;
    int                  workingLevelIndex   = 0;
    bool                 belowLevelsOpacity  = false;
    bool                 night               = false;
    LayerType            layerType           = LayerType::TILES;
    SpecialTileType      specialTileType     = SpecialTileType::GRASS;
    bool                 gridVisible         = true;
    bool                 mustShowCoordinates = false;

    std::vector<std::unique_ptr<MapperCommand>> commandsHistory;
    size_t                                      commandsIndex    = 0;
    static constexpr size_t                     MaxCommandsCount = 10;
};

#endif // MAPPERWIDGET_H
