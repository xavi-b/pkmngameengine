#include "pkmn.h"

#include "game.h"

Pkmn::Pkmn(PkmnDef::PkmnDefPtr definition, size_t level) : definition(definition), level(level)
{
    happiness = definition->getHappiness();
}

std::map<PkmnDef::Stat, size_t> Pkmn::getStats()
{
    std::map<PkmnDef::Stat, size_t> baseStats = definition->getBaseStats();
    std::map<PkmnDef::Stat, size_t> stats;
    stats[PkmnDef::HP] =
        (((2 * baseStats[PkmnDef::HP] + IVs[PkmnDef::HP] + EVs[PkmnDef::HP] / 4) * level) / 100) + level + 10;
    for (size_t i = static_cast<int>(PkmnDef::HP) + 1; i < PkmnDef::StatCount; ++i)
    {
        PkmnDef::Stat stat = static_cast<PkmnDef::Stat>(i);
        stats[PkmnDef::HP] = ((((2 * baseStats[stat] + IVs[stat] + EVs[stat] / 4) * level) / 100) + 5); // TODO * nature
    }
    return stats;
}

std::map<PkmnDef::Stat, size_t> const& Pkmn::getIVs() const
{
    return IVs;
}

void Pkmn::setIVs(std::map<PkmnDef::Stat, size_t> const& newIVs)
{
    IVs = newIVs;
}

std::map<PkmnDef::Stat, size_t> const& Pkmn::getEVs() const
{
    return EVs;
}

void Pkmn::setEVs(std::map<PkmnDef::Stat, size_t> const& newEVs)
{
    EVs = newEVs;
}

std::array<Move::MovePtr, 4> const& Pkmn::getMoves() const
{
    return moves;
}

std::array<Move::MovePtr, 4>& Pkmn::getMoves()
{
    return moves;
}

void Pkmn::setMoves(std::array<Move::MovePtr, 4> const& newMoves)
{
    moves = newMoves;
}

size_t Pkmn::getLevel() const
{
    return level;
}

void Pkmn::incrementLevel()
{
    ++level;
    if (happiness <= 99)
        happiness += 5;
    else if (happiness <= 199)
        happiness += 3;
    else
        happiness += 2;
}

Pkmn::StatusCondition Pkmn::getStatusCondition() const
{
    return statusCondition;
}

PkmnDef::PkmnDefPtr Pkmn::getDefinition() const
{
    return definition;
}

size_t Pkmn::getHP() const
{
    return hp;
}

bool Pkmn::isKO() const
{
    return hp == 0;
}

void Pkmn::setHP(size_t newHp)
{
    hp = newHp;
}

void Pkmn::decreaseHP(size_t n)
{
    if (n > hp)
        hp = 0;
    else
        hp = hp - n;
}

void Pkmn::resetHP()
{
    hp = getStats()[PkmnDef::HP];
}

size_t Pkmn::expToNextLevel()
{
    // https://bulbapedia.bulbagarden.net/wiki/Experience#Relation_to_level
    switch (definition->getGrowthRate())
    {
    case PkmnDef::ERRATIC:
        if (level < 50)
            return level * level * level * (100 - level) / 50;
        else if (level < 68)
            return level * level * level * (150 - level) / 100;
        else if (level < 98)
            return level * level * level * (1911 - 10 * level) / 3 / 500;
        else
            return level * level * level * (160 - level) / 100;
    case PkmnDef::FAST:
        return 4 * level * level * level / 5;
    case PkmnDef::MEDIUM_FAST:
        return level * level * level;
    case PkmnDef::MEDIUM_SLOW:
        return 6 * level * level * level / 5 - 15 * level * level + 100 * level - 140;
    case PkmnDef::SLOW:
        return 5 * level * level * level / 4;
    case PkmnDef::FLUCTUATING:
        if (level < 15)
            return level * level * level * (((level + 1) / 3)) + 24 / 50;
        else if (level < 36)
            return level * level * level * (level + 14) / 50;
        else
            return level * level * level * (level / 2 + 32) / 50;
    case PkmnDef::__SIZE_GROWTH_RATE:
        return 1;
    }
}

size_t Pkmn::getExp() const
{
    return exp;
}

void Pkmn::setExp(size_t newExp)
{
    exp = newExp;
}

void Pkmn::increaseExp(size_t newExp)
{
    exp += newExp;
}

unsigned char Pkmn::getHappiness() const
{
    return happiness;
}

void Pkmn::setHappiness(unsigned char newHappiness)
{
    happiness = newHappiness;
}

void tag_invoke(js::value_from_tag, js::value& jv, Pkmn::PkmnPtr const& o)
{
    if (o)
    {
        js::array jsIVs;
        for (auto const& e : o->IVs)
            jsIVs.push_back(js::value{
                {"id",    e.first },
                {"value", e.second}
            });
        js::array jsEVs;
        for (auto const& e : o->EVs)
            jsEVs.push_back(js::value{
                {"id",    e.first },
                {"value", e.second}
            });
        jv = {
            {"moves", js::value_from<std::array<Move::MovePtr, 4> const&>(o->moves)},
            {"level", o->level},
            {"exp", o->exp},
            {"hp", o->hp},
            {"nickName", o->nickName},
            {"definition", o->definition->getId()},
            {"IVs", jsIVs},
            {"EVs", jsEVs},
            {"statusCondition", o->statusCondition},
            {"happiness", o->happiness}
        };
    }
    else
    {
        jv = {};
    }
}

Pkmn::PkmnPtr tag_invoke(js::value_to_tag<Pkmn::PkmnPtr>, js::value const& jv)
{
    js::object const& obj = jv.as_object();
    if (obj.empty())
        return nullptr;

    auto   definition     = Game::instance()->data.pkmnDefFor(js::value_to<std::string>(obj.at("definition")));
    size_t level          = js::value_to<size_t>(obj.at("level"));
    auto   pkmn           = std::make_shared<Pkmn>(definition, level);
    pkmn->exp             = js::value_to<size_t>(obj.at("exp"));
    pkmn->hp              = js::value_to<size_t>(obj.at("hp"));
    pkmn->nickName        = js::value_to<std::string>(obj.at("nickName"));
    pkmn->moves           = js::value_to<std::array<Move::MovePtr, 4>>(obj.at("moves"));
    pkmn->statusCondition = static_cast<Pkmn::StatusCondition>(js::value_to<int>(obj.at("statusCondition")));
    pkmn->happiness       = js::value_to<unsigned char>(obj.at("happiness"));
    for (auto& value : obj.at("IVs").as_array())
    {
        js::object const& obj                                                  = value.as_object();
        pkmn->IVs[static_cast<PkmnDef::Stat>(js::value_to<int>(obj.at("id")))] = js::value_to<size_t>(obj.at("value"));
    }
    for (auto& value : obj.at("EVs").as_array())
    {
        js::object const& obj                                                  = value.as_object();
        pkmn->EVs[static_cast<PkmnDef::Stat>(js::value_to<int>(obj.at("id")))] = js::value_to<size_t>(obj.at("value"));
    }
    return pkmn;
}
