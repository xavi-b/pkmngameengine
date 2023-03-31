#include "event.h"

Event::Event()
{
}

void tag_invoke(js::value_from_tag, js::value& jv, Event::EventPtr const& o)
{
    if (o)
    {
        jv = {};
    }
    else
    {
        jv = {};
    }
}

Event::EventPtr tag_invoke(js::value_to_tag<Event::EventPtr>, js::value const& jv)
{
    Event::EventPtr   o;
    js::object const& obj = jv.as_object();
    if (obj.empty())
        return nullptr;
    else
        return std::make_unique<Event>();
}
