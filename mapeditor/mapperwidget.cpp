#include "mapperwidget.h"

MapperWidget::MapperWidget(QWidget* parent) : QWidget(parent)
{
    setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Ignored);
    setMouseTracking(true);

    rubberBand = new QRubberBand(QRubberBand::Rectangle, this);
    rubberBand->setGeometry(QRect({0, 0}, QSize(tilePixelSize, tilePixelSize)));
    rubberBand->show();
}

void MapperWidget::setPixmap(const QPixmap& pixmap)
{
    this->pixmap = pixmap;
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

void MapperWidget::mousePressEvent(QMouseEvent* event)
{
}

void MapperWidget::mouseMoveEvent(QMouseEvent* event)
{
    float scaleFactor = 1.0 * width() / tileSizeInPixels().width();
    int   selSize     = tilePixelSize * scaleFactor;

    if (event->pos().x() < selSize * gridSize.width() && event->pos().y() < selSize * gridSize.height())
    {
        int    col    = event->pos().x() / selSize * selSize;
        int    row    = event->pos().y() / selSize * selSize;
        QPoint origin = {col, row};

        rubberBand->setGeometry(QRect(origin, QSize(selSize, selSize)));
        event->accept();
    }
}

void MapperWidget::mouseReleaseEvent(QMouseEvent* event)
{
}

void MapperWidget::resizeEvent(QResizeEvent* event)
{
    float scaleFactor = 1.0 * width() / tileSizeInPixels().width();

    QPoint pos = mapFromGlobal(QCursor::pos());

    int    selSize = tilePixelSize * scaleFactor;
    int    col     = pos.x() / selSize * selSize;
    int    row     = pos.y() / selSize * selSize;
    QPoint origin  = {col, row};

    rubberBand->setGeometry(QRect(origin, QSize(selSize, selSize)));
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
            painter.drawRect(QRect({i * selSize, j * selSize}, QSize(selSize - 1, selSize - 1)));
        }
    }
}
