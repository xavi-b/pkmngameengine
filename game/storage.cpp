#include "storage.h"

Box::Box()
{
}

void tag_invoke(js::value_from_tag, js::value& jv, Box const& o)
{
    jv = {
        {"pkmns", js::value_from<std::vector<Pkmn::PkmnPtr> const&>(o.pkmns)}
    };
}

Box tag_invoke(js::value_to_tag<Box>, js::value const& jv)
{
    Box box;
    js::object const& obj = jv.as_object();
    box.pkmns = js::value_to<std::vector<Pkmn::PkmnPtr>>(obj.at("pkmns"));
    return box;
}

Storage::Storage()
{
}

void Storage::addPkmn(Pkmn::PkmnPtr const& pkmn)
{
    for (auto& box : boxes)
    {
        if (box.pkmns.size() < 30)
        {
            box.pkmns.push_back(pkmn);
            return;
        }
    }
    boxes.push_back(Box());
    boxes.back().pkmns.push_back(pkmn);
}

void tag_invoke(js::value_from_tag, js::value& jv, Storage const& o)
{
    jv = {
        {"boxes", js::value_from<std::vector<Box> const&>(o.boxes)}
    };
}

Storage tag_invoke(js::value_to_tag<Storage>, js::value const& jv)
{
    Storage storage;
    js::object const& obj = jv.as_object();
    storage.boxes = js::value_to<std::vector<Box>>(obj.at("boxes"));
    return storage;
}