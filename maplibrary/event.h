#ifndef EVENT_H
#define EVENT_H

#include <boost/json.hpp>
#include <memory>
#include <string>
#include <vector>

namespace js = boost::json;

class Event
{
public:
    using EventPtr = std::unique_ptr<Event>;

    Event(std::string const& id);

    std::string getId() const;

    friend void     tag_invoke(js::value_from_tag, js::value& jv, EventPtr const& o);
    friend EventPtr tag_invoke(js::value_to_tag<EventPtr>, js::value const& jv);

    friend Event::EventPtr clone(Event::EventPtr const& other);

private:
    std::string id;
};

#endif // EVENT_H
