#include "abilitybattle.h"
#include "moveeffects.h"
#include "trainerscenedata.h"

#include "game.h"
#include "itemutils.h"
#include "move.h"
#include "statuscondition.h"
#include "item.h"
#include "utils.h"

#include <algorithm>
#include <boost/format.hpp>
#include <boost/locale.hpp>

namespace lc = boost::locale;

namespace
{
void noteEject(SingleBattleSceneData& data, BattleBattler& battler)
{
    if (&battler == &data.playerBattler)
        data.playerEjectSwitch = true;
    else
        data.opponentEjectSwitch = true;
}

bool active(BattleBattler const& battler)
{
    return !battler.abilitySuppressed && !battler.abilityId.empty();
}

std::string nameOf(Pkmn::PkmnPtr const& pkmn)
{
    return pkmn ? pkmn->getDisplayName() : std::string();
}
} // namespace

void assignBattleAbility(Pkmn::PkmnPtr const& pkmn, BattleBattler& battler)
{
    battler.abilitySuppressed = false;
    battler.abilityId.clear();
    if (!pkmn)
        return;
    battler.abilityId = pkmn->effectiveAbilityId();
}

void refreshPlateType(Pkmn::PkmnPtr const& pkmn, BattleBattler& battler)
{
    if (battler.abilityId != "MULTITYPE" && battler.abilityId != "RKSSYSTEM")
        return;
    if (battler.abilitySuppressed)
    {
        battler.typesOverride.clear();
        return;
    }
    std::string typed = ItemUtils::plateTypeForItem(ItemUtils::activeHeldId(pkmn, &battler));
    if (typed.empty())
        battler.typesOverride.clear();
    else
        battler.typesOverride = {typed};
}

void triggerAbility(SingleBattleSceneData& data, AbilityMoment moment, AbilityEvent& event)
{
    switch (moment)
    {
    case AbilityMoment::SwitchIn:
        abilityOnSwitchIn(data, event.playerSide, event.messages);
        break;
    case AbilityMoment::BeforeMove:
        abilityBeforeMove(data, event.playerSide, event.messages, event.blocked);
        break;
    case AbilityMoment::OnHit:
        event.userFainted = abilityOnHit(data, event.playerSide, event.contact, event.moveType, event.moveId, event.messages);
        break;
    case AbilityMoment::OnStatus:
    {
        auto target = event.playerSide ? data.playerPkmn : data.opponentPkmn;
        BattleBattler const& battler = event.playerSide ? data.playerBattler : data.opponentBattler;
        event.blocked = !event.ignoreAbility && abilityBlocksStatus(target, battler, event.status);
        if (!event.ignoreAbility && !event.blocked && active(battler) && battler.abilityId == "LEAFGUARD"
            && data.fieldWeather() == Map::Weather::HARSH_SUNLIGHT
            && ItemUtils::activeHeldId(target, &battler) != "UTILITYUMBRELLA")
            event.blocked = true;
        if (event.blocked && target)
            event.messages.push_back((boost::format(lc::translate("%1%'s %2% protected it!")) % nameOf(target) % battler.abilityId).str());
        break;
    }
    case AbilityMoment::Residual:
        abilityOnResidual(data, event.playerSide, event.messages);
        break;
    }
}

void abilityBeforeMove(SingleBattleSceneData& data, bool playerIsUser, std::vector<std::string>& messages, bool& blocked)
{
    auto user = playerIsUser ? data.playerPkmn : data.opponentPkmn;
    BattleBattler& battler = playerIsUser ? data.playerBattler : data.opponentBattler;
    if (!user || !active(battler))
        return;
    if (battler.abilityId == "TRUANT")
    {
        if (battler.truantLoaf)
        {
            battler.truantLoaf = false;
            blocked = true;
            messages.push_back((boost::format(lc::translate("%1% is loafing around!")) % nameOf(user)).str());
        }
        else
            battler.truantLoaf = true;
    }
    if (battler.abilityId == "PROTEAN" || battler.abilityId == "LIBERO")
    {
        // The move type is applied by the caller through the battler's type list once the move is known.
    }
    if (battler.slowStartTurns > 0)
        messages.push_back((boost::format(lc::translate("%1%'s Slow Start keeps it from getting going!")) % nameOf(user)).str());
}

void abilityOnResidual(SingleBattleSceneData& data, bool playerSide, std::vector<std::string>& messages)
{
    auto pkmn = playerSide ? data.playerPkmn : data.opponentPkmn;
    BattleBattler& battler = playerSide ? data.playerBattler : data.opponentBattler;
    if (!pkmn || pkmn->isKO() || !active(battler))
        return;
    auto id = battler.abilityId;
    auto status = pkmn->getStatusCondition();
    bool cure = (id == "IMMUNITY" && (status == Pkmn::StatusCondition::POISON || status == Pkmn::StatusCondition::BADLY_POISON))
             || (id == "LIMBER" && status == Pkmn::StatusCondition::PARALYSIS)
             || ((id == "INSOMNIA" || id == "VITALSPIRIT") && status == Pkmn::StatusCondition::SLEEP)
             || ((id == "WATERVEIL" || id == "WATERBUBBLE") && status == Pkmn::StatusCondition::BURN)
             || (id == "MAGMAARMOR" && status == Pkmn::StatusCondition::FREEZE)
             || (id == "SHEDSKIN" && status != Pkmn::StatusCondition::NONE && Utils::randuint(1, 100) <= 30);
    if (cure)
    {
        pkmn->setStatusCondition(Pkmn::StatusCondition::NONE);
        messages.push_back((boost::format(lc::translate("%1%'s %2% cured its status!")) % nameOf(pkmn) % id).str());
    }
    if (id == "SPEEDBOOST" && !battler.switchedInThisTurn)
    {
        auto raised = applyStatChange(data, battler, pkmn, BattleBattler::StageStat::Speed, 1, messages, false);
        if (raised.ejected)
            noteEject(data, battler);
    }
    bool umbrella = ItemUtils::activeHeldId(pkmn, &battler) == "UTILITYUMBRELLA";
    auto weather  = data.fieldWeather();
    if (id == "RAINDISH" && weather == Map::Weather::RAIN && !umbrella && battler.healBlockTurns <= 0)
    {
        pkmn->setHP(std::min(pkmn->getTotalHP(), pkmn->getHP() + std::max<size_t>(1, pkmn->getTotalHP() / 16)));
        messages.push_back((boost::format(lc::translate("%1%'s Rain Dish restored its HP!")) % nameOf(pkmn)).str());
    }
    if (id == "DRYSKIN" && weather == Map::Weather::RAIN && !umbrella && battler.healBlockTurns <= 0)
    {
        pkmn->setHP(std::min(pkmn->getTotalHP(), pkmn->getHP() + std::max<size_t>(1, pkmn->getTotalHP() / 8)));
        messages.push_back((boost::format(lc::translate("%1%'s Dry Skin restored its HP!")) % nameOf(pkmn)).str());
    }
    if (id == "DRYSKIN" && weather == Map::Weather::HARSH_SUNLIGHT && !umbrella
        && battler.abilityId != "MAGICGUARD")
    {
        pkmn->decreaseHP(std::max<size_t>(1, pkmn->getTotalHP() / 8));
        messages.push_back((boost::format(lc::translate("%1%'s Dry Skin hurt it!")) % nameOf(pkmn)).str());
        if (pkmn->isKO())
            messages.push_back((boost::format(lc::translate("%1% fainted !")) % nameOf(pkmn)).str());
    }
    if (id == "ICEBODY" && weather == Map::Weather::HAIL && battler.healBlockTurns <= 0)
    {
        pkmn->setHP(std::min(pkmn->getTotalHP(), pkmn->getHP() + std::max<size_t>(1, pkmn->getTotalHP() / 16)));
        messages.push_back((boost::format(lc::translate("%1%'s Ice Body restored its HP!")) % nameOf(pkmn)).str());
    }
    if (id == "MOODY" && !battler.switchedInThisTurn)
    {
        BattleBattler::StageStat pool[] = {
            BattleBattler::StageStat::Attack, BattleBattler::StageStat::Defense, BattleBattler::StageStat::SpecialAttack,
            BattleBattler::StageStat::SpecialDefense, BattleBattler::StageStat::Speed};
        std::vector<BattleBattler::StageStat> raiseable;
        std::vector<BattleBattler::StageStat> lowerable;
        for (auto s : pool)
        {
            if (battler.stages[static_cast<size_t>(s)] < 6)
                raiseable.push_back(s);
            if (battler.stages[static_cast<size_t>(s)] > -6)
                lowerable.push_back(s);
        }
        if (!raiseable.empty())
        {
            auto up = raiseable[Utils::randuint(0, raiseable.size() - 1)];
            auto raised = applyStatChange(data, battler, pkmn, up, 2, messages, false);
            if (raised.ejected)
                noteEject(data, battler);
            lowerable.erase(std::remove(lowerable.begin(), lowerable.end(), up), lowerable.end());
            if (!lowerable.empty())
            {
                auto down = lowerable[Utils::randuint(0, lowerable.size() - 1)];
                auto lowered = applyStatChange(data, battler, pkmn, down, -1, messages, false);
                if (lowered.ejected)
                    noteEject(data, battler);
            }
        }
    }
    if (id == "ICEFACE" && battler.iceFaceBroken && weather == Map::Weather::HAIL)
    {
        battler.iceFaceBroken = false;
        messages.push_back((boost::format(lc::translate("%1%'s Ice Face was restored!")) % nameOf(pkmn)).str());
    }
    if (battler.slowStartTurns > 0)
        --battler.slowStartTurns;
    if (id == "HYDRATION" && weather == Map::Weather::RAIN && !umbrella && status != Pkmn::StatusCondition::NONE)
    {
        pkmn->setStatusCondition(Pkmn::StatusCondition::NONE);
        messages.push_back((boost::format(lc::translate("%1%'s Hydration cured its status!")) % nameOf(pkmn)).str());
    }
    if (id == "BADDREAMS")
    {
        auto foe = playerSide ? data.opponentPkmn : data.playerPkmn;
        if (foe && !foe->isKO() && foe->getStatusCondition() == Pkmn::StatusCondition::SLEEP)
        {
            BattleBattler const& foeBattler = playerSide ? data.opponentBattler : data.playerBattler;
            if (foeBattler.abilitySuppressed || foeBattler.abilityId != "MAGICGUARD")
            {
                foe->decreaseHP(std::max<size_t>(1, foe->getTotalHP() / 8));
                messages.push_back((boost::format(lc::translate("%1% is tormented by %2%'s Bad Dreams!")) % nameOf(foe) % nameOf(pkmn)).str());
            }
        }
    }
    if (id == "HARVEST" && !pkmn->getHeldItem() && !battler.consumedItemId.empty()
        && battler.consumedItemId.find("BERRY") != std::string::npos
        && ((weather == Map::Weather::HARSH_SUNLIGHT && !umbrella) || Utils::randuint(0, 1) == 0))
    {
        auto def = Game::instance()->data.itemDefFor(battler.consumedItemId);
        if (def)
        {
            pkmn->setHeldItem(std::make_shared<Item>(def, 1));
            battler.consumedItemId.clear();
            battler.unburdenActive = false;
            messages.push_back((boost::format(lc::translate("%1%'s Harvest restored its berry!")) % nameOf(pkmn)).str());
        }
    }
    if (id == "SOLARPOWER" && weather == Map::Weather::HARSH_SUNLIGHT && !umbrella
        && battler.abilityId != "MAGICGUARD")
    {
        pkmn->decreaseHP(std::max<size_t>(1, pkmn->getTotalHP() / 8));
        messages.push_back((boost::format(lc::translate("%1% is hurt by its Solar Power!")) % nameOf(pkmn)).str());
    }
}

