#include "imageviewer.h"

ImageViewer::ImageViewer(QWidget* parent) : QScrollArea(parent)
{
    setMinimumSize(400, 400);
    imageWidget = new ImageWidget;
    setWidget(imageWidget);
}

void ImageViewer::setImage(const QImage& image)
{
    imageWidget->setPixmap(QPixmap::fromImage(image));
    imageWidget->adjustSize();

    scaleFactor = 1.0;
}

void ImageViewer::scaleImage(double factor)
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

void ImageViewer::wheelEvent(QWheelEvent* event)
{
    if (event->modifiers() & Qt::ControlModifier)
    {
        if (event->pixelDelta().y() > 0)
        {
            if (scaleFactor < 3.0)
                scaleImage(1.25);
            event->accept();
            return;
        }
        else if (event->pixelDelta().y() < 0)
        {
            if (scaleFactor > 0.333)
                scaleImage(0.8);
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
