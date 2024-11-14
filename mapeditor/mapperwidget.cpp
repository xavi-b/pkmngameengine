#include "mapperwidget.h"

#include "commands/animatedpropertycommand.h"
#include "commands/changecommand.h"
#include "commands/changescommand.h"
#include "commands/doorpropertycommand.h"
#include "commands/parallelcommand.h"
#include "commands/resetcommand.h"

MapperWidget::MapperWidget(QWidget* parent) : QWidget(parent)
{
    setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Ignored);
    setMouseTracking(true);

    map = std::make_unique<Map>(5, 5);

    connect(this, &MapperWidget::gridVisibleChanged, this, [this]() {
        update();
    });
}

void MapperWidget::setSelectionPixmap(QPair<QString, QRect> const& data)
{
    this->data    = data;
    QRect rect    = {data.second.topLeft() * TilePixelSize, data.second.size() * TilePixelSize};
    overlayPixmap = QPixmap(data.first).copy(rect);
    update();
}

QSize MapperWidget::sizeHint() const
{
    float scaleFactor = 1.0 * width() / tileSizeInPixels().width();
    return tileSizeInPixels() * scaleFactor;
}

QSize MapperWidget::tileSizeInPixels() const
{
    return {int(map->getNCol() * TilePixelSize), int(map->getNRow() * TilePixelSize)};
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
    commandClear();
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
    commandClear();
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
    commandClear();
}

void MapperWidget::setBelowLevelsOpacity(bool opacity)
{
    belowLevelsOpacity = opacity;
    update();
}

void MapperWidget::setNight(bool newNight)
{
    night = newNight;
    update();
}

void MapperWidget::setLayerType(LayerType type)
{
    layerType = type;
}

void MapperWidget::setCurrentSpecialTileType(SpecialTileType type)
{
    specialTileType = type;
}

std::unique_ptr<Map> const& MapperWidget::getMap() const
{
    return map;
}

void MapperWidget::swapMap(std::unique_ptr<Map>&& newMap)
{
    map.swap(newMap);
    update();
    emit reset();
    commandClear();
}

void MapperWidget::setMapWidth(size_t v)
{
    map->setNCol(v);
    resize(size());
    update();
    commandClear();
}

void MapperWidget::setMapHeight(size_t v)
{
    map->setNRow(v);
    resize(size());
    update();
    commandClear();
}

void MapperWidget::mousePressEvent(QMouseEvent* event)
{
    Q_UNUSED(event)
    pressed = true;
}

void MapperWidget::mouseMoveEvent(QMouseEvent* event)
{
    float scaleFactor = 1.0 * width() / tileSizeInPixels().width();
    int   selSize     = TilePixelSize * scaleFactor;

    if (event->pos().x() < selSize * int(map->getNCol()) && event->pos().y() < selSize * int(map->getNRow()))
    {
        int col = event->pos().x() / selSize * selSize;
        int row = event->pos().y() / selSize * selSize;
        origin  = {col, row};

        if (pressed)
        {
            processMouseEvent(event);
            pressed = true;
        }
        else
        {
            event->accept();
            update();
        }
    }
}

void MapperWidget::mouseReleaseEvent(QMouseEvent* event)
{
    processMouseEvent(event);
}