void applyMimicry(SingleBattleSceneData& data, std::vector<std::string>& messages)
{
    (void)data;
    (void)messages;
}

namespace {

void clearFormeStats(BattleBattler& battler)
{
    battler.attackOverride    = -1;
    battler.defenseOverride   = -1;
    battler.spAttackOverride  = -1;
    battler.spDefenseOverride = -1;
    battler.speedOverride     = -1;
}

void applyFormeBases(Pkmn::PkmnPtr const& pkmn, BattleBattler& battler, size_t attack, size_t defense, size_t specialAttack,
                     size_t specialDefense, size_t speed)
{
    battler.attackOverride    = static_cast<int>(pkmn->statFromBase(PkmnDef::ATTACK, attack));
    battler.defenseOverride   = static_cast<int>(pkmn->statFromBase(PkmnDef::DEFENSE, defense));
    battler.spAttackOverride  = static_cast<int>(pkmn->statFromBase(PkmnDef::SPECIAL_ATTACK, specialAttack));
    battler.spDefenseOverride = static_cast<int>(pkmn->statFromBase(PkmnDef::SPECIAL_DEFENSE, specialDefense));
    battler.speedOverride     = static_cast<int>(pkmn->statFromBase(PkmnDef::SPEED, speed));
}

} // namespace

void refreshForme(SingleBattleSceneData& data, bool playerSide, bool allowZen)
{
    auto           pkmn    = playerSide ? data.playerPkmn : data.opponentPkmn;
    BattleBattler& battler = playerSide ? data.playerBattler : data.opponentBattler;
    if (!pkmn)
        return;
    bool        off = battler.abilitySuppressed || pkmn->isKO();
    std::string id  = battler.abilityId;
    auto setSuffix = [&](std::string const& suffix) { battler.spriteFormeSuffix = suffix; };
    if (id == "FORECAST")
    {
        bool umbrella = ItemUtils::activeHeldId(pkmn, &battler) == "UTILITYUMBRELLA";
        auto weather  = data.fieldWeather();
        if (off || weather == Map::NONE || weather == Map::SANDSTORM)
        {
            battler.typesOverride.clear();
            setSuffix({});
        }
        else if (weather == Map::HARSH_SUNLIGHT && !umbrella)
        {
            battler.typesOverride = {"FIRE"};
            setSuffix("_1");
        }
        else if (weather == Map::RAIN && !umbrella)
        {
            battler.typesOverride = {"WATER"};
            setSuffix("_2");
        }
        else if (weather == Map::HAIL)
        {
            battler.typesOverride = {"ICE"};
            setSuffix("_3");
        }
        else
        {
            battler.typesOverride.clear();
            setSuffix({});
        }
        return;
    }
    if (id == "HUNGERSWITCH")
    {
        if (off)
        {
            battler.typesOverride.clear();
            setSuffix({});
        }
        else if (battler.hangry)
        {
            battler.typesOverride = {"ELECTRIC", "DARK"};
            setSuffix("_1");
        }
        else
        {
            battler.typesOverride.clear();
            setSuffix({});
        }
        return;
    }
    if (id == "FLOWERGIFT")
    {
        if (!off && data.fieldWeather() == Map::HARSH_SUNLIGHT
            && ItemUtils::activeHeldId(pkmn, &battler) != "UTILITYUMBRELLA")
        {
            auto stats                = pkmn->getStats();
            battler.attackOverride    = static_cast<int>(stats[PkmnDef::ATTACK] * 3 / 2);
            battler.spDefenseOverride = static_cast<int>(stats[PkmnDef::SPECIAL_DEFENSE] * 3 / 2);
            setSuffix("_1");
        }
        else
        {
            battler.attackOverride    = -1;
            battler.spDefenseOverride = -1;
            setSuffix({});
        }
        return;
    }
    if (id == "ICEFACE")
    {
        if (off || !battler.iceFaceBroken)
        {
            clearFormeStats(battler);
            setSuffix({});
        }
        else
        {
            applyFormeBases(pkmn, battler, 80, 70, 65, 50, 130);
            setSuffix("_1");
        }
        return;
    }
    if (battler.gulpGorging)
        setSuffix("_2");
    else if (battler.gulpGulping)
        setSuffix("_1");
    else if (battler.battleBondActive)
        setSuffix("_1");
    else if (battler.bladeForme)
        setSuffix("_1");
    else if (battler.pirouette)
        setSuffix("_1");
    else if (id == "GULPMISSILE")
        setSuffix({});
    if (id != "SCHOOLING" && id != "ZENMODE" && id != "SHIELDSDOWN")
        return;
    if (id == "ZENMODE" && !allowZen)
        return;
    bool formed = false;
    if (!off && pkmn->getTotalHP() > 0)
    {
        if (id == "SCHOOLING")
            formed = pkmn->getLevel() >= 20 && pkmn->getHP() * 4 > pkmn->getTotalHP();
        else
            formed = pkmn->getHP() * 2 <= pkmn->getTotalHP();
    }
    if (!formed)
    {
        clearFormeStats(battler);
        if (id == "SCHOOLING" || id == "ZENMODE" || id == "SHIELDSDOWN")
            setSuffix({});
        return;
    }
    if (id == "SCHOOLING")
    {
        applyFormeBases(pkmn, battler, 140, 130, 140, 135, 30);
        setSuffix("_1");
    }
    else if (id == "ZENMODE")
    {
        applyFormeBases(pkmn, battler, 30, 105, 140, 105, 55);
        setSuffix("_1");
    }
    else
    {
        applyFormeBases(pkmn, battler, 100, 60, 100, 60, 120);
        setSuffix("_1");
    }
}

