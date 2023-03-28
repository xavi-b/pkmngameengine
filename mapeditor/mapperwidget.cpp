#include "mapperwidget.h"

MapperWidget::MapperWidget(QWidget* parent) : QWidget(parent)
{
    setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Ignored);
    setMouseTracking(true);

    map = std::make_unique<Map>(gridSize.width(), gridSize.height());
    map->addTileLayer(TileLayer::Type::GROUND);
    map->addTileLayer(TileLayer::Type::SOLID);
    map->addTileLayer(TileLayer::Type::OVERLAY);

    visibleLayers.resize(map->getTileLayers().size(), true);
}

void MapperWidget::setSelectionPixmap(const QPair<QString, QRect>& data)
{
    this->data    = data;
    QRect rect    = {data.second.topLeft() * tilePixelSize, data.second.size() * tilePixelSize};
    overlayPixmap = QPixmap(data.first).copy(rect);
}

QSize MapperWidget::sizeHint() const
{
    float scaleFactor = 1.0 * width() / tileSizeInPixels().width();
    return tileSizeInPixels() * scaleFactor;
}

QSize MapperWidget::tileSizeInPixels() const
{
    return {gridSize.width() * tilePixelSize, gridSize.height() * tilePixelSize};
}

Map* MapperWidget::getMap() const
{
    return map.get();
}

int MapperWidget::getWorkingLayerIndex() const
{
    return workingLayerIndex;
}

void MapperWidget::setWorkingLayerIndex(int index)
{
    if (workingLayerIndex == index)
        return;

    workingLayerIndex = index;
    emit workingLayerIndexChanged(index);
}

bool MapperWidget::isLayerVisible(int index) const
{
    if (index >= int(visibleLayers.size()))
        return false;

    return visibleLayers[index];
}

void MapperWidget::setLayerVisible(int index, bool visible)
{
    if (index >= int(visibleLayers.size()))
        return;

    if (visibleLayers[index] == visible)
        return;

    visibleLayers[index] = visible;
    emit layerVisibleChanged(index, visible);
}

void MapperWidget::mousePressEvent(QMouseEvent* event)
{
}

void MapperWidget::mouseMoveEvent(QMouseEvent* event)
{
    float scaleFactor = 1.0 * width() / tileSizeInPixels().width();
    int   selSize     = tilePixelSize * scaleFactor;

    if (event->pos().x() < selSize * gridSize.width() && event->pos().y() < selSize * gridSize.height())
    {
        int col = event->pos().x() / selSize * selSize;
        int row = event->pos().y() / selSize * selSize;
        origin  = {col, row};
        event->accept();
        update();
    }
}

void MapperWidget::mouseReleaseEvent(QMouseEvent* event)
{
    float scaleFactor = 1.0 * width() / tileSizeInPixels().width();
    int   selSize     = tilePixelSize * scaleFactor;

    if (event->pos().x() < selSize * gridSize.width() && event->pos().y() < selSize * gridSize.height())
    {
        int posX = event->pos().x();
        int posY = event->pos().y();
        int col  = posX / selSize;
        int row  = posY / selSize;

        QRect rect = data.second;

        for (int i = 0; i < rect.width(); ++i)
        {
            if (col + i >= gridSize.width())
                continue;

            for (int j = 0; j < rect.height(); ++j)
            {
                if (row + j >= gridSize.height())
                    continue;

                auto tile = std::make_unique<Tile>(data.first.toStdString(), rect.x() + i, rect.y() + j);
                (*map->getTileLayers()[workingLayerIndex].get())(col + i, row + j).swap(tile);
            }
        }

        event->accept();
        update();
    }
}

void MapperWidget::resizeEvent(QResizeEvent* event)
{
}

void MapperWidget::paintEvent(QPaintEvent* event)
{
    float scaleFactor = 1.0 * width() / tileSizeInPixels().width();

    QPainter painter(this);

    painter.setPen(Qt::black);

    int selSize = tilePixelSize * scaleFactor;

    for (int i = 0; i < gridSize.width(); ++i)
    {
        for (int j = 0; j < gridSize.height(); ++j)
        {
            QPoint origin = {i * selSize, j * selSize};
            auto&  tile   = (*map->getTileLayers()[workingLayerIndex].get())(i, j);
            if (tile)
            {
                QRect   rect   = {QPoint(tile->getRow() * tilePixelSize, tile->getCol() * tilePixelSize),
                                  QSize(tilePixelSize, tilePixelSize)};
                QPixmap pixmap = QPixmap(tile->getSpritePath().c_str()).copy(rect);
                painter.drawPixmap(QRect(origin, pixmap.size() * scaleFactor), pixmap);
            }
            painter.drawRect(QRect(origin, QSize(selSize - 1, selSize - 1)));
        }
    }

    painter.drawPixmap(QRect(origin, overlayPixmap.size() * scaleFactor), overlayPixmap);
}