void MapperWidget::processMouseEvent(QMouseEvent* event)
{
    pressed = false;

    float scaleFactor = 1.0 * width() / tileSizeInPixels().width();
    int   selSize     = TilePixelSize * scaleFactor;

    if (event->pos().x() >= selSize * int(map->getNCol()) || event->pos().y() >= selSize * int(map->getNRow())
        || event->pos().x() < 0 || event->pos().y() < 0) // invalid events
        return;

    int posX = event->pos().x();
    int posY = event->pos().y();
    int col  = posX / selSize;
    int row  = posY / selSize;

    auto& level = map->getLevels()[workingLevelIndex];

    switch (layerType)
    {
    case LayerType::EVENTS: {
        auto& layer = level->getEventLayer();

        if (event->button() == Qt::RightButton)
        {
            auto& previousTile = (*layer.get())(col, row);
            commandExecute(std::make_unique<ResetCommand<Event>>(previousTile));
        }
        else if (event->button() == Qt::LeftButton)
        {
            auto& previousEvent = (*layer.get())(col, row);

            QString id = QInputDialog::getText(this,
                                               "Event ID",
                                               QString(),
                                               QLineEdit::Normal,
                                               previousEvent ? previousEvent->getId().c_str() : QString());

            if (id != "")
            {
                auto event = std::make_unique<Event>(id.toStdString());
                commandExecute(std::make_unique<ChangeCommand<Event>>(previousEvent, std::move(event)));
            }
        }
        break;
    }
    case LayerType::SPECIAL_TILE: {
        auto& layer = level->getSpecialTileLayer();

        if (event->button() == Qt::RightButton)
        {
            auto& previousTile = (*layer.get())(col, row);
            commandExecute(std::make_unique<ResetCommand<SpecialTileType>>(previousTile));
        }
        else if (event->button() == Qt::LeftButton)
        {
            auto& previousType = (*layer.get())(col, row);
            auto  type         = std::make_unique<SpecialTileType>(specialTileType);
            commandExecute(std::make_unique<ChangeCommand<SpecialTileType>>(previousType, std::move(type)));
        }
        break;
    }
    default: {
        auto& layer = level->getTileLayers()[workingLayerIndex];

        if (event->button() == Qt::RightButton)
        {
            auto& previousTile = (*layer.get())(col, row);
            commandExecute(std::make_unique<ResetCommand<Tile>>(previousTile));
        }
        else if (event->button() == Qt::LeftButton)
        {
            if (event->modifiers() & Qt::ShiftModifier)
            {
                auto& tile = (*layer.get())(col, row);
                commandExecute(std::make_unique<AnimatedPropertyCommand>(tile));
            }
            else if (event->modifiers() & Qt::AltModifier)
            {
                auto& tile = (*layer.get())(col, row);
                commandExecute(std::make_unique<DoorPropertyCommand>(tile));
            }
            else
            {
                QRect rect = data.second;

                if (!data.first.isEmpty())
                {
                    std::vector<std::unique_ptr<Tile>*> previousTiles;
                    std::vector<std::unique_ptr<Tile>>  newTiles;

                    for (size_t i = 0; i < size_t(rect.width()); ++i)
                    {
                        if (col + i >= map->getNCol())
                            continue;

                        for (size_t j = 0; j < size_t(rect.height()); ++j)
                        {
                            if (row + j >= map->getNRow())
                                continue;

                            std::string spritePath   = data.first.toStdString();
                            auto        previousTile = &(*layer.get())(col + i, row + j);
                            auto        tile         = std::make_unique<Tile>(spritePath, rect.x() + i, rect.y() + j);
                            previousTiles.push_back(previousTile);
                            newTiles.push_back(std::move(tile));
                        }
                    }

                    commandExecute(std::make_unique<ChangesCommand<Tile>>(previousTiles, std::move(newTiles)));
                }
            }
        }
        break;
    }
    }

    event->accept();
    update();
}

void MapperWidget::resizeEvent(QResizeEvent* event)
{
    Q_UNUSED(event)
}