void tickFormes(SingleBattleSceneData& data)
{
    auto toggle = [](Pkmn::PkmnPtr const& pkmn, BattleBattler& battler) {
        if (pkmn && !pkmn->isKO() && !battler.abilitySuppressed && battler.abilityId == "HUNGERSWITCH")
            battler.hangry = !battler.hangry;
    };
    toggle(data.playerPkmn, data.playerBattler);
    toggle(data.opponentPkmn, data.opponentBattler);
    refreshForme(data, true, true);
    refreshForme(data, false, true);
}

void abilityOnSwitchIn(SingleBattleSceneData& data, bool playerSide, std::vector<std::string>& messages)
{
    auto pkmn = playerSide ? data.playerPkmn : data.opponentPkmn;
    auto other = playerSide ? data.opponentPkmn : data.playerPkmn;
    BattleBattler& battler = playerSide ? data.playerBattler : data.opponentBattler;
    BattleBattler& otherBattler = playerSide ? data.opponentBattler : data.playerBattler;
    if (!pkmn)
        return;
    struct WhiteHerbOnExit
    {
        SingleBattleSceneData&    data;
        std::vector<std::string>& messages;
        ~WhiteHerbOnExit()
        {
            auto restore = [&](Pkmn::PkmnPtr const& mon, BattleBattler& side) {
                auto tick = ItemUtils::consumeWhiteHerb(mon, &side);
                if (!tick.message.empty())
                    messages.push_back(tick.message);
            };
            restore(data.playerPkmn, data.playerBattler);
            restore(data.opponentPkmn, data.opponentBattler);
        }
    } whiteHerbOnExit{data, messages};
    if (!otherBattler.abilitySuppressed && otherBattler.abilityId == "NEUTRALIZINGGAS" && battler.abilityId != "NEUTRALIZINGGAS")
    {
        battler.abilitySuppressed = true;
        refreshForme(data, playerSide);
        return;
    }
    refreshPlateType(pkmn, battler);
    refreshForme(data, playerSide);
    if (!active(battler))
        return;

    if (battler.abilityId == "INTIMIDATE" && other && !other->isKO())
    {
        auto dropped = applyStatChange(data, otherBattler, other, BattleBattler::StageStat::Attack, -1, messages, true);
        if (dropped.ejected)
            noteEject(data, otherBattler);
    }
    else if (battler.abilityId == "DRIZZLE")
    {
        if (!primordialWeatherBlocksNewWeather(data))
        {
            data.weather         = Map::Weather::RAIN;
            data.weatherFromMove = false;
            data.weatherTurns    = 0;
            messages.push_back((boost::format(lc::translate("%1%'s Drizzle made it rain!")) % nameOf(pkmn)).str());
        }
    }
    else if (battler.abilityId == "DROUGHT")
    {
        if (!primordialWeatherBlocksNewWeather(data))
        {
            data.weather         = Map::Weather::HARSH_SUNLIGHT;
            data.weatherFromMove = false;
            data.weatherTurns    = 0;
            messages.push_back((boost::format(lc::translate("%1%'s Drought intensified the sun!")) % nameOf(pkmn)).str());
        }
    }
    else if (battler.abilityId == "SANDSTREAM")
    {
        if (!primordialWeatherBlocksNewWeather(data))
        {
            data.weather         = Map::Weather::SANDSTORM;
            data.weatherFromMove = false;
            data.weatherTurns    = 0;
            messages.push_back(
                (boost::format(lc::translate("%1%'s Sand Stream whipped up a sandstorm!")) % nameOf(pkmn)).str());
        }
    }
    else if (battler.abilityId == "SNOWWARNING")
    {
        if (!primordialWeatherBlocksNewWeather(data))
        {
            data.weather         = Map::Weather::HAIL;
            data.weatherFromMove = true;
            data.weatherTurns    = ItemUtils::weatherTurnsForHolder(pkmn, Map::Weather::HAIL, &battler);
            messages.push_back(
                (boost::format(lc::translate("%1%'s Snow Warning started a hailstorm!")) % nameOf(pkmn)).str());
        }
    }
    else if (battler.abilityId == "PRIMORDIALSEA")
    {
        data.weather         = Map::Weather::RAIN;
        data.weatherFromMove = true;
        data.weatherTurns    = -1;
        data.strongWinds     = false;
        messages.push_back((boost::format(lc::translate("%1%'s Primordial Sea made a heavy rain fall!")) % nameOf(pkmn)).str());
    }
    else if (battler.abilityId == "DESOLATELAND")
    {
        data.weather         = Map::Weather::HARSH_SUNLIGHT;
        data.weatherFromMove = true;
        data.weatherTurns    = -1;
        data.strongWinds     = false;
        messages.push_back(
            (boost::format(lc::translate("%1%'s Desolate Land turned the sunlight extremely harsh!")) % nameOf(pkmn)).str());
    }
    else if (battler.abilityId == "DELTASTREAM")
    {
        data.strongWinds     = true;
        data.weatherFromMove = true;
        data.weatherTurns    = -1;
        messages.push_back(
            (boost::format(lc::translate("%1%'s Delta Stream whipped up mysterious strong winds!")) % nameOf(pkmn)).str());
    }
    else if (battler.abilityId == "DOWNLOAD" && other)
    {
        size_t defense = otherBattler.defenseOverride >= 0 ? static_cast<size_t>(otherBattler.defenseOverride)
                                                           : other->getStats()[PkmnDef::DEFENSE];
        size_t special = otherBattler.spDefenseOverride >= 0 ? static_cast<size_t>(otherBattler.spDefenseOverride)
                                                             : other->getStats()[PkmnDef::SPECIAL_DEFENSE];
        defense = otherBattler.effectiveStat(defense, BattleBattler::StageStat::Defense);
        special = otherBattler.effectiveStat(special, BattleBattler::StageStat::SpecialDefense);
        auto stat = defense < special ? BattleBattler::StageStat::Attack : BattleBattler::StageStat::SpecialAttack;
        auto raised = applyStatChange(data, battler, pkmn, stat, 1, messages, false);
        if (raised.ejected)
            noteEject(data, battler);
    }
    else if (battler.abilityId == "TRACE" && other && !otherBattler.abilityId.empty())
    {
        static char const* untraceable[] = {
            "TRACE", "ILLUSION", "MULTITYPE", "STANCECHANGE", "DISGUISE", "IMPOSTER", "RKSSYSTEM",
            "SCHOOLING", "ZENMODE", "BATTLEBOND", "POWERCONSTRUCT", "SHIELDSDOWN", "COMATOSE",
            "GULPMISSILE", "ICEFACE", "HUNGERSWITCH", "FORECAST", "FLOWERGIFT", "NEUTRALIZINGGAS"};
        bool banned = false;
        for (auto id : untraceable)
        {
            if (otherBattler.abilityId == id)
            {
                banned = true;
                break;
            }
        }
        if (!banned)
        {
            battler.abilityId = otherBattler.abilityId;
            messages.push_back(
                (boost::format(lc::translate("%1%'s Trace copied %2%!")) % nameOf(pkmn) % battler.abilityId).str());
            if (battler.abilityId != "TRACE")
                abilityOnSwitchIn(data, playerSide, messages);
            return;
        }
    }
    else if (battler.abilityId == "FOREWARN" && other)
    {
        std::string warned = "a move";
        size_t best = 0;
        for (auto const& slot : other->getMoves())
        {
            if (slot && slot->getDefinition() && slot->getDefinition()->getPower() >= best)
            {
                best = slot->getDefinition()->getPower();
                warned = slot->getDefinition()->getName();
            }
        }
        messages.push_back((boost::format(lc::translate("%1%'s Forewarn alerted it to %2%!")) % nameOf(pkmn) % warned).str());
    }
    else if (battler.abilityId == "ANTICIPATION" && other)
    {
        bool shiver = false;
        for (auto const& slot : other->getMoves())
        {
            if (!slot || !slot->getDefinition())
                continue;
            auto def = slot->getDefinition();
            auto code = def->getFunctionCode();
            if (code == MoveDef::FunctionCode::OHKO || code == MoveDef::FunctionCode::OHKOHitsUndergroundTarget
                || code == MoveDef::FunctionCode::OHKOIce)
                shiver = true;
            else if (def->getPower() > 0)
            {
                for (auto const& typeId : activeTypes(pkmn, &battler))
                {
                    auto type = Game::instance()->data.typeFor(typeId);
                    if (!type)
                        continue;
                    auto const& weak = type->getWeaknesses();
                    if (std::find(weak.begin(), weak.end(), def->getType()) != weak.end())
                        shiver = true;
                }
            }
        }
        if (shiver)
            messages.push_back((boost::format(lc::translate("%1%'s Anticipation made it shiver!")) % nameOf(pkmn)).str());
    }
    else if (battler.abilityId == "FRISK" && other && other->getHeldItem() && other->getHeldItem()->getDefinition())
        messages.push_back((boost::format(lc::translate("%1%'s Frisk found %2%!")) % nameOf(pkmn)
                            % other->getHeldItem()->getDefinition()->getName()).str());
    else if (battler.abilityId == "IMPOSTER" && other && !other->isKO())
    {
        if (otherBattler.substituteHP > 0 || otherBattler.illusionActive || otherBattler.transformActive
            || otherBattler.semiInvulnerable)
            ; // fail vs Substitute / Illusion / Transform / semi-invulnerable
        else
        {
            copyTransformFrom(pkmn, battler, other, otherBattler);
            messages.push_back((boost::format(lc::translate("%1%'s Imposter transformed it!")) % nameOf(pkmn)).str());
            return;
        }
    }
    if (!battler.abilitySuppressed && battler.abilityId == "SLOWSTART")
        battler.slowStartTurns = 5;
    if (!battler.abilitySuppressed && battler.abilityId == "INTREPIDSWORD")
    {
        auto raised = applyStatChange(data, battler, pkmn, BattleBattler::StageStat::Attack, 1, messages, false);
        if (raised.ejected)
            noteEject(data, battler);
    }
    if (!battler.abilitySuppressed && battler.abilityId == "DAUNTLESSSHIELD")
    {
        auto raised = applyStatChange(data, battler, pkmn, BattleBattler::StageStat::Defense, 1, messages, false);
        if (raised.ejected)
            noteEject(data, battler);
    }
    if (!battler.abilitySuppressed && battler.abilityId == "SCREENCLEANER")
    {
        data.playerReflect = data.playerLightScreen = data.playerAuroraVeil = 0;
        data.opponentReflect = data.opponentLightScreen = data.opponentAuroraVeil = 0;
        messages.push_back((boost::format(lc::translate("%1%'s Screen Cleaner cleared the screens!")) % nameOf(pkmn)).str());
    }
    if (!battler.abilitySuppressed && battler.abilityId == "NEUTRALIZINGGAS")
    {
        otherBattler.abilitySuppressed = true;
        refreshPlateType(other, otherBattler);
        refreshForme(data, !playerSide);
        messages.push_back((boost::format(lc::translate("%1%'s Neutralizing Gas filled the area!")) % nameOf(pkmn)).str());
    }

    if (battler.abilityId == "ILLUSION")
    {
        Pkmn::PkmnPtr disguise;
        auto          consider = [&](auto const& party) {
            for (auto const& member : party)
            {
                if (member && member != pkmn && !member->isKO())
                    disguise = member;
            }
        };
        if (playerSide)
            consider(Game::instance()->data.player.pkmns);
        else if (auto* trainer = dynamic_cast<TrainerSceneData*>(&data))
        {
            if (trainer->opponentTrainer)
                consider(trainer->opponentTrainer->pkmns);
        }
        if (disguise)
        {
            battler.illusionActive   = true;
            battler.illusionDisguise = disguise;
            messages.push_back((boost::format(lc::translate("%1% disguised itself!")) % nameOf(pkmn)).str());
        }
    }

    auto tryTrap = [](BattleBattler const& trapper, Pkmn::PkmnPtr const& prey, BattleBattler& preyBattler,
                      SingleBattleSceneData const& field) {
        if (!prey || prey->isKO() || !active(trapper))
            return;
        bool groundedPrey = grounded(prey, preyBattler, field);
        if (trapper.abilityId == "SHADOWTAG")
            preyBattler.trapped = true;
        else if (trapper.abilityId == "ARENATRAP" && groundedPrey)
            preyBattler.trapped = true;
        else if (trapper.abilityId == "MAGNETPULL" && hasType(prey, "STEEL", &preyBattler))
            preyBattler.trapped = true;
    };
    // Entrant traps the resident, and a resident trapper re-traps the entrant.
    if (other && !other->isKO())
    {
        tryTrap(battler, other, otherBattler, data);
        tryTrap(otherBattler, pkmn, battler, data);
    }
}

