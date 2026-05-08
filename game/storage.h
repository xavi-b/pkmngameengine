#ifndef STORAGE_H
#define STORAGE_H

#include "pkmn.h"

#include <string>

class Box
{
public:
    Box();

    friend void tag_invoke(js::value_from_tag, js::value& jv, Box const& o);
    friend Box  tag_invoke(js::value_to_tag<Box>, js::value const& jv);

    std::vector<Pkmn::PkmnPtr> pkmns;
};

class Storage
{
public:
    Storage();

    void addPkmn(Pkmn::PkmnPtr const& pkmn);

    friend void    tag_invoke(js::value_from_tag, js::value& jv, Storage const& o);
    friend Storage tag_invoke(js::value_to_tag<Storage>, js::value const& jv);

private:
    std::vector<Box> boxes;
};

#endif // STORAGE_H
