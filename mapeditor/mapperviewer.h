#ifndef MAPPERVIEWER_H
#define MAPPERVIEWER_H

#include "mapperwidget.h"

#include <QLabel>
#include <QScrollArea>
#include <QScrollBar>
#include <QWheelEvent>

class MapperViewer : public QScrollArea
{
    Q_OBJECT

public:
    MapperViewer(QWidget* parent = nullptr);

    void scaleContent(double factor);
    void adjustScrollBar(QScrollBar* scrollBar, double factor);

    MapperWidget* contentWidget() const;

    QPixmap grabInternalImage() const;

signals:
    void entered();

protected:
    virtual void wheelEvent(QWheelEvent* event) override;
    virtual void mousePressEvent(QMouseEvent* event) override;
    virtual void mouseMoveEvent(QMouseEvent* event) override;
    virtual void mouseReleaseEvent(QMouseEvent* event) override;

private:
    MapperWidget* mapperWidget;
    float         scaleFactor = 1.0;
    QPoint        dragOrigin;
};

#endif // MAPPERVIEWER_H