bool abilityOnHit(SingleBattleSceneData& data,
                  bool                    attackerIsPlayer,
                  bool                    contact,
                  std::string const&      moveType,
                  std::string const&      moveId,
                  std::vector<std::string>& messages)
{
    (void)moveType;
    auto attacker = attackerIsPlayer ? data.playerPkmn : data.opponentPkmn;
    auto defender = attackerIsPlayer ? data.opponentPkmn : data.playerPkmn;
    BattleBattler& attackerBattler = attackerIsPlayer ? data.playerBattler : data.opponentBattler;
    BattleBattler& defenderBattler = attackerIsPlayer ? data.opponentBattler : data.playerBattler;
    if (!attacker || !defender || !active(defenderBattler) || attacker->isKO())
        return false;
    if (contact && !attackerBattler.abilitySuppressed && attackerBattler.abilityId == "POISONTOUCH"
        && Utils::randuint(1, 100) <= 30
        && !abilityBlocksStatus(defender, defenderBattler, Pkmn::StatusCondition::POISON))
        applyStatusCondition(defender, Pkmn::StatusCondition::POISON, defenderBattler.toxicCounter, false, &defenderBattler);
    bool physicalHit = false;
    if (!moveId.empty())
    {
        if (auto moveDef = Game::instance()->data.moveDefFor(moveId))
            physicalHit = moveDef->getCategory() == MoveDef::PHYSICAL;
    }
    auto sticky = [](BattleBattler const& holder) {
        return !holder.abilitySuppressed && holder.abilityId == "STICKYHOLD";
    };
    auto steal = [&](Pkmn::PkmnPtr const& taker, BattleBattler& takerBattler, BattleBattler& holderBattler,
                     Pkmn::PkmnPtr const& holder, char const* text) {
        if (!taker || !holder || taker->getHeldItem() || !holder->getHeldItem() || sticky(holderBattler))
            return;
        std::string lost = ItemUtils::activeHeldId(holder, &holderBattler);
        taker->setHeldItem(holder->getHeldItem());
        ItemUtils::noteHeldItemLoss(holder, &holderBattler, lost);
        holder->setHeldItem(nullptr);
        // Receiving an item clears Unburden for the thief.
        takerBattler.unburdenActive = false;
        messages.push_back((boost::format(lc::translate(text)) % nameOf(taker) % nameOf(holder)).str());
    };
    if (!contact && defenderBattler.abilityId != "JUSTIFIED" && defenderBattler.abilityId != "STAMINA"
        && defenderBattler.abilityId != "ANGERPOINT" && defenderBattler.abilityId != "CURSEDBODY"
        && defenderBattler.abilityId != "COTTONDOWN" && !(defenderBattler.abilityId == "WEAKARMOR" && physicalHit))
    {
        if (!attackerBattler.abilitySuppressed && attackerBattler.abilityId == "MAGICIAN")
            steal(attacker, attackerBattler, defenderBattler, defender, "%1% stole %2%'s item with Magician!");
        return false;
    }

    auto sting = [&](Pkmn::StatusCondition status, char const* text) {
        if (abilityBlocksStatus(attacker, attackerBattler, status))
            return;
        if (applyStatusCondition(attacker, status, attackerBattler.toxicCounter, false, &attackerBattler))
            messages.push_back((boost::format(lc::translate(text)) % nameOf(attacker) % nameOf(defender)).str());
    };

    if (defenderBattler.abilityId == "STATIC" && Utils::randuint(1, 100) <= 30)
        sting(Pkmn::StatusCondition::PARALYSIS, "%1% was paralyzed by %2%'s Static!");
    else if (defenderBattler.abilityId == "FLAMEBODY" && Utils::randuint(1, 100) <= 30)
        sting(Pkmn::StatusCondition::BURN, "%1% was burned by %2%'s Flame Body!");
    else if (defenderBattler.abilityId == "POISONPOINT" && Utils::randuint(1, 100) <= 30)
        sting(Pkmn::StatusCondition::POISON, "%1% was poisoned by %2%'s Poison Point!");
    else if (defenderBattler.abilityId == "ROUGHSKIN"
             && (attackerBattler.abilitySuppressed || attackerBattler.abilityId != "MAGICGUARD"))
    {
        attacker->decreaseHP(std::max<size_t>(1, attacker->getTotalHP() / 16));
        messages.push_back((boost::format(lc::translate("%1% was hurt by %2%'s %3%!")) % nameOf(attacker) % nameOf(defender)
                            % defenderBattler.abilityId)
                               .str());
    }
    else if (defenderBattler.abilityId == "CUTECHARM" && Utils::randuint(1, 100) <= 30)
    {
        auto attackerGender = attacker->getGender();
        auto defenderGender = defender->getGender();
        bool blocked = !attackerBattler.abilitySuppressed
                    && (attackerBattler.abilityId == "OBLIVIOUS" || attackerBattler.abilityId == "AROMAVEIL");
        if (!blocked && attackerGender != defenderGender && attackerGender != Pkmn::Gender::GENDERLESS
            && defenderGender != Pkmn::Gender::GENDERLESS)
        {
            attackerBattler.infatuated = true;
            messages.push_back(
                (boost::format(lc::translate("%1% fell for %2%'s Cute Charm!")) % nameOf(attacker) % nameOf(defender)).str());
            if (ItemUtils::activeHeldId(attacker, &attackerBattler) == "DESTINYKNOT")
            {
                bool foeBlocked = !defenderBattler.abilitySuppressed
                               && (defenderBattler.abilityId == "OBLIVIOUS" || defenderBattler.abilityId == "AROMAVEIL");
                if (!foeBlocked)
                    defenderBattler.infatuated = true;
            }
            ItemUtils::consumeMentalHerb(attacker, &attackerBattler, messages);
            ItemUtils::consumeMentalHerb(defender, &defenderBattler, messages);
        }
    }
    else if (defenderBattler.abilityId == "EFFECTSPORE")
    {
        auto roll = Utils::randuint(1, 100);
        if (roll <= 10)
            sting(Pkmn::StatusCondition::POISON, "%1% was poisoned by %2%'s Effect Spore!");
        else if (roll <= 20)
            sting(Pkmn::StatusCondition::PARALYSIS, "%1% was paralyzed by %2%'s Effect Spore!");
        else if (roll <= 30 && data.playerBattler.uproarTurns <= 0 && data.opponentBattler.uproarTurns <= 0)
            sting(Pkmn::StatusCondition::SLEEP, "%1% was put to sleep by %2%'s Effect Spore!");
    }
    else if (defenderBattler.abilityId == "MUMMY")
    {
        static char const* formeLocked[] = {
            "MULTITYPE", "STANCECHANGE", "DISGUISE", "RKSSYSTEM", "SCHOOLING", "ZENMODE", "BATTLEBOND",
            "POWERCONSTRUCT", "SHIELDSDOWN", "COMATOSE", "GULPMISSILE", "ICEFACE", "HUNGERSWITCH",
            "NEUTRALIZINGGAS", "ASONECHILLINGNEIGH", "ASONEGRIMNEIGH"};
        bool locked = false;
        for (auto id : formeLocked)
        {
            if (attackerBattler.abilityId == id)
            {
                locked = true;
                break;
            }
        }
        if (!locked)
        {
            attackerBattler.abilityId = "MUMMY";
            messages.push_back((boost::format(lc::translate("%1%'s ability became Mummy!")) % nameOf(attacker)).str());
        }
    }
    else if (defenderBattler.abilityId == "GOOEY")
    {
        auto outcome = applyStatChange(data, attackerBattler, attacker, BattleBattler::StageStat::Speed, -1, messages, true);
        if (outcome.ejected)
            noteEject(data, attackerBattler);
        if (outcome.changed && outcome.applied < 0)
            messages.push_back((boost::format(lc::translate("%1%'s Speed fell from %2%'s Gooey!")) % nameOf(attacker) % nameOf(defender)).str());
    }
    else if (defenderBattler.abilityId == "CURSEDBODY" && !moveId.empty() && Utils::randuint(1, 100) <= 30)
    {
        bool aroma = !attackerBattler.abilitySuppressed && attackerBattler.abilityId == "AROMAVEIL";
        if (!aroma && attackerBattler.disableTurns <= 0)
        {
            attackerBattler.disableTurns   = 4;
            attackerBattler.disabledMoveId = moveId;
            messages.push_back(
                (boost::format(lc::translate("%1%'s Cursed Body disabled the move!")) % nameOf(defender)).str());
        }
    }
    else if (defenderBattler.abilityId == "WEAKARMOR" && physicalHit)
    {
        auto dropped = applyStatChange(data, defenderBattler, defender, BattleBattler::StageStat::Defense, -1, messages, false);
        auto raised = applyStatChange(data, defenderBattler, defender, BattleBattler::StageStat::Speed, 2, messages, false);
        if (dropped.ejected || raised.ejected)
            noteEject(data, defenderBattler);
        if (raised.changed && raised.applied > 0)
            messages.push_back((boost::format(lc::translate("%1%'s Weak Armor raised its Speed!")) % nameOf(defender)).str());
    }
    else if (defenderBattler.abilityId == "STAMINA")
    {
        auto outcome = applyStatChange(data, defenderBattler, defender, BattleBattler::StageStat::Defense, 1, messages, false);
        if (outcome.ejected)
            noteEject(data, defenderBattler);
        if (outcome.changed && outcome.applied > 0)
            messages.push_back((boost::format(lc::translate("%1%'s Stamina raised its Defense!")) % nameOf(defender)).str());
    }
    else if (defenderBattler.abilityId == "JUSTIFIED" && moveType == "DARK")
    {
        auto outcome = applyStatChange(data, defenderBattler, defender, BattleBattler::StageStat::Attack, 1, messages, false);
        if (outcome.ejected)
            noteEject(data, defenderBattler);
        if (outcome.changed && outcome.applied > 0)
            messages.push_back((boost::format(lc::translate("%1%'s Justified raised its Attack!")) % nameOf(defender)).str());
    }
    else if (defenderBattler.abilityId == "COTTONDOWN")
    {
        auto outcome = applyStatChange(data, attackerBattler, attacker, BattleBattler::StageStat::Speed, -1, messages, true);
        if (outcome.ejected)
            noteEject(data, attackerBattler);
        if (outcome.changed && outcome.applied < 0)
            messages.push_back((boost::format(lc::translate("%1%'s Speed fell from %2%'s Cotton Down!")) % nameOf(attacker) % nameOf(defender)).str());
    }
    else if (contact && defenderBattler.abilityId == "TANGLINGHAIR")
    {
        auto outcome = applyStatChange(data, attackerBattler, attacker, BattleBattler::StageStat::Speed, -1, messages, true);
        if (outcome.ejected)
            noteEject(data, attackerBattler);
        if (outcome.changed && outcome.applied < 0)
            messages.push_back((boost::format(lc::translate("%1%'s Speed fell from %2%'s Tangling Hair!")) % nameOf(attacker) % nameOf(defender)).str());
    }
    else if (contact && defenderBattler.abilityId == "PERISHBODY")
    {
        if (attackerBattler.perishCount < 0)
            attackerBattler.perishCount = 3;
        if (defenderBattler.perishCount < 0)
            defenderBattler.perishCount = 3;
        messages.push_back((boost::format(lc::translate("%1%'s Perish Body started the count!")) % nameOf(defender)).str());
    }
    else if (contact && defenderBattler.abilityId == "WANDERINGSPIRIT" && !attackerBattler.abilitySuppressed)
    {
        static char const* formeLocked[] = {
            "MULTITYPE", "STANCECHANGE", "DISGUISE", "RKSSYSTEM", "SCHOOLING", "ZENMODE", "BATTLEBOND",
            "POWERCONSTRUCT", "SHIELDSDOWN", "COMATOSE", "GULPMISSILE", "ICEFACE", "HUNGERSWITCH",
            "NEUTRALIZINGGAS", "ASONECHILLINGNEIGH", "ASONEGRIMNEIGH"};
        bool locked = false;
        for (auto id : formeLocked)
        {
            if (attackerBattler.abilityId == id)
            {
                locked = true;
                break;
            }
        }
        if (!locked)
        {
            std::swap(attackerBattler.abilityId, defenderBattler.abilityId);
            messages.push_back(
                (boost::format(lc::translate("%1% swapped abilities with Wandering Spirit!")) % nameOf(defender)).str());
        }
    }

    if (!attackerBattler.abilitySuppressed && attackerBattler.abilityId == "MAGICIAN")
        steal(attacker, attackerBattler, defenderBattler, defender, "%1% stole %2%'s item with Magician!");
    if (contact && defenderBattler.abilityId == "PICKPOCKET")
        steal(defender, defenderBattler, attackerBattler, attacker, "%1% stole %2%'s item with Pickpocket!");
    return attacker->isKO();
}

