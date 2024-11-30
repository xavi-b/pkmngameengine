#ifndef ITEM_H
#define ITEM_H

#include "itemdef.h"

class Item
{
public:
    using ItemPtr = std::shared_ptr<Item>;

    friend void    tag_invoke(js::value_from_tag, js::value& jv, ItemPtr const& o);
    friend ItemPtr tag_invoke(js::value_to_tag<ItemPtr>, js::value const& jv);

    Item(ItemDef::ItemDefPtr definition, size_t count = 0);

    ItemDef::ItemDefPtr getDefinition() const;
    size_t              getCount() const;

private:
    ItemDef::ItemDefPtr definition;
    size_t              count = 0;
};

#endif // ITEM_H
