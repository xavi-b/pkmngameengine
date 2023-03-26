#include "mapperwidget.h"

MapperWidget::MapperWidget(QWidget* parent) : QWidget(parent)
{
    setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Ignored);
    setMouseTracking(true);
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
        int col = event->pos().x() / selSize * selSize;
        int row = event->pos().y() / selSize * selSize;
        origin  = {col, row};
        event->accept();
        update();
    }
}

void MapperWidget::mouseReleaseEvent(QMouseEvent* event)
{
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
            painter.drawRect(QRect({i * selSize, j * selSize}, QSize(selSize - 1, selSize - 1)));
        }
    }

    painter.drawPixmap(QRect(origin, pixmap.size() * scaleFactor), pixmap);
}
