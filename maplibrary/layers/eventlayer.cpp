#include "eventlayer.h"

EventLayer::EventLayer(int nCol, int nRow) : Layer(nCol, nRow)
{
}

void tag_invoke(js::value_from_tag, js::value& jv, std::unique_ptr<EventLayer> const& o)
{
    jv = {{"elements", js::value_from<std::vector<std::vector<Event::EventPtr>> const&>(o->elements)},
          {"nCol", o->nCol},
          {"nRow", o->nRow}};
}

std::unique_ptr<EventLayer> tag_invoke(js::value_to_tag<std::unique_ptr<EventLayer>>, js::value const& jv)
{
    js::object const&           obj = jv.as_object();
    std::unique_ptr<EventLayer> o =
        std::make_unique<EventLayer>(js::value_to<int>(obj.at("nCol")), js::value_to<int>(obj.at("nRow")));
    o->elements = js::value_to<std::vector<std::vector<Event::EventPtr>>>(obj.at("elements"));
    return o;
}