bool abilityBlocksStatus(Pkmn::PkmnPtr const& target, BattleBattler const& battler, Pkmn::StatusCondition status,
                         bool fromOther)
{
    (void)target;
    if (!active(battler))
        return false;
    auto id = battler.abilityId;
    if ((id == "IMMUNITY" || id == "PASTELVEIL") && (status == Pkmn::StatusCondition::POISON || status == Pkmn::StatusCondition::BADLY_POISON))
        return true;
    if (id == "LIMBER" && status == Pkmn::StatusCondition::PARALYSIS)
        return true;
    if ((id == "INSOMNIA" || id == "VITALSPIRIT" || id == "SWEETVEIL") && status == Pkmn::StatusCondition::SLEEP)
        return true;
    if ((id == "WATERVEIL" || id == "WATERBUBBLE") && status == Pkmn::StatusCondition::BURN)
        return true;
    if (id == "MAGMAARMOR" && status == Pkmn::StatusCondition::FREEZE)
        return true;
    if (fromOther && id == "FLOWERVEIL" && target && hasType(target, "GRASS", &battler))
        return true;
    if (id == "COMATOSE")
        return true;
    if (id == "SHIELDSDOWN" && target && target->getTotalHP() > 0 && target->getHP() * 2 > target->getTotalHP())
        return true;
    if (id == "OBLIVIOUS")
        return false;
    return false;
}

