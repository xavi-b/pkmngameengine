#include "imageviewer.h"

ImageViewer::ImageViewer(QWidget* parent) : QScrollArea(parent)
{
    setMinimumSize(400, 400);
    imageWidget = new ImageWidget;
    setWidget(imageWidget);
}

void ImageViewer::setPixmap(QString const& pixmap)
{
    imageWidget->setPixmap(pixmap);
    imageWidget->adjustSize();

    scaleFactor = 1.0;
}

void ImageViewer::scaleContent(double factor)
{
    scaleFactor *= factor;
    imageWidget->resize(scaleFactor * imageWidget->pixmap(Qt::ReturnByValue).size());

    adjustScrollBar(horizontalScrollBar(), factor);
    adjustScrollBar(verticalScrollBar(), factor);
}

void ImageViewer::adjustScrollBar(QScrollBar* scrollBar, double factor)
{
    scrollBar->setValue(int(factor * scrollBar->value() + ((factor - 1) * scrollBar->pageStep() / 2)));
}

ImageWidget* ImageViewer::contentWidget() const
{
    return imageWidget;
}

void ImageViewer::wheelEvent(QWheelEvent* event)
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

void ImageViewer::mousePressEvent(QMouseEvent* event)
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

void ImageViewer::mouseMoveEvent(QMouseEvent* event)
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

void ImageViewer::mouseReleaseEvent(QMouseEvent* event)
{
    dragOrigin = event->pos();
    setCursor(Qt::ArrowCursor);
    event->accept();
}
