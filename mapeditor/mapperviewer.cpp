#include "mapperviewer.h"

MapperViewer::MapperViewer(QWidget* parent) : QScrollArea(parent)
{
    setMinimumSize(400, 400);
    mapperWidget = new MapperWidget;
    setWidget(mapperWidget);
    mapperWidget->resize(scaleFactor * mapperWidget->tileSizeInPixels());
}

void MapperViewer::scaleContent(double factor)
{
    scaleFactor *= factor;
    mapperWidget->resize(scaleFactor * mapperWidget->tileSizeInPixels());

    adjustScrollBar(horizontalScrollBar(), factor);
    adjustScrollBar(verticalScrollBar(), factor);
}

void MapperViewer::adjustScrollBar(QScrollBar* scrollBar, double factor)
{
    scrollBar->setValue(int(factor * scrollBar->value() + ((factor - 1) * scrollBar->pageStep() / 2)));
}

QPixmap MapperViewer::grabInternalImage() const
{
    mapperWidget->setGridVisible(false);
    auto pix = mapperWidget->grab();
    mapperWidget->setGridVisible(true);
    return pix;
}

MapperWidget* MapperViewer::contentWidget() const
{
    return mapperWidget;
}

void MapperViewer::wheelEvent(QWheelEvent* event)
{
    if (event->modifiers() & Qt::ControlModifier)
    {
        if (event->pixelDelta().y() > 0)
        {
            if (scaleFactor < 3.0)
                scaleContent(1.25);
            event->accept();
            return;
        }
        else if (event->pixelDelta().y() < 0)
        {
            if (scaleFactor > 0.333)
                scaleContent(0.8);
            event->accept();
            return;
        }
    }
    else if (event->modifiers() & Qt::ShiftModifier)
    {
        horizontalScrollBar()->setValue(horizontalScrollBar()->value() + event->angleDelta().y());
        event->accept();
        return;
    }
    QScrollArea::wheelEvent(event);
}

void MapperViewer::mousePressEvent(QMouseEvent* event)
{
    if (event->buttons() & Qt::MiddleButton)
    {
        dragOrigin = event->pos();
        setCursor(Qt::ClosedHandCursor);
        event->accept();
    }
    else
    {
        event->ignore();
    }
}

void MapperViewer::mouseMoveEvent(QMouseEvent* event)
{
    if (event->buttons() & Qt::MiddleButton)
    {
        horizontalScrollBar()->setValue(horizontalScrollBar()->value() - (event->pos().x() - dragOrigin.x()));
        verticalScrollBar()->setValue(verticalScrollBar()->value() - (event->pos().y() - dragOrigin.y()));
        dragOrigin = event->pos();
        event->accept();
    }
    else
    {
        event->ignore();
    }
}

void MapperViewer::mouseReleaseEvent(QMouseEvent* event)
{
    dragOrigin = event->pos();
    setCursor(Qt::ArrowCursor);
    event->accept();
}

void MapperViewer::keyPressEvent(QKeyEvent* event)
{
    if (event->key() == Qt::Key_Escape)
    {
        mapperWidget->setSelectionPixmap({});
        event->accept();
        return;
    }
    else if (event->modifiers() & (Qt::ControlModifier | Qt::ShiftModifier))
    {
        if (event->key() == Qt::Key_Up)
        {
            mapperWidget->moveContentUp();
            event->accept();
            return;
        }
        else if (event->key() == Qt::Key_Down)
        {
            mapperWidget->moveContentDown();
            event->accept();
            return;
        }
        else if (event->key() == Qt::Key_Left)
        {
            mapperWidget->moveContentLeft();
            event->accept();
            return;
        }
        else if (event->key() == Qt::Key_Right)
        {
            mapperWidget->moveContentRight();
            event->accept();
            return;
        }
    }
    event->ignore();
}
