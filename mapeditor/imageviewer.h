#ifndef IMAGEVIEWER_H
#define IMAGEVIEWER_H

#include "imagewidget.h"

#include <QLabel>
#include <QScrollArea>
#include <QScrollBar>
#include <QWheelEvent>

class ImageViewer : public QScrollArea
{
    Q_OBJECT

public:
    ImageViewer(QWidget* parent = nullptr);

    void setPixmap(QString const& pixmap);
    void scaleContent(double factor);
    void adjustScrollBar(QScrollBar* scrollBar, double factor);

    ImageWidget* contentWidget() const;

protected:
    virtual void wheelEvent(QWheelEvent* event) override;
    virtual void mousePressEvent(QMouseEvent* event) override;
    virtual void mouseMoveEvent(QMouseEvent* event) override;
    virtual void mouseReleaseEvent(QMouseEvent* event) override;

private:
    ImageWidget* imageWidget;
    float        scaleFactor = 1.0;
    QPoint       dragOrigin;
};

#endif // IMAGEVIEWER_H