float abilityPowerMultiplier(Pkmn::PkmnPtr const& user, BattleBattler const& battler, MoveDef::MoveDefPtr const& move,
                             Map::Weather weather)
{
    (void)weather;
    if (!user || !move || !active(battler))
        return 1.f;
    if (battler.abilityId == "GUTS" && move->getCategory() == MoveDef::PHYSICAL
        && user->getStatusCondition() != Pkmn::StatusCondition::NONE)
        return 1.5f;
    if (battler.slowStartTurns > 0 && move->getCategory() == MoveDef::PHYSICAL)
        return 0.5f;
    return 1.f;
}

void clearTrapsOnLeave(SingleBattleSceneData& data, bool playerSide)
{
    BattleBattler const& battler = playerSide ? data.playerBattler : data.opponentBattler;
    if (battler.abilityId == "SHADOWTAG" || battler.abilityId == "ARENATRAP" || battler.abilityId == "MAGNETPULL")
    {
        BattleBattler& otherBattler = playerSide ? data.opponentBattler : data.playerBattler;
        // Do not wipe Mean Look / Octolock / Jaw Lock / bind / Ingrain.
        if (!otherBattler.meanLooked && !otherBattler.octolock && !otherBattler.jawLocked
            && otherBattler.bindingTurns <= 0 && !otherBattler.ingrain)
            otherBattler.trapped = false;
    }
    // Move-based traps end when the trapper leaves (switch or faint).
    BattleBattler& otherBattler = playerSide ? data.opponentBattler : data.playerBattler;
    otherBattler.bindingTurns = 0;
    otherBattler.octolock     = false;
    otherBattler.meanLooked   = false;
    if (battler.jawLocked || otherBattler.jawLocked)
    {
        BattleBattler& self = playerSide ? data.playerBattler : data.opponentBattler;
        self.jawLocked      = false;
        otherBattler.jawLocked = false;
        if (!self.meanLooked && !self.octolock && self.bindingTurns <= 0 && !self.ingrain)
            self.trapped = false;
    }
    if (!otherBattler.meanLooked && !otherBattler.octolock && !otherBattler.jawLocked
        && otherBattler.bindingTurns <= 0 && !otherBattler.ingrain)
        otherBattler.trapped = false;
    else
        otherBattler.trapped = true;
    otherBattler.infatuated = false;

    // Neutralizing Gas leaving (switch or faint) re-activates the foe's ability.
    BattleBattler& leaving = playerSide ? data.playerBattler : data.opponentBattler;
    if (leaving.abilityId == "NEUTRALIZINGGAS")
    {
        // Clear gas first so abilityOnSwitchIn does not immediately re-suppress.
        leaving.abilityId.clear();
        otherBattler.abilitySuppressed = false;
        auto other = playerSide ? data.opponentPkmn : data.playerPkmn;
        if (other && !other->isKO())
        {
            refreshPlateType(other, otherBattler);
            refreshForme(data, !playerSide);
            abilityOnSwitchIn(data, !playerSide, data.pendingAbilityMessages);
        }
    }
}

void abilityOnSwitchOut(SingleBattleSceneData& data, bool playerSide)
{
    auto pkmn = playerSide ? data.playerPkmn : data.opponentPkmn;
    BattleBattler const& battler = playerSide ? data.playerBattler : data.opponentBattler;
    if (!pkmn || !active(battler))
        return;
    if (battler.abilityId == "NATURALCURE")
        pkmn->setStatusCondition(Pkmn::StatusCondition::NONE);
    if (battler.abilityId == "REGENERATOR" && battler.healBlockTurns <= 0)
        pkmn->setHP(std::min(pkmn->getTotalHP(), pkmn->getHP() + std::max<size_t>(1, pkmn->getTotalHP() / 3)));
    // clearTrapsOnLeave also releases Neutralizing Gas and re-runs the foe's switch-in.
    clearTrapsOnLeave(data, playerSide);
    if (battler.abilityId == "PRIMORDIALSEA" || battler.abilityId == "DESOLATELAND" || battler.abilityId == "DELTASTREAM")
    {
        BattleBattler const& otherBattler = playerSide ? data.opponentBattler : data.playerBattler;
        auto otherId = active(otherBattler) ? otherBattler.abilityId : std::string{};
        bool otherKeeps = otherId == "PRIMORDIALSEA" || otherId == "DESOLATELAND" || otherId == "DELTASTREAM";
        if (!otherKeeps)
        {
            if (battler.abilityId == "DELTASTREAM")
                data.strongWinds = false;
            if (data.weatherTurns < 0)
            {
                data.weatherTurns    = 0;
                data.weatherFromMove = false;
                data.weather         = Map::Weather::NONE;
            }
        }
        else if (battler.abilityId == "DELTASTREAM" && otherId != "DELTASTREAM")
            data.strongWinds = false;
    }
}

