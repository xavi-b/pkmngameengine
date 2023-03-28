#ifndef EVENTLAYER_H
#define EVENTLAYER_H

#include <vector>
#include <memory>
#include "event.h"
#include "layer.h"

class EventLayer : public Layer<Event>
{
public:
    EventLayer(int nCol, int nRow);
};

#endif // LAYER_H