void MapperWidget::paintEvent(QPaintEvent* event)
{
    Q_UNUSED(event)

    float scaleFactor = 1.0 * width() / tileSizeInPixels().width();

    QPainter painter(this);

    painter.setPen(Qt::black);

    int selSize = TilePixelSize * scaleFactor;

    for (size_t l = 0; l < map->getLevels().size(); ++l)
    {
        auto& level = map->getLevels()[l];

        if (!level->isVisible())
            continue;

        float opacity = 1.0;
        if (belowLevelsOpacity && int(l) < workingLevelIndex)
            opacity = 0.5;

        for (size_t h = 0; h < level->getTileLayers().size(); ++h)
        {
            auto& layer = level->getTileLayers()[h];

            if (!layer->isVisible())
                continue;

            if (layer->getType() == TileLayer::Type::GROUND_LIGHTS || layer->getType() == TileLayer::Type::SOLID_LIGHTS
                || layer->getType() == TileLayer::Type::OVERLAY_LIGHTS)
            {
                if (!night)
                    continue;
            }

            for (size_t i = 0; i < map->getNCol(); ++i)
            {
                for (size_t j = 0; j < map->getNRow(); ++j)
                {
                    QPoint origin = {int(i * selSize), int(j * selSize)};
                    auto&  tile   = (*layer.get())(i, j);
                    if (tile)
                    {
                        QRect   rect = {QPoint(tile->getCol() * TilePixelSize, tile->getRow() * TilePixelSize),
                                        QSize(TilePixelSize, TilePixelSize)};
                        QString path = tile->getSpritePath().c_str();
                        if (layer->getType() == TileLayer::Type::GROUND_LIGHTS
                            || layer->getType() == TileLayer::Type::SOLID_LIGHTS
                            || layer->getType() == TileLayer::Type::OVERLAY_LIGHTS)
                        {
                            if (!pixmaps.contains(path))
                            {
                                pixmaps[path] = QPixmap(path);
                            }
                            painter.setOpacity(opacity);
                            painter.drawPixmap(QRect(origin, rect.size() * scaleFactor), pixmaps[path], rect);
                        }
                        else
                        {
                            if (night)
                            {
                                if (!nightPixmaps.contains(path))
                                {
                                    nightPixmaps[path] = QPixmap(path);

                                    QPainter p;

                                    p.begin(&nightPixmaps[path]);
                                    p.setCompositionMode(QPainter::CompositionMode_SourceAtop);
                                    p.fillRect(nightPixmaps[path].rect(),
                                               QColor(TileLayer::NightColor[0],
                                                      TileLayer::NightColor[1],
                                                      TileLayer::NightColor[2],
                                                      TileLayer::NightColor[3]));
                                    p.end();
                                }
                                painter.setOpacity(opacity);
                                painter.drawPixmap(QRect(origin, rect.size() * scaleFactor), nightPixmaps[path], rect);
                            }
                            else
                            {
                                if (!pixmaps.contains(path))
                                {
                                    pixmaps[path] = QPixmap(path);
                                }
                                painter.setOpacity(opacity);
                                painter.drawPixmap(QRect(origin, rect.size() * scaleFactor), pixmaps[path], rect);
                            }
                        }

                        if (tile->isAnimated())
                        {
                            painter.save();
                            painter.setPen(QPen(Qt::cyan, 2));
                            painter.setOpacity(opacity);
                            painter.drawRect(
                                QRect(QPoint(origin.x() + 2, origin.y() + 2), QSize(selSize - 4, selSize - 4)));
                            painter.restore();
                        }

                        if (tile->isDoor())
                        {
                            painter.save();
                            painter.setPen(QPen(Qt::red, 2, Qt::DashLine));
                            painter.setOpacity(opacity);
                            painter.drawRect(
                                QRect(QPoint(origin.x() + 4, origin.y() + 4), QSize(selSize - 8, selSize - 8)));
                            painter.restore();
                        }
                    }
                    painter.setOpacity(1.0);
                    if (isGridVisible())
                        painter.drawRect(QRect(origin, QSize(selSize - 1, selSize - 1)));
                    if (mustShowCoordinates)
                        painter.drawText(origin, QString("%1;%2").arg(i).arg(j));
                }
            }
        }

        auto& eventLayer = level->getEventLayer();

        for (size_t i = 0; i < map->getNCol(); ++i)
        {
            for (size_t j = 0; j < map->getNRow(); ++j)
            {
                QPoint origin = {int(i * selSize), int(j * selSize)};
                auto&  event  = (*eventLayer.get())(i, j);
                if (event)
                {
                    painter.save();
                    painter.setPen(QPen(Qt::red, 2));
                    painter.setOpacity(opacity);
                    QRect rect = QRect(QPoint(origin.x() + 2, origin.y() + 2), QSize(selSize - 4, selSize - 4));
                    painter.drawRect(rect);
                    painter.drawLine(QLine(rect.topLeft(), rect.bottomRight()));
                    painter.drawLine(QLine(rect.topRight(), rect.bottomLeft()));
                    painter.restore();
                }
            }
        }

        auto& specialTileLayer = level->getSpecialTileLayer();

        for (size_t i = 0; i < map->getNCol(); ++i)
        {
            for (size_t j = 0; j < map->getNRow(); ++j)
            {
                QPoint origin      = {int(i * selSize), int(j * selSize)};
                auto&  specialTile = (*specialTileLayer.get())(i, j);
                if (specialTile)
                {
                    painter.save();
                    QRect rect = QRect(QPoint(origin.x() + 2, origin.y() + 2), QSize(selSize - 4, selSize - 4));
                    painter.setPen(QPen(Qt::green, 2));
                    painter.setOpacity(opacity);
                    painter.drawRoundedRect(rect, selSize / 2, selSize / 2);
                    QFont font = painter.font();
                    font.setPixelSize(selSize / 2);
                    painter.setFont(font);
                    QTextOption o;
                    o.setAlignment(Qt::AlignCenter);
                    painter.drawText(rect, QString::number(*specialTile.get()), o);
                    painter.restore();
                }
            }
        }
    }

    if (showSelectionPixmap && layerType == LayerType::TILES)
        painter.drawPixmap(QRect(origin, overlayPixmap.size() * scaleFactor), overlayPixmap);
}

void MapperWidget::enterEvent(QEnterEvent* event)
{
    Q_UNUSED(event)
    showSelectionPixmap = true;
    update();
}

void MapperWidget::leaveEvent(QEvent* event)
{
    Q_UNUSED(event)
    showSelectionPixmap = false;
    update();
}

bool MapperWidget::isGridVisible() const
{
    return gridVisible;
}