float abilityTypePowerMultiplier(Pkmn::PkmnPtr const& user, BattleBattler const& battler, std::string const& type)
{
    if (!user || !active(battler) || type.empty())
        return 1.f;
    bool pinch = user->getHP() * 2 <= user->getTotalHP();
    if (pinch && ((battler.abilityId == "OVERGROW" && type == "GRASS") || (battler.abilityId == "BLAZE" && type == "FIRE")
                  || (battler.abilityId == "TORRENT" && type == "WATER") || (battler.abilityId == "SWARM" && type == "BUG")))
        return 1.5f;
    if (battler.abilityId == "FLASHFIRE" && battler.flashFireBoost && type == "FIRE")
        return 1.5f;
    return 1.f;
}

float abilityOutgoingMultiplier(Pkmn::PkmnPtr const& user, BattleBattler const& battler, MoveDef::MoveDefPtr const& move,
                                bool targetMoved, Map::Weather weather, float effectiveness, bool critical,
                                std::string moveType)
{
    if (!user || !move || !active(battler))
        return 1.f;
    if (moveType.empty())
        moveType = move->getType();
    float mult = 1.f;
    if (battler.abilityId == "TECHNICIAN" && move->getPower() > 0 && move->getPower() <= 60)
        mult *= 1.5f;
    if (battler.abilityId == "SHEERFORCE" && move->getEffectChance() > 0)
        mult *= 1.3f;
    if (battler.abilityId == "WATERBUBBLE" && moveType == "WATER")
        mult *= 2.f;
    if (battler.abilityId == "TOUGHCLAWS" && move->hasFlag(MoveDef::Flag::Contact))
        mult *= 1.3f;
    if (battler.abilityId == "IRONFIST" && move->hasFlag(MoveDef::Flag::Punching))
        mult *= 1.2f;
    if (battler.abilityId == "MEGALAUNCHER" && move->hasFlag(MoveDef::Flag::Pulse))
        mult *= 1.5f;
    if (battler.abilityId == "ANALYTIC" && targetMoved)
        mult *= 1.3f;
    if (battler.abilityId == "HUSTLE" && move->getCategory() == MoveDef::PHYSICAL)
        mult *= 1.5f;
    if (battler.abilityId == "STRONGJAW" && move->hasFlag(MoveDef::Flag::Biting))
        mult *= 1.5f;
    if (battler.abilityId == "PUNKROCK" && move->hasFlag(MoveDef::Flag::Sound))
        mult *= 1.3f;
    auto code = move->getFunctionCode();
    bool recoil = code == MoveDef::FunctionCode::RecoilQuarterOfDamageDealt
               || code == MoveDef::FunctionCode::RecoilThirdOfDamageDealt
               || code == MoveDef::FunctionCode::RecoilThirdOfDamageDealtBurnTarget
               || code == MoveDef::FunctionCode::RecoilThirdOfDamageDealtParalyzeTarget
               || code == MoveDef::FunctionCode::RecoilHalfOfDamageDealt
               || code == MoveDef::FunctionCode::CrashDamageIfFailsUnusableInGravity;
    if (battler.abilityId == "RECKLESS" && recoil)
        mult *= 1.2f;
    if (battler.abilityId == "FLAREBOOST" && move->getCategory() == MoveDef::SPECIAL
        && user->getStatusCondition() == Pkmn::StatusCondition::BURN)
        mult *= 1.5f;
    if (battler.abilityId == "TOXICBOOST" && move->getCategory() == MoveDef::PHYSICAL
        && (user->getStatusCondition() == Pkmn::StatusCondition::POISON
            || user->getStatusCondition() == Pkmn::StatusCondition::BADLY_POISON))
        mult *= 1.5f;
    if (battler.abilityId == "DEFEATIST" && user->getHP() * 2 <= user->getTotalHP())
        mult *= 0.5f;
    if (battler.abilityId == "SOLARPOWER" && move->getCategory() == MoveDef::SPECIAL
        && weather == Map::Weather::HARSH_SUNLIGHT
        && ItemUtils::activeHeldId(user, &battler) != "UTILITYUMBRELLA")
        mult *= 1.5f;
    if (battler.abilityId == "TINTEDLENS" && effectiveness > 0.f && effectiveness < 1.f)
        mult *= 2.f;
    if (battler.abilityId == "NEUROFORCE" && effectiveness > 1.f)
        mult *= 1.25f;
    if (battler.abilityId == "SNIPER" && critical)
        mult *= 1.5f;
    return mult;
}

bool moveIgnoresTargetAbility(BattleBattler const& user, MoveDef::FunctionCode code)
{
    if (code == MoveDef::FunctionCode::IgnoreTargetAbility
        || code == MoveDef::FunctionCode::CategoryDependsOnHigherDamageIgnoreTargetAbility)
        return true;
    if (user.abilitySuppressed)
        return false;
    return user.abilityId == "MOLDBREAKER" || user.abilityId == "TERAVOLT" || user.abilityId == "TURBOBLAZE";
}

float abilityIncomingMultiplier(Pkmn::PkmnPtr const& target, BattleBattler const& battler, std::string const& moveType,
                                float effectiveness, bool critical, MoveDef::Category category, bool contact,
                                bool ignoreAbility, bool strongWinds)
{
    (void)critical;
    if (!target || !active(battler))
        return 1.f;
    float mult = 1.f;
    if (battler.abilityId == "PRISMARMOR" && effectiveness > 1.f)
        mult *= 0.75f;
    if (!ignoreAbility && (battler.abilityId == "FILTER" || battler.abilityId == "SOLIDROCK") && effectiveness > 1.f)
        mult *= 0.75f;
    if (!ignoreAbility && battler.abilityId == "FURCOAT" && category == MoveDef::PHYSICAL)
        mult *= 0.5f;
    if (!ignoreAbility && battler.abilityId == "ICESCALES" && category == MoveDef::SPECIAL)
        mult *= 0.5f;
    if (!ignoreAbility && battler.abilityId == "FLUFFY")
    {
        if (contact)
            mult *= 0.5f;
        if (moveType == "FIRE")
            mult *= 2.f;
    }
    if (battler.abilityId == "SHADOWSHIELD" && target->getHP() == target->getTotalHP())
        mult *= 0.5f;
    if (!ignoreAbility && battler.abilityId == "MULTISCALE" && target->getHP() == target->getTotalHP())
        mult *= 0.5f;
    if (!ignoreAbility && battler.abilityId == "THICKFAT" && (moveType == "FIRE" || moveType == "ICE"))
        mult *= 0.5f;
    if (!ignoreAbility && battler.abilityId == "HEATPROOF" && moveType == "FIRE")
        mult *= 0.5f;
    if (!ignoreAbility && battler.abilityId == "WATERBUBBLE" && moveType == "FIRE")
        mult *= 0.5f;
    if (!ignoreAbility && battler.abilityId == "DRYSKIN" && moveType == "FIRE")
        mult *= 1.25f;
    // Strong Winds / Delta Stream: super-effective hits vs Flying become neutral.
    if ((strongWinds || battler.abilityId == "DELTASTREAM") && hasType(target, "FLYING", &battler)
        && effectiveness > 1.f)
        mult /= effectiveness;
    if (!ignoreAbility && battler.abilityId == "MARVELSCALE" && category == MoveDef::PHYSICAL
        && target->getStatusCondition() != Pkmn::StatusCondition::NONE)
        mult *= 2.f / 3.f;
    return mult;
}

