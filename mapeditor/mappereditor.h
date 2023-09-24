#ifndef MAPPEREDITOR_H
#define MAPPEREDITOR_H

#include "encounterseditor.h"
#include "mapperviewer.h"

class MapperEditor : public QWidget
{
public:
    MapperEditor(QWidget* parent = nullptr);

    void swapMap(std::unique_ptr<Map>&& newMap);

    MapperViewer*     getMapperViewer() const;
    EncountersEditor* getEncountersEditor() const;

private:
    MapperViewer*     mapperViewer;
    EncountersEditor* encountersEditor;
};

#endif // MAPPEREDITOR_H
