#include "imagewidget.h"

ImageWidget::ImageWidget(QWidget* parent) : QLabel(parent)
{
    setAlignment(Qt::AlignTop | Qt::AlignLeft);
    setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Ignored);
    setScaledContents(true);

    rubberBand = new QRubberBand(QRubberBand::Rectangle, this);
    rubberBand->setGeometry(QRect({0, 0}, QSize(selectionSize, selectionSize)));
    rubberBand->show();
}

QPixmap ImageWidget::currentSelectionPixmap() const
{
    float scaleFactor = 1.0 * width() / pixmap(Qt::ReturnByValue).width();
    QRect selection   = rubberBand->geometry();
    QRect rect        = QRect(selection.topLeft() * scaleFactor, selection.size() * scaleFactor);
    return pixmap().copy(rect);
}

void ImageWidget::mousePressEvent(QMouseEvent* event)
{
    if (event->buttons() & Qt::LeftButton)
    {
        float scaleFactor = 1.0 * width() / pixmap(Qt::ReturnByValue).width();
        float selSize     = selectionSize * scaleFactor;
        int   col         = int(event->pos().x() * scaleFactor) / scaleFactor / selSize;
        int   row         = int(event->pos().y() * scaleFactor) / scaleFactor / selSize;
        pressPoint        = {col, row};
        releasePoint      = {0, 0};
        QSize size        = {1 + releasePoint.x() - pressPoint.x(), 1 + releasePoint.y() - pressPoint.y()};

        rubberBand->setGeometry(QRect(pressPoint * selSize, size * selSize));
        event->accept();
    }
    else
    {
        event->ignore();
    }
}

void ImageWidget::mouseMoveEvent(QMouseEvent* event)
{
    if (event->buttons() & Qt::LeftButton)
    {
        float scaleFactor = 1.0 * width() / pixmap(Qt::ReturnByValue).width();
        float selSize     = selectionSize * scaleFactor;
        int   col         = int(event->pos().x() * scaleFactor) / scaleFactor / selSize;
        int   row         = int(event->pos().y() * scaleFactor) / scaleFactor / selSize;
        releasePoint      = {col, row};
        QSize size        = {1 + releasePoint.x() - pressPoint.x(), 1 + releasePoint.y() - pressPoint.y()};

        rubberBand->setGeometry(QRect(pressPoint * selSize, size * selSize));
        event->accept();
    }
    else
    {
        event->ignore();
    }
}

void ImageWidget::mouseReleaseEvent(QMouseEvent* event)
{
    if (event->buttons() & Qt::LeftButton)
    {
        float scaleFactor = 1.0 * width() / pixmap(Qt::ReturnByValue).width();
        float selSize     = selectionSize * scaleFactor;
        int   col         = int(event->pos().x() * scaleFactor) / scaleFactor / selSize;
        int   row         = int(event->pos().y() * scaleFactor) / scaleFactor / selSize;
        releasePoint      = {col, row};
        QSize size        = {1 + releasePoint.x() - pressPoint.x(), 1 + releasePoint.y() - pressPoint.y()};

        rubberBand->setGeometry(QRect(pressPoint * selSize, size * selSize));
        event->accept();
    }
    else
    {
        event->ignore();
    }
}

void ImageWidget::resizeEvent(QResizeEvent* event)
{
    float scaleFactor = 1.0 * width() / pixmap(Qt::ReturnByValue).width();
    float selSize     = selectionSize * scaleFactor;
    QSize size        = {1 + releasePoint.x() - pressPoint.x(), 1 + releasePoint.y() - pressPoint.y()};

    rubberBand->setGeometry(QRect(pressPoint * selSize, size * selSize));
    event->accept();
}
