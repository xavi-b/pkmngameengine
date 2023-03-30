#ifndef ENCOUNTERSLAYER_H
#define ENCOUNTERSLAYER_H

#include <vector>
#include <memory>
#include "layer.h"

enum EncounterType
{
    NONE,
    GRASS,
    WATER
};

class EncountersLayer : public Layer<EncounterType>
{
public:
    EncountersLayer(int nCol, int nRow);
};

#endif // ENCOUNTERSLAYER_H
