#include "mapperwidget.h"

MapperWidget::MapperWidget(QWidget* parent) : QWidget(parent)
{
    setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Ignored);
    setMouseTracking(true);

    map = std::make_unique<Map>(gridSize.width(), gridSize.height());
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

std::unique_ptr<TileLayer>& MapperWidget::getWorkingLayer()
{
    return map->getLevels()[workingLevelIndex]->getTileLayers()[workingLayerIndex];
}

std::unique_ptr<Level>& MapperWidget::getWorkingLevel()
{
    return map->getLevels()[workingLevelIndex];
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
    update();
}

int MapperWidget::getWorkingLevelIndex() const
{
    return workingLevelIndex;
}

void MapperWidget::setWorkingLevelIndex(int index)
{
    if (workingLevelIndex == index)
        return;

    workingLevelIndex = index;
    emit workingLevelIndexChanged(index);
    update();
}

std::vector<std::unique_ptr<Level>>& MapperWidget::getLevels()
{
    return map->getLevels();
}

std::vector<std::unique_ptr<TileLayer>>& MapperWidget::getWorkingLevelLayers()
{
    return map->getLevels()[workingLevelIndex]->getTileLayers();
}

bool MapperWidget::isLayerVisible(int index) const
{
    if (index >= int(map->getLevels()[workingLevelIndex]->getTileLayers().size()))
        return false;

    return map->getLevels()[workingLevelIndex]->getTileLayers()[index]->isVisible();
}

void MapperWidget::setLayerVisible(int index, bool visible)
{
    if (index >= int(map->getLevels()[workingLevelIndex]->getTileLayers().size()))
        return;

    map->getLevels()[workingLevelIndex]->getTileLayers()[index]->setVisible(visible);
    emit layerVisibleChanged(index, visible);
    update();
}

bool MapperWidget::isLevelVisible(int index) const
{
    if (index >= int(map->getLevels().size()))
        return false;

    return map->getLevels()[index]->isVisible();
}

void MapperWidget::setLevelVisible(int index, bool visible)
{
    if (index >= int(map->getLevels().size()))
        return;

    map->getLevels()[index]->setVisible(visible);
    emit levelVisibleChanged(index, visible);
    update();
}

void MapperWidget::addLevel()
{
    map->addLevel();
    emit levelAdded();
    update();
}

void MapperWidget::removeLevel(int index)
{
    if (index >= int(map->getLevels().size()))
        return;

    if (map->getLevels().size() < 2)
        return;

    map->getLevels().erase(map->getLevels().begin() + index);
    emit levelRemoved(index);
    update();
}

void MapperWidget::setBelowLevelsOpacity(bool opacity)
{
    belowLevelsOpacity = opacity;
    update();
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

                auto  tile  = std::make_unique<Tile>(data.first.toStdString(), rect.x() + i, rect.y() + j);
                auto& level = map->getLevels()[workingLevelIndex];
                auto& layer = level->getTileLayers()[workingLayerIndex];
                (*layer.get())(col + i, row + j).swap(tile);
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

    for (size_t l = 0; l < map->getLevels().size(); ++l)
    {
        auto& level = map->getLevels()[l];

        if (!level->isVisible())
            continue;

        float opacity = 1.0;
        if (belowLevelsOpacity && l < workingLevelIndex)
            opacity = 0.5;

        for (size_t h = 0; h < level->getTileLayers().size(); ++h)
        {
            auto& layer = level->getTileLayers()[h];

            if (!layer->isVisible())
                continue;

            for (int i = 0; i < gridSize.width(); ++i)
            {
                for (int j = 0; j < gridSize.height(); ++j)
                {
                    QPoint origin = {i * selSize, j * selSize};
                    auto&  tile   = (*layer.get())(i, j);
                    if (tile)
                    {
                        QRect   rect   = {QPoint(tile->getRow() * tilePixelSize, tile->getCol() * tilePixelSize),
                                          QSize(tilePixelSize, tilePixelSize)};
                        QPixmap pixmap = QPixmap(tile->getSpritePath().c_str()).copy(rect);
                        painter.setOpacity(opacity);
                        painter.drawPixmap(QRect(origin, pixmap.size() * scaleFactor), pixmap);
                    }
                    painter.setOpacity(1.0);
                    painter.drawRect(QRect(origin, QSize(selSize - 1, selSize - 1)));
                }
            }
        }
    }

    painter.drawPixmap(QRect(origin, overlayPixmap.size() * scaleFactor), overlayPixmap);
}

void MapperWidget::enterEvent(QEnterEvent* event)
{
    emit entered();
}

void MapperWidget::leaveEvent(QEvent* event)
{
    setSelectionPixmap({});
    update();
}
