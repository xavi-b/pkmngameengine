#include "mapperviewer.h"

MapperViewer::MapperViewer(QWidget* parent) : QScrollArea(parent)
{
    setMinimumSize(400, 400);
    mapperWidget = new MapperWidget;
    setWidget(mapperWidget);
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
