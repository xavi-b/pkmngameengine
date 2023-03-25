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

QPixmap ImageWidget::currentSelectionImage() const
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
        pressed           = true;
        float scaleFactor = 1.0 * width() / pixmap(Qt::ReturnByValue).width();

        int selSize  = selectionSize * scaleFactor;
        int col      = event->pos().x() / selSize * selSize;
        int row      = event->pos().y() / selSize * selSize;
        pressPoint   = {col, row};
        releasePoint = {0, 0};

        rubberBand->setGeometry(QRect(pressPoint, QSize(selSize, selSize)));
        event->accept();
    }
    else
    {
        pressed = false;
    }
}

void ImageWidget::mouseMoveEvent(QMouseEvent* event)
{
    if (pressed)
    {
        float scaleFactor = 1.0 * width() / pixmap(Qt::ReturnByValue).width();
        int   selSize     = selectionSize * scaleFactor;

        int    col    = (event->pos().x() / selSize + 1) * selSize;
        int    row    = (event->pos().y() / selSize + 1) * selSize;
        QPoint origin = {col, row};

        rubberBand->setGeometry(QRect(pressPoint, origin));
        event->accept();
    }
}

void ImageWidget::mouseReleaseEvent(QMouseEvent* event)
{
    if (pressed)
    {
        pressed = false;

        float scaleFactor = 1.0 * width() / pixmap(Qt::ReturnByValue).width();
        int   selSize     = selectionSize * scaleFactor;

        int col      = (event->pos().x() / selSize + 1) * selSize;
        int row      = (event->pos().y() / selSize + 1) * selSize;
        releasePoint = {col, row};

        rubberBand->setGeometry(QRect(pressPoint, releasePoint));
        event->accept();
    }
}

void ImageWidget::resizeEvent(QResizeEvent* event)
{
    float scaleFactor     = 1.0 * width() / pixmap(Qt::ReturnByValue).width();
    float scaleDifference = 1.0 * event->size().width() / event->oldSize().width();

    int selSize = selectionSize * scaleFactor;
    pressPoint.setX(pressPoint.x() * scaleDifference / selSize * selSize);
    pressPoint.setY(pressPoint.y() * scaleDifference / selSize * selSize);
    releasePoint.setX(releasePoint.x() * scaleDifference / selSize * selSize);
    releasePoint.setY(releasePoint.y() * scaleDifference / selSize * selSize);

    rubberBand->setGeometry(QRect(pressPoint, releasePoint));
}
