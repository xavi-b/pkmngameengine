#ifndef EVENT_H
#define EVENT_H

#include <vector>
#include <memory>
#include <string>

#include <boost/json.hpp>
namespace js = boost::json;

class Event
{
public:
    using EventPtr = std::unique_ptr<Event>;

    Event();

    friend void     tag_invoke(js::value_from_tag, js::value& jv, EventPtr const& o);
    friend EventPtr tag_invoke(js::value_to_tag<EventPtr>, js::value const& jv);
};

#endif // EVENT_H
