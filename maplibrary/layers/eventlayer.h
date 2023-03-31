#ifndef EVENTLAYER_H
#define EVENTLAYER_H

#include <vector>
#include <memory>
#include "event.h"
#include "layer.h"

class EventLayer : public Layer<Event::EventPtr>
{
public:
    EventLayer(size_t nCol, size_t nRow);

    friend void tag_invoke(js::value_from_tag, js::value& jv, std::unique_ptr<EventLayer> const& o);
    friend std::unique_ptr<EventLayer> tag_invoke(js::value_to_tag<std::unique_ptr<EventLayer>>, js::value const& jv);
};

#endif // LAYER_H