bool primordialWeatherBlocksNewWeather(SingleBattleSceneData const& data)
{
    if (data.strongWinds)
        return true;
    auto primordial = [](BattleBattler const& battler) {
        if (battler.abilitySuppressed || battler.abilityId.empty())
            return false;
        return battler.abilityId == "PRIMORDIALSEA" || battler.abilityId == "DESOLATELAND"
            || battler.abilityId == "DELTASTREAM";
    };
    return primordial(data.playerBattler) || primordial(data.opponentBattler);
}

std::string battleDisplayName(Pkmn::PkmnPtr const& pkmn, BattleBattler const& battler)
{
    if (battler.transformDisguise)
        return battler.transformDisguise->getDisplayName();
    if (battler.illusionActive && battler.illusionDisguise)
        return battler.illusionDisguise->getDisplayName();
    return pkmn ? pkmn->getDisplayName() : std::string();
}

std::string predictedIllusionName(Pkmn::PkmnPtr const& pkmn, bool playerSide, SingleBattleSceneData const& data)
{
    if (!pkmn || pkmn->effectiveAbilityId() != "ILLUSION")
        return pkmn ? pkmn->getDisplayName() : std::string();
    Pkmn::PkmnPtr disguise;
    auto          consider = [&](auto const& party) {
        for (auto const& member : party)
        {
            if (member && member != pkmn && !member->isKO())
                disguise = member;
        }
    };
    if (playerSide)
        consider(Game::instance()->data.player.pkmns);
    else if (auto const* trainer = dynamic_cast<TrainerSceneData const*>(&data))
    {
        if (trainer->opponentTrainer)
            consider(trainer->opponentTrainer->pkmns);
    }
    return disguise ? disguise->getDisplayName() : pkmn->getDisplayName();
}

void abilityOnTargetFaint(SingleBattleSceneData& data, bool attackerIsPlayer, std::vector<std::string>& messages)
{
    auto attacker = attackerIsPlayer ? data.playerPkmn : data.opponentPkmn;
    BattleBattler& battler = attackerIsPlayer ? data.playerBattler : data.opponentBattler;
    if (!attacker || !active(battler))
        return;
    auto boost = [&](BattleBattler::StageStat stat) {
        auto raised = applyStatChange(data, battler, attacker, stat, 1, messages, false);
        if (raised.ejected)
            noteEject(data, battler);
    };
    if (battler.abilityId == "BATTLEBOND" && !battler.battleBondActive)
    {
        battler.battleBondActive  = true;
        battler.spriteFormeSuffix = "_1";
        applyFormeBases(attacker, battler, 145, 67, 153, 71, 132);
        messages.push_back(
            (boost::format(lc::translate("%1% became Ash-Greninja with Battle Bond!")) % nameOf(attacker)).str());
    }
    else if (battler.abilityId == "MOXIE" || battler.abilityId == "CHILLINGNEIGH" || battler.abilityId == "ASONECHILLINGNEIGH")
        boost(BattleBattler::StageStat::Attack);
    else if (battler.abilityId == "BEASTBOOST")
    {
        auto const& stats = attacker->getStats();
        struct StatPick
        {
            size_t value;
            BattleBattler::StageStat stat;
        };
        StatPick picks[] = {
            {stats.at(PkmnDef::ATTACK), BattleBattler::StageStat::Attack},
            {stats.at(PkmnDef::DEFENSE), BattleBattler::StageStat::Defense},
            {stats.at(PkmnDef::SPECIAL_ATTACK), BattleBattler::StageStat::SpecialAttack},
            {stats.at(PkmnDef::SPECIAL_DEFENSE), BattleBattler::StageStat::SpecialDefense},
            {stats.at(PkmnDef::SPEED), BattleBattler::StageStat::Speed},
        };
        StatPick best = picks[0];
        for (auto const& pick : picks)
        {
            if (pick.value > best.value)
                best = pick;
        }
        boost(best.stat);
    }
    else if (battler.abilityId == "GRIMNEIGH" || battler.abilityId == "SOULHEART" || battler.abilityId == "ASONEGRIMNEIGH")
        boost(BattleBattler::StageStat::SpecialAttack);
}

float abilitySpeedMultiplier(Pkmn::PkmnPtr const& pkmn, BattleBattler const& battler, Map::Weather weather)
{
    if (!active(battler))
        return 1.f;
    float mult = 1.f;
    bool umbrella = ItemUtils::activeHeldId(pkmn, &battler) == "UTILITYUMBRELLA";
    if (!umbrella && battler.abilityId == "SWIFTSWIM" && weather == Map::Weather::RAIN)
        mult *= 2.f;
    if (!umbrella && battler.abilityId == "CHLOROPHYLL" && weather == Map::Weather::HARSH_SUNLIGHT)
        mult *= 2.f;
    if (battler.abilityId == "SANDRUSH" && weather == Map::Weather::SANDSTORM)
        mult *= 2.f;
    if (battler.abilityId == "SLUSHRUSH" && weather == Map::Weather::HAIL)
        mult *= 2.f;
    if (battler.slowStartTurns > 0)
        mult *= 0.5f;
    if (battler.abilityId == "QUICKFEET" && pkmn && pkmn->getStatusCondition() != Pkmn::StatusCondition::NONE)
        mult *= 1.5f;
    if (battler.unburdenActive)
        mult *= 2.f;
    // Stall is a same-priority tie-break (see moves phase), not a Speed cut.
    return mult;
}

bool abilityGrantsImmunity(SingleBattleSceneData& data,
                           Pkmn::PkmnPtr const&    target,
                           BattleBattler&         targetBattler,
                           std::string const&     moveType,
                           bool                   ignoreAbility,
                           std::vector<std::string>& messages,
                           bool                   pierceGroundImmunity)
{
    if (!target || ignoreAbility || !active(targetBattler))
        return false;
    auto id = targetBattler.abilityId;
    if (id == "LEVITATE" && moveType == "GROUND" && !targetBattler.groundedByMove
        && !pierceGroundImmunity)
    {
        messages.push_back((boost::format(lc::translate("%1%'s Levitate made it immune!")) % nameOf(target)).str());
        return true;
    }
    if (id == "FLASHFIRE" && moveType == "FIRE")
    {
        targetBattler.flashFireBoost = true;
        messages.push_back((boost::format(lc::translate("%1%'s Flash Fire raised its Fire power!")) % nameOf(target)).str());
        return true;
    }
    if ((id == "WATERABSORB" || id == "DRYSKIN") && moveType == "WATER")
    {
        if (targetBattler.healBlockTurns <= 0)
        {
            size_t heal = std::max<size_t>(1, target->getTotalHP() / 4);
            target->setHP(std::min(target->getTotalHP(), target->getHP() + heal));
            messages.push_back((boost::format(lc::translate("%1%'s %2% restored its HP!")) % nameOf(target) % id).str());
        }
        return true;
    }
    if (id == "VOLTABSORB" && moveType == "ELECTRIC")
    {
        if (targetBattler.healBlockTurns <= 0)
            target->setHP(std::min(target->getTotalHP(), target->getHP() + std::max<size_t>(1, target->getTotalHP() / 4)));
        messages.push_back((boost::format(lc::translate("%1%'s %2% took the attack!")) % nameOf(target) % id).str());
        return true;
    }
    if (id == "STORMDRAIN" && moveType == "WATER")
    {
        auto outcome = applyStatChange(data, targetBattler, target, BattleBattler::StageStat::SpecialAttack, 1, messages, false);
        if (outcome.ejected)
            noteEject(data, targetBattler);
        messages.push_back((boost::format(lc::translate("%1%'s Storm Drain took the attack!")) % nameOf(target)).str());
        return true;
    }
    if (id == "SAPSIPPER" && moveType == "GRASS")
    {
        auto outcome = applyStatChange(data, targetBattler, target, BattleBattler::StageStat::Attack, 1, messages, false);
        if (outcome.ejected)
            noteEject(data, targetBattler);
        if (outcome.changed && outcome.applied > 0)
            messages.push_back((boost::format(lc::translate("%1%'s Sap Sipper raised its Attack!")) % nameOf(target)).str());
        return true;
    }
    return false;
}
