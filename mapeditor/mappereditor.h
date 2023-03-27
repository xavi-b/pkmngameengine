#ifndef MAPPEREDITOR_H
#define MAPPEREDITOR_H

#include "mapperviewer.h"

class MapperEditor : public QWidget
{
public:
    MapperEditor(QWidget* parent = nullptr);

    MapperViewer* viewer() const;

private:
    MapperViewer* mapperViewer;
};

#endif // MAPPEREDITOR_H
