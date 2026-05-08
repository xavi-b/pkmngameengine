#include "trainer.h"

#include <algorithm>

Trainer::Trainer()
{
}

size_t Trainer::getPkmnCount() const
{
    return std::count_if(pkmns.begin(), pkmns.end(), [](auto e) {
        return e != nullptr;
    });
}

Trainer::Gender Trainer::getGender() const
{
    return gender;
}

void Trainer::setGender(Gender newGender)
{
    gender = newGender;
}

void tag_invoke(js::value_from_tag, js::value& jv, Trainer const& o)
{
    jv = {
        {"name", o.name},
        {"gender", static_cast<int>(o.getGender())},
        {"pkmns", js::value_from<std::array<Pkmn::PkmnPtr, 6> const&>(o.pkmns)}
    };
}

Trainer tag_invoke(js::value_to_tag<Trainer>, js::value const& jv)
{
    Trainer           t;
    js::object const& obj = jv.as_object();
    t.name                = js::value_to<std::string>(obj.at("name"));
    t.gender              = static_cast<Trainer::Gender>(js::value_to<int>(obj.at("gender")));
    t.pkmns               = js::value_to<std::array<Pkmn::PkmnPtr, 6>>(obj.at("pkmns"));
    return t;
}
