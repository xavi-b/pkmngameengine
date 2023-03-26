#ifndef MAPPERVIEWER_H
#define MAPPERVIEWER_H

#include <QScrollArea>
#include <QScrollBar>
#include <QLabel>
#include <QWheelEvent>
#include "mapperwidget.h"

class MapperViewer : public QScrollArea
{
    Q_OBJECT
public:
    MapperViewer(QWidget* parent = nullptr);

    void scaleContent(double factor);
    void adjustScrollBar(QScrollBar* scrollBar, double factor);

    MapperWidget* contentWidget() const;

signals:
    void entered();

protected:
    virtual void wheelEvent(QWheelEvent* event) override;
    virtual void mousePressEvent(QMouseEvent* event) override;
    virtual void mouseMoveEvent(QMouseEvent* event) override;
    virtual void mouseReleaseEvent(QMouseEvent* event) override;
    virtual void enterEvent(QEnterEvent* event) override;

private:
    MapperWidget* mapperWidget;
    float         scaleFactor = 1.0;
    QPoint        dragOrigin;
};

#endif // MAPPERVIEWER_H