void MapperWidget::setGridVisible(bool newGridVisible)
{
    if (gridVisible == newGridVisible)
        return;
    gridVisible = newGridVisible;
    emit gridVisibleChanged();
}

void MapperWidget::showCoordinates(bool show)
{
    if (mustShowCoordinates == show)
        return;
    mustShowCoordinates = show;
    update();
}

size_t MapperWidget::getCommandsIndex() const
{
    return commandsIndex;
}

size_t MapperWidget::getCommandsHistorySize() const
{
    return commandsHistory.size();
}

void MapperWidget::moveContentUp()
{
    moveContent(0, 1);
}

void MapperWidget::moveContentDown()
{
    moveContent(0, -1);
}

void MapperWidget::moveContentLeft()
{
    moveContent(1, 0);
}

void MapperWidget::moveContentRight()
{
    moveContent(-1, 0);
}

void MapperWidget::commandExecute(std::unique_ptr<MapperCommand> cmd)
{
    commandsHistory.insert(commandsHistory.begin() + commandsIndex, std::move(cmd));
    commandsHistory.resize(commandsIndex + 1);

    if (commandsHistory.size() >= MaxCommandsCount)
        commandsHistory.erase(commandsHistory.begin());
    else
        commandsIndex++;

    commandsHistory[commandsIndex - 1]->execute();

    emit commandsIndexChanged();
    update();
}

void MapperWidget::commandUndo()
{
    if (commandsIndex > 0)
    {
        commandsHistory[commandsIndex - 1]->undo();
        commandsIndex--;
        emit commandsIndexChanged();
        update();
    }
}

void MapperWidget::commandRedo()
{
    if (commandsIndex < commandsHistory.size())
    {
        commandsHistory[commandsIndex]->execute();
        commandsIndex++;
        emit commandsIndexChanged();
        update();
    }
}

void MapperWidget::commandClear()
{
    commandsHistory.clear();
    commandsIndex = 0;
    emit commandsIndexChanged();
    update();
}

void MapperWidget::moveContent(int x, int y)
{
    if (x == y)
        return;

    if (x != 0 && y != 0)
        return;

    if (x > 1 || x < -1)
        return;

    if (y > 1 || y < -1)
        return;

    std::vector<std::unique_ptr<Tile>*> previousTiles;
    std::vector<std::unique_ptr<Tile>>  newTiles;

    std::vector<std::unique_ptr<SpecialTileType>*> previousSpecialTiles;
    std::vector<std::unique_ptr<SpecialTileType>>  newSpecialTiles;

    std::vector<std::unique_ptr<Event>*> previousEvents;
    std::vector<std::unique_ptr<Event>>  newEvents;

    auto push = [&map = this->map, x, y](auto& layer, auto& previousArray, auto& newArray, size_t i, size_t j) {
        auto previous = &(*layer.get())(i, j);
        if (x == -1 && i == 0)
        {
            newArray.push_back(nullptr);
        }
        else if (x == 1 && i == map->getNCol() - 1)
        {
            newArray.push_back(nullptr);
        }
        else if (y == -1 && j == 0)
        {
            newArray.push_back(nullptr);
        }
        else if (y == 1 && j == map->getNRow() - 1)
        {
            newArray.push_back(nullptr);
        }
        else if (auto& next = (*layer.get())(i + x, j + y); next)
        {
            auto e = clone(next);
            newArray.push_back(std::move(e));
        }
        else
        {
            newArray.push_back(nullptr);
        }
        previousArray.push_back(previous);
    };

    for (size_t l = 0; l < map->getLevels().size(); ++l)
    {
        auto& level = map->getLevels()[l];

        for (size_t i = 0; i < map->getNCol(); ++i)
        {
            for (size_t j = 0; j < map->getNRow(); ++j)
            {
                for (size_t h = 0; h < level->getTileLayers().size(); ++h)
                {
                    auto& layer = level->getTileLayers()[h];
                    push(layer, previousTiles, newTiles, i, j);
                }

                auto& specialTileLayer = level->getSpecialTileLayer();
                push(specialTileLayer, previousSpecialTiles, newSpecialTiles, i, j);

                auto& eventLayer = level->getEventLayer();
                push(eventLayer, previousEvents, newEvents, i, j);
            }
        }
    }

    auto cmd = std::make_unique<ParallelCommand>();
    cmd->addCommand(std::make_unique<ChangesCommand<Tile>>(previousTiles, std::move(newTiles)));
    cmd->addCommand(
        std::make_unique<ChangesCommand<SpecialTileType>>(previousSpecialTiles, std::move(newSpecialTiles)));
    cmd->addCommand(std::make_unique<ChangesCommand<Event>>(previousEvents, std::move(newEvents)));
    commandExecute(std::move(cmd));
}
