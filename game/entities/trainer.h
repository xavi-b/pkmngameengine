#ifndef TRAINER_H
#define TRAINER_H

#include "entity.h"
#include "pkmn.h"

#include <array>

class Trainer : public Entity
{
public:
    using TrainerPtr = std::shared_ptr<Trainer>;

    enum class Gender
    {
        BOY,
        GIRL
    };

    friend void    tag_invoke(js::value_from_tag, js::value& jv, Trainer const& o);
    friend Trainer tag_invoke(js::value_to_tag<Trainer>, js::value const& jv);

    Trainer();

    size_t getPkmnCount() const;

    Gender getGender() const;
    void   setGender(Gender newGender);

    std::string name;

    std::array<Pkmn::PkmnPtr, 6> pkmns;

private:
    Gender gender = Gender::BOY;
};

#endif // TRAINER_H
