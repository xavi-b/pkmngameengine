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

protected:
    virtual void wheelEvent(QWheelEvent* event) override;

private:
    MapperWidget* mapperWidget;
    float         scaleFactor = 1.0;
};

#endif // MAPPERVIEWER_H
