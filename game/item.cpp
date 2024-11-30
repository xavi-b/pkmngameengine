#include "item.h"

#include "game.h"

Item::Item(ItemDef::ItemDefPtr definition, size_t count) : definition(definition), count(count)
{
}


ItemDef::ItemDefPtr Item::getDefinition() const
{
    return definition;
}


size_t Item::getCount() const
{
    return count;
}

void tag_invoke(js::value_from_tag, js::value& jv, Item::ItemPtr const& o)
{
    if (o)
    {
        jv = {
            {"count",      o->count              },
            {"definition", o->definition->getId()}
        };
    }
    else
    {
        jv = {};
    }
}

Item::ItemPtr tag_invoke(js::value_to_tag<Item::ItemPtr>, js::value const& jv)
{
    js::object const& obj = jv.as_object();
    if (obj.empty())
        return nullptr;

    auto   definition = Game::instance()->data.itemDefFor(js::value_to<std::string>(obj.at("definition")));
    size_t count      = js::value_to<size_t>(obj.at("count"));
    auto   item       = std::make_shared<Item>(definition, count);
    return item;
}
