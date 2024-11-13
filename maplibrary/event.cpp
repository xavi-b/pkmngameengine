#include "event.h"

Event::Event(std::string const& id) : id(id)
{
}

std::string Event::getId() const
{
    return id;
}

void tag_invoke(js::value_from_tag, js::value& jv, Event::EventPtr const& o)
{
    if (o && !o->id.empty())
    {
        jv = {
            {"id", o->id}
        };
    }
    else
    {
        jv = {};
    }
}

Event::EventPtr tag_invoke(js::value_to_tag<Event::EventPtr>, js::value const& jv)
{
    js::object const& obj = jv.as_object();

    if (obj.empty())
        return nullptr;

    std::string id = js::value_to<std::string>(obj.at("id"));
    if (id.empty())
        return nullptr;
    else
        return std::make_unique<Event>(id);
}

Event::EventPtr clone(Event::EventPtr const& other)
{
    return Event::EventPtr(new Event(*other.get()));
}
