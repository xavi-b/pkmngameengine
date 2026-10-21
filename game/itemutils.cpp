#include "itemutils.h"

#include "game.h"
#include "scenes/battle/abilitybattle.h"
#include "scenes/battle/battlebattler.h"
#include "scenes/battle/confusionstatus.h"
#include "scenes/battle/moveeffects.h"
#include "scenes/battle/statuscondition.h"
#include "utils.h"

#include <algorithm>
#include <boost/format.hpp>
#include <boost/locale.hpp>
#include <cctype>
#include <limits>

namespace lc = boost::locale;

namespace
{

size_t healingAmountForItem(std::string const& itemId)
{
    // Amounts match Essentials/PBS descriptions (and Gen III where they differ from old lists).
    if (itemId == "POTION" || itemId == "BERRYJUICE" || itemId == "SWEETHEART")
        return 20;
    if (itemId == "SUPERPOTION")
        return 60;
    if (itemId == "HYPERPOTION")
        return 120;
    if (itemId == "FRESHWATER")
        return 30;
    if (itemId == "SODAPOP")
        return 50;
    if (itemId == "LEMONADE")
        return 70;
    if (itemId == "MOOMOOMILK")
        return 100;
    if (itemId == "ORANBERRY")
        return 10;
    if (itemId == "SITRUSBERRY")
        return 30; // Gen III fixed heal when used from bag
    if (itemId == "ENERGYPOWDER")
        return 50;
    if (itemId == "ENERGYROOT")
        return 200;
    if (itemId == "MAXPOTION" || itemId == "FULLRESTORE" || itemId == "MAXHONEY")
        return std::numeric_limits<size_t>::max();
    return 0;
}

bool curesStatus(std::string const& itemId, Pkmn::StatusCondition status)
{
    if (status == Pkmn::StatusCondition::NONE)
        return false;
    if (itemId == "FULLHEAL" || itemId == "FULLRESTORE" || itemId == "LUMBERRY" || itemId == "LAVACOOKIE"
        || itemId == "OLDGATEAU" || itemId == "CASTELIACONE" || itemId == "LUMIOSEGALETTE" || itemId == "SHALOURSABLE"
        || itemId == "BIGMALASADA" || itemId == "HEALPOWDER" || itemId == "PEWTERCRUNCHIES"
        || itemId == "RAGECANDYBAR")
        return true;
    if ((itemId == "AWAKENING" || itemId == "CHESTOBERRY" || itemId == "POKEFLUTE" || itemId == "BLUEFLUTE")
        && status == Pkmn::StatusCondition::SLEEP)
        return true;
    if ((itemId == "ANTIDOTE" || itemId == "PECHABERRY")
        && (status == Pkmn::StatusCondition::POISON || status == Pkmn::StatusCondition::BADLY_POISON))
        return true;
    if ((itemId == "BURNHEAL" || itemId == "RAWSTBERRY") && status == Pkmn::StatusCondition::BURN)
        return true;
    if ((itemId == "PARALYZEHEAL" || itemId == "CHERIBERRY") && status == Pkmn::StatusCondition::PARALYSIS)
        return true;
    if ((itemId == "ICEHEAL" || itemId == "ASPEARBERRY") && status == Pkmn::StatusCondition::FREEZE)
        return true;
    return false;
}

bool restorePPOnMove(Move::MovePtr const& move, size_t amount, bool toMax)
{
    if (!move || !move->getDefinition())
        return false;
    size_t maxPp = move->getDefinition()->getTotalPP();
    size_t cur   = move->getCurrentPP();
    if (cur >= maxPp)
        return false;
    if (toMax)
        move->setCurrentPP(maxPp);
    else
        move->setCurrentPP(std::min(maxPp, cur + amount));
    return true;
}

bool restorePPForItem(std::string const& itemId, Pkmn::PkmnPtr const& target)
{
    if (!target)
        return false;

    bool used = false;
    if (itemId == "ELIXIR")
    {
        for (auto& move : target->getMoves())
            used = restorePPOnMove(move, 10, false) || used;
        return used;
    }
    if (itemId == "MAXELIXIR")
    {
        for (auto& move : target->getMoves())
            used = restorePPOnMove(move, 0, true) || used;
        return used;
    }
    if (itemId == "ETHER" || itemId == "LEPPABERRY")
    {
        // No move picker yet: restore the first move that is missing PP.
        for (auto& move : target->getMoves())
        {
            if (restorePPOnMove(move, 10, false))
                return true;
        }
        return false;
    }
    if (itemId == "MAXETHER")
    {
        for (auto& move : target->getMoves())
        {
            if (restorePPOnMove(move, 0, true))
                return true;
        }
        return false;
    }
    return false;
}

int xItemStageDelta(std::string const& itemId)
{
    if (itemId.size() >= 1 && itemId.back() == '6')
        return 6;
    if (itemId.size() >= 1 && itemId.back() == '3')
        return 3;
    if (itemId.size() >= 1 && itemId.back() == '2')
        return 2;
    // A plain X item raises by 2. Suffixed 2, 3, and 6 items keep that stage.
    return 2;
}

bool applyXStat(std::string const& itemId, SingleBattleSceneData& data, Pkmn::PkmnPtr const& pkmn, BattleBattler& battler,
                std::vector<std::string>& messages)
{
    using SS = BattleBattler::StageStat;
    int delta = xItemStageDelta(itemId);
    auto raise = [&](SS stat, int amount) {
        auto outcome = applyStatChange(data, battler, pkmn, stat, amount, messages, false);
        if (outcome.ejected)
            data.playerEjectSwitch = true;
        return outcome.changed;
    };

    if (itemId.rfind("XATTACK", 0) == 0)
        return raise(SS::Attack, delta);
    if (itemId.rfind("XDEFENSE", 0) == 0 || itemId.rfind("XDEFEND", 0) == 0)
        return raise(SS::Defense, delta);
    if (itemId.rfind("XSPATK", 0) == 0 || itemId.rfind("XSPECIAL", 0) == 0)
        return raise(SS::SpecialAttack, delta);
    if (itemId.rfind("XSPDEF", 0) == 0)
        return raise(SS::SpecialDefense, delta);
    if (itemId.rfind("XSPEED", 0) == 0)
        return raise(SS::Speed, delta);
    if (itemId.rfind("XACCURACY", 0) == 0)
        return raise(SS::Accuracy, delta);
    if (itemId == "MAXMUSHROOMS")
    {
        bool changed = false;
        changed |= raise(SS::Attack, 1);
        changed |= raise(SS::Defense, 1);
        changed |= raise(SS::SpecialAttack, 1);
        changed |= raise(SS::SpecialDefense, 1);
        changed |= raise(SS::Speed, 1);
        return changed;
    }
    if (itemId.rfind("DIREHIT", 0) == 0)
    {
        if (battler.focusEnergy)
            return false;
        battler.focusEnergy = 2;
        return true;
    }
    return false;
}

bool speciesHasType(PkmnDef::PkmnDefPtr const& def, std::string const& typeId)
{
    if (!def)
        return false;
    for (auto const& type : def->getTypes())
    {
        if (type == typeId)
            return true;
    }
    return false;
}

bool evolvesByMoonStone(PkmnDef::PkmnDefPtr const& def)
{
    if (!def)
        return false;
    auto const range = def->getEvolutions().equal_range(PkmnDef::EvolutionType::Item);
    for (auto it = range.first; it != range.second; ++it)
    {
        if (it->second.data == "MOONSTONE")
            return true;
    }
    return false;
}

bool speciesAlreadyOwned(std::string const& speciesId)
{
    if (speciesId.empty() || !Game::instance())
        return false;
    for (auto const& mon : Game::instance()->data.player.pkmns)
    {
        if (mon && mon->getDefinition() && mon->getDefinition()->getId() == speciesId)
            return true;
    }
    return Game::instance()->data.storage.containsSpecies(speciesId);
}

bool isUltraBeast(std::string const& speciesId)
{
    return speciesId == "NIHILEGO" || speciesId == "BUZZWOLE" || speciesId == "PHEROMOSA" || speciesId == "XURKITREE"
        || speciesId == "CELESTEELA" || speciesId == "KARTANA" || speciesId == "GUZZLORD" || speciesId == "POIPOLE"
        || speciesId == "NAGANADEL" || speciesId == "STAKATAKA" || speciesId == "BLACEPHALON";
}

float ballBonusForId(std::string const&                 itemId,
                     ItemUtils::CaptureContext const&   context,
                     Pkmn::PkmnPtr const&               foe)
{
    auto def = foe ? foe->getDefinition() : nullptr;
    if (itemId == "MASTERBALL")
        return 255.0f;
    if (itemId == "ULTRABALL" || itemId == "SPORTBALL")
        return 2.0f;
    if (itemId == "GREATBALL" || itemId == "SAFARIBALL")
        return 1.5f;
    if (itemId == "DIVEBALL")
        return (context.underwater || context.fishing) ? 3.5f : 1.0f;
    if (itemId == "DUSKBALL")
        return (context.atNight || context.inCave) ? 3.5f : 1.0f;
    if (itemId == "QUICKBALL")
        return context.turn <= 1 ? 5.0f : 1.0f;
    if (itemId == "TIMERBALL")
    {
        // Gen VII: 1 + turns passed * 1229/4096, capped at 4. The first turn has passed none.
        int   passed = std::max(0, context.turn - 1);
        float bonus  = 1.0f + static_cast<float>(passed) * 1229.0f / 4096.0f;
        return std::min(4.0f, bonus);
    }
    if (itemId == "NETBALL")
        return (speciesHasType(def, "BUG") || speciesHasType(def, "WATER")) ? 3.5f : 1.0f;
    if (itemId == "NESTBALL")
    {
        size_t level = foe ? foe->getLevel() : 30;
        if (level >= 30)
            return 1.0f;
        return static_cast<float>(41 - level) / 10.0f;
    }
    if (itemId == "FASTBALL")
    {
        size_t speed = 0;
        if (def)
        {
            auto const& stats = def->getBaseStats();
            auto        it    = stats.find(PkmnDef::SPEED);
            if (it != stats.end())
                speed = it->second;
        }
        return speed >= 100 ? 4.0f : 1.0f;
    }
    if (itemId == "LEVELBALL")
    {
        size_t wild = foe ? foe->getLevel() : 0;
        if (wild == 0 || context.playerLevel == 0)
            return 1.0f;
        if (context.playerLevel / 4 >= wild)
            return 8.0f;
        if (context.playerLevel / 2 >= wild)
            return 4.0f;
        if (context.playerLevel > wild)
            return 2.0f;
        return 1.0f;
    }
    if (itemId == "LOVEBALL")
    {
        if (!foe || !def || context.playerSpeciesId.empty())
            return 1.0f;
        bool opposite = context.playerGender != foe->getGender() && context.playerGender != Pkmn::Gender::GENDERLESS
                     && foe->getGender() != Pkmn::Gender::GENDERLESS;
        return (opposite && def->getId() == context.playerSpeciesId) ? 8.0f : 1.0f;
    }
    if (itemId == "MOONBALL")
        return evolvesByMoonStone(def) ? 4.0f : 1.0f;
    if (itemId == "LUREBALL")
        return context.fishing ? 5.0f : 1.0f;
    if (itemId == "DREAMBALL")
        return foe && foe->getStatusCondition() == Pkmn::StatusCondition::SLEEP ? 4.0f : 1.0f;
    if (itemId == "BEASTBALL")
        return def && isUltraBeast(def->getId()) ? 5.0f : 410.0f / 4096.0f;
    if (itemId == "REPEATBALL")
        return def && speciesAlreadyOwned(def->getId()) ? 3.5f : 1.0f;
    return 1.0f; // Poké Ball, Premier, Luxury, Heal, Cherish, Friend, Heavy, and unknown
}

int heavyBallCatchModifier(PkmnDef::PkmnDefPtr const& def)
{
    float weight = def ? def->getWeight() : 0.f;
    if (weight < 102.4f)
        return -20;
    if (weight < 204.8f)
        return 0;
    if (weight < 307.2f)
        return 20;
    if (weight < 409.6f)
        return 30;
    return 40;
}

bool isPoisonType(Pkmn::PkmnPtr const& pkmn, BattleBattler const* battler = nullptr)
{
    return hasType(pkmn, "POISON", battler);
}

} // namespace

bool ItemUtils::curesConfusion(std::string const& itemId)
{
    return itemId == "PERSIMBERRY" || itemId == "YELLOWFLUTE" || itemId == "FULLHEAL" || itemId == "FULLRESTORE"
        || itemId == "LUMBERRY" || itemId == "LAVACOOKIE" || itemId == "OLDGATEAU" || itemId == "CASTELIACONE"
        || itemId == "LUMIOSEGALETTE" || itemId == "SHALOURSABLE" || itemId == "BIGMALASADA" || itemId == "HEALPOWDER"
        || itemId == "PEWTERCRUNCHIES" || itemId == "RAGECANDYBAR";
}

bool ItemUtils::isEscapeItem(std::string const& itemId)
{
    return itemId == "POKEDOLL" || itemId == "FLUFFYTAIL" || itemId == "POKETOY";
}

bool ItemUtils::useItemOnPkmn(Item::ItemPtr const& item, Pkmn::PkmnPtr const& target, bool inBattle)
{
    if (!item || !item->getDefinition() || !target)
        return false;

    std::string itemId = item->getDefinition()->getId();
    bool        used   = false;

    if (itemId == "RARECANDY")
    {
        if (inBattle)
            return false;

        constexpr size_t MaxLevel = 100;
        if (target->getLevel() >= MaxLevel)
            return false;

        size_t oldMax = target->getTotalHP();
        size_t oldHp  = target->getHP();
        target->incrementLevel();
        size_t newMax = target->getTotalHP();
        if (oldHp > 0)
        {
            if (newMax >= oldMax)
                target->setHP(std::min(newMax, oldHp + (newMax - oldMax)));
            else
                target->setHP(std::min(oldHp, newMax));
        }
        return true;
    }

    size_t healing = healingAmountForItem(itemId);
    if (healing > 0 && !target->isKO())
    {
        size_t maxHp = target->getTotalHP();
        size_t newHp = (itemId == "MAXPOTION" || itemId == "FULLRESTORE" || itemId == "MAXHONEY")
                         ? maxHp
                         : std::min(maxHp, target->getHP() + healing);
        if (newHp != target->getHP())
        {
            target->setHP(newHp);
            used = true;
        }
    }

    if (allowFaintedTarget(item) && target->isKO())
    {
        target->setHP(itemId == "REVIVE" ? std::max<size_t>(1, target->getTotalHP() / 2) : target->getTotalHP());
        used = true;
    }

    if (curesStatus(itemId, target->getStatusCondition()))
    {
        target->setStatusCondition(Pkmn::StatusCondition::NONE);
        used = true;
    }

    if (inBattle && itemId == "FULLRESTORE")
        target->setStatusCondition(Pkmn::StatusCondition::NONE);

    // Elixir family is BattleUse::OnPokemon in PBS
    if (restorePPForItem(itemId, target))
        used = true;

    return used;
}

bool ItemUtils::useItemOnMove(Item::ItemPtr const& item, Pkmn::PkmnPtr const& target)
{
    if (!item || !item->getDefinition() || !target)
        return false;
    return restorePPForItem(item->getDefinition()->getId(), target);
}

bool ItemUtils::useItemOnBattler(Item::ItemPtr const& item,
                                 Pkmn::PkmnPtr const&  target,
                                 BattleBattler&       battler,
                                 ConfusionStatus*     confusion,
                                 SingleBattleSceneData& data,
                                 std::vector<std::string>& messages)
{
    if (!item || !item->getDefinition() || !target)
        return false;

    std::string itemId = item->getDefinition()->getId();
    bool        used   = false;

    if (applyXStat(itemId, data, target, battler, messages))
        used = true;

    if (curesConfusion(itemId) && confusion && confusion->active)
    {
        confusion->clear();
        used = true;
    }

    if (itemId == "REDFLUTE" && battler.infatuated)
    {
        battler.infatuated = false;
        used               = true;
    }

    return used;
}

bool ItemUtils::useBattleItemDirect(Item::ItemPtr const& item,
                                    Pkmn::PkmnPtr const&  playerPkmn,
                                    BattleBattler&       playerBattler,
                                    bool&                endsBattle,
                                    std::string&         message,
                                    int&                 sideMistTurns)
{
    endsBattle = false;
    message.clear();
    (void)playerBattler;
    if (!item || !item->getDefinition())
        return false;

    std::string itemId = item->getDefinition()->getId();

    if (itemId == "GUARDSPEC")
    {
        if (sideMistTurns > 0)
            return false;
        sideMistTurns = 5;
        message       = lc::translate("Your team became shrouded in mist!");
        return true;
    }

    if (isEscapeItem(itemId))
    {
        endsBattle = true;
        message    = lc::translate("Got away safely!");
        return true;
    }

    if (itemId == "POKEFLUTE")
    {
        if (!playerPkmn || playerPkmn->getStatusCondition() != Pkmn::StatusCondition::SLEEP)
            return false;
        playerPkmn->setStatusCondition(Pkmn::StatusCondition::NONE);
        message = (boost::format(lc::translate("%1% woke up!")) % playerPkmn->getDisplayName()).str();
        return true;
    }

    return false;
}

bool ItemUtils::useCaptureBall(Item::ItemPtr const&        ball,
                               Pkmn::PkmnPtr const&        foePkmn,
                               CaptureContext const&       context)
{
    if (!ball || !ball->getDefinition() || !foePkmn || foePkmn->isKO())
        return false;

    auto def = ball->getDefinition();
    if (!def->hasFlag(ItemDef::Flag::PokeBall) && def->getBattleUse() != ItemDef::BattleUse::OnFoe)
        return false;

    std::string itemId = def->getId();

    auto applyCatchEffect = [&]() {
        if (itemId == "HEALBALL")
        {
            foePkmn->resetHP();
            foePkmn->setStatusCondition(Pkmn::StatusCondition::NONE);
        }
        else if (itemId == "FRIENDBALL")
            foePkmn->setHappiness(200);
    };

    if (itemId == "MASTERBALL")
    {
        applyCatchEffect();
        return true;
    }

    if (!foePkmn->getDefinition())
        return false;

    float ballBonus = ballBonusForId(itemId, context, foePkmn);

    size_t maxHp = std::max<size_t>(1, foePkmn->getTotalHP());
    size_t hp    = foePkmn->getHP();
    int    catchRate = static_cast<int>(foePkmn->getDefinition()->getCatchRate());
    if (itemId == "HEAVYBALL")
        catchRate = std::max(1, catchRate + heavyBallCatchModifier(foePkmn->getDefinition()));
    float a = ((3.0f * maxHp - 2.0f * hp) * static_cast<float>(catchRate) * ballBonus) / (3.0f * maxHp);

    // Status modifiers (Gen 5–8)
    switch (foePkmn->getStatusCondition())
    {
    case Pkmn::StatusCondition::SLEEP:
    case Pkmn::StatusCondition::FREEZE:
        a *= 2.5f;
        break;
    case Pkmn::StatusCondition::PARALYSIS:
    case Pkmn::StatusCondition::POISON:
    case Pkmn::StatusCondition::BADLY_POISON:
    case Pkmn::StatusCondition::BURN:
        a *= 1.5f;
        break;
    default:
        break;
    }

    float chance = std::clamp(a / 255.0f, 0.0f, 1.0f);
    if (Utils::randuint(0, 10000) > static_cast<size_t>(chance * 10000.0f))
        return false;
    applyCatchEffect();
    return true;
}

bool ItemUtils::useItemInField(Player& player, Item::ItemPtr const& item, std::string& message)
{
    message.clear();
    if (!item || !item->getDefinition())
        return false;

    bool used = false;
    switch (item->getDefinition()->getFieldUse())
    {
    case ItemDef::FieldUse::Direct:
    {
        std::string itemId = item->getDefinition()->getId();
        if (itemId == "REPEL" || itemId == "SUPERREPEL" || itemId == "MAXREPEL")
        {
            if (player.repelSteps > 0)
            {
                message = lc::translate("It won't have any effect.");
                return false;
            }
            if (itemId == "REPEL")
                player.repelSteps = 100;
            else if (itemId == "SUPERREPEL")
                player.repelSteps = 200;
            else
                player.repelSteps = 250;
            message = (boost::format(lc::translate("%1% used the %2%.")) % player.name % item->getDefinition()->getName())
                           .str();
            used = true;
            break;
        }
        if (itemId == "WHITEFLUTE" || itemId == "BLACKFLUTE")
        {
            if (itemId == "WHITEFLUTE")
                player.flute = Player::Flute::White;
            else
                player.flute = Player::Flute::Black;
            message = (boost::format(lc::translate("%1% used the %2%.")) % player.name % item->getDefinition()->getName())
                           .str();
            used = true;
            break;
        }
        used = true; // The other direct items still to come.
        break;
    }
    case ItemDef::FieldUse::OnPkmn:
        for (auto const& pkmn : player.pkmns)
        {
            if (!pkmn)
                continue;
            if (useItemOnPkmn(item, pkmn, false))
            {
                used = true;
                break;
            }
        }
        break;
    case ItemDef::FieldUse::TR:
    case ItemDef::FieldUse::HM:
    case ItemDef::FieldUse::None:
    default:
        used = false;
        break;
    }

    if (used && item->getDefinition()->isConsumable())
        player.removeOneItem(item->getDefinition());
    return used;
}

bool ItemUtils::allowFaintedTarget(Item::ItemPtr const& item)
{
    if (!item || !item->getDefinition())
        return false;
    auto const& id = item->getDefinition()->getId();
    return id == "REVIVE" || id == "MAXREVIVE" || id == "REVIVALHERB" || id == "MAXHONEY";
}

ItemUtils::HeldItemTickResult ItemUtils::consumeWhiteHerb(Pkmn::PkmnPtr const& pkmn, BattleBattler* battler)
{
    HeldItemTickResult result;
    if (!pkmn || pkmn->isKO() || !battler || activeHeldId(pkmn, battler) != "WHITEHERB")
        return result;
    bool lowered = false;
    for (size_t i = 0; i < BattleBattler::StageStatCount; ++i)
    {
        if (battler->stages[i] < 0)
            lowered = true;
    }
    if (!lowered)
        return result;
    for (size_t i = 0; i < BattleBattler::StageStatCount; ++i)
    {
        if (battler->stages[i] < 0)
            battler->stages[i] = 0;
    }
    result.message = (boost::format(lc::translate("%1%'s White Herb restored its stats!")) % pkmn->getDisplayName()).str();
    result.used    = true;
    pkmn->setHeldItem(nullptr);
    return result;
}

ItemUtils::HeldItemTickResult ItemUtils::consumeStatusBerry(Pkmn::PkmnPtr const& pkmn, BattleBattler* battler,
                                                            ConfusionStatus* confusion)
{
    HeldItemTickResult result;
    if (battler && (battler->itemsBlocked || battler->embargoTurns > 0))
        return result;
    if (battler && !battler->abilitySuppressed && battler->abilityId == "KLUTZ")
        return result;
    if (!pkmn || pkmn->isKO() || !pkmn->getHeldItem() || !pkmn->getHeldItem()->getDefinition())
        return result;

    auto        held   = pkmn->getHeldItem();
    std::string itemId = held->getDefinition()->getId();
    auto        consume = [&]() {
        if (battler)
        {
            battler->consumedItemId = itemId;
            if (itemId.find("BERRY") != std::string::npos)
                battler->consumedBerry = true;
            if (!battler->abilitySuppressed && battler->abilityId == "UNBURDEN")
                battler->unburdenActive = true;
            if (battler->healBlockTurns <= 0 && !battler->abilitySuppressed && battler->abilityId == "CHEEKPOUCH"
                && itemId.find("BERRY") != std::string::npos)
                pkmn->setHP(std::min(pkmn->getTotalHP(), pkmn->getHP() + std::max<size_t>(1, pkmn->getTotalHP() / 3)));
        }
        pkmn->setHeldItem(nullptr);
        result.used = true;
    };

    auto status = pkmn->getStatusCondition();
    if (curesStatus(itemId, status))
    {
        pkmn->setStatusCondition(Pkmn::StatusCondition::NONE);
        result.message =
            (boost::format(lc::translate("%1%'s %2% cured its status problem!")) % pkmn->getDisplayName()
             % held->getDefinition()->getName())
                .str();
        consume();
        return result;
    }
    if (curesConfusion(itemId) && confusion && confusion->active)
    {
        confusion->clear();
        result.message =
            (boost::format(lc::translate("%1%'s %2% snapped it out of confusion!")) % pkmn->getDisplayName()
             % held->getDefinition()->getName())
                .str();
        consume();
        return result;
    }
    return result;
}

void ItemUtils::noteHeldItemLoss(Pkmn::PkmnPtr const& pkmn, BattleBattler* battler, std::string const& itemId)
{
    (void)pkmn;
    if (!battler || itemId.empty())
        return;
    battler->consumedItemId = itemId;
    if (!battler->abilitySuppressed && battler->abilityId == "UNBURDEN")
        battler->unburdenActive = true;
}

ItemUtils::HeldItemTickResult ItemUtils::consumePinchBerry(Pkmn::PkmnPtr const& pkmn, BattleBattler* battler,
                                                           ConfusionStatus* confusion, SingleBattleSceneData& data,
                                                           bool ignorePinch)
{
    HeldItemTickResult result;
    if (!ignorePinch && battler && (battler->itemsBlocked || battler->embargoTurns > 0))
        return result;
    if (!ignorePinch && battler && !battler->abilitySuppressed && battler->abilityId == "KLUTZ")
        return result;
    if (!pkmn || pkmn->isKO() || !pkmn->getHeldItem() || !pkmn->getHeldItem()->getDefinition())
        return result;

    auto        held   = pkmn->getHeldItem();
    std::string itemId = held->getDefinition()->getId();
    bool        ripen  = battler && !battler->abilitySuppressed && battler->abilityId == "RIPEN";
    auto        consume = [&]() {
        if (battler)
        {
            battler->consumedItemId = itemId;
            if (itemId.find("BERRY") != std::string::npos)
                battler->consumedBerry = true;
            if (!ignorePinch && !battler->abilitySuppressed && battler->abilityId == "UNBURDEN")
                battler->unburdenActive = true;
            if (!ignorePinch && battler->healBlockTurns <= 0 && !battler->abilitySuppressed && battler->abilityId == "CHEEKPOUCH"
                && itemId.find("BERRY") != std::string::npos)
                pkmn->setHP(std::min(pkmn->getTotalHP(), pkmn->getHP() + std::max<size_t>(1, pkmn->getTotalHP() / 3)));
        }
        pkmn->setHeldItem(nullptr);
        result.used = true;
    };
    bool healBlocked = battler && battler->healBlockTurns > 0;
    auto healMsg = [&](size_t amount) {
        if (healBlocked)
        {
            consume();
            return true;
        }
        size_t maxHp = pkmn->getTotalHP();
        size_t hp    = pkmn->getHP();
        size_t neu   = std::min(maxHp, hp + amount);
        if (neu == hp)
            return false;
        pkmn->setHP(neu);
        result.message = (boost::format(lc::translate("%1% restored HP using its %2%!")) % pkmn->getDisplayName()
                          % held->getDefinition()->getName())
                             .str();
        return true;
    };

    bool pinch = ignorePinch || pkmn->getHP() * 2 <= pkmn->getTotalHP();
    if (pinch)
    {
        auto raise = [&](BattleBattler::StageStat stat) {
            if (!battler)
                return false;
            std::vector<std::string> lines;
            auto outcome = applyStatChange(data, *battler, pkmn, stat, ripen ? 2 : 1, lines, false);
            if (!outcome.changed)
                return false;
            for (auto const& line : lines)
            {
                if (!result.message.empty())
                    result.message += " ";
                result.message += line;
            }
            result.ejected = result.ejected || outcome.ejected;
            consume();
            return true;
        };
        if ((itemId == "FIGYBERRY" || itemId == "WIKIBERRY" || itemId == "MAGOBERRY" || itemId == "AGUAVBERRY"
             || itemId == "IAPAPABERRY")
            && healMsg(std::max<size_t>(1, pkmn->getTotalHP() / 3) * (ripen ? 2 : 1)))
        {
            auto nature = pkmn->getNature();
            bool dislikes = (itemId == "FIGYBERRY"
                             && (nature == Pkmn::Nature::BOLD || nature == Pkmn::Nature::TIMID
                                 || nature == Pkmn::Nature::MODEST || nature == Pkmn::Nature::CALM))
                         || (itemId == "WIKIBERRY"
                             && (nature == Pkmn::Nature::ADAMANT || nature == Pkmn::Nature::IMPISH
                                 || nature == Pkmn::Nature::JOLLY || nature == Pkmn::Nature::CAREFUL))
                         || (itemId == "MAGOBERRY"
                             && (nature == Pkmn::Nature::BRAVE || nature == Pkmn::Nature::RELAXED
                                 || nature == Pkmn::Nature::QUIET || nature == Pkmn::Nature::SASSY))
                         || (itemId == "AGUAVBERRY"
                             && (nature == Pkmn::Nature::NAUGHTY || nature == Pkmn::Nature::LAX
                                 || nature == Pkmn::Nature::NAIVE || nature == Pkmn::Nature::RASH))
                         || (itemId == "IAPAPABERRY"
                             && (nature == Pkmn::Nature::LONELY || nature == Pkmn::Nature::HASTY
                                 || nature == Pkmn::Nature::MILD || nature == Pkmn::Nature::GENTLE));
            if (dislikes && confusion && confusion->apply())
            {
                result.message += " ";
                result.message += (boost::format(lc::translate("%1% became confused!")) % pkmn->getDisplayName()).str();
            }
            consume();
            return result;
        }
        if (battler && itemId == "LIECHIBERRY" && raise(BattleBattler::StageStat::Attack))
            return result;
        if (battler && itemId == "GANLONBERRY" && raise(BattleBattler::StageStat::Defense))
            return result;
        if (battler && itemId == "SALACBERRY" && raise(BattleBattler::StageStat::Speed))
            return result;
        if (battler && itemId == "PETAYABERRY" && raise(BattleBattler::StageStat::SpecialAttack))
            return result;
        if (battler && itemId == "APICOTBERRY" && raise(BattleBattler::StageStat::SpecialDefense))
            return result;
        if (battler && itemId == "LANSATBERRY" && !battler->focusEnergy)
        {
            battler->focusEnergy = ripen ? 4 : 2;
            result.message = (boost::format(lc::translate("%1%'s Lansat Berry is getting pumped!")) % pkmn->getDisplayName()).str();
            consume();
            return result;
        }
        if (battler && itemId == "STARFBERRY")
        {
            BattleBattler::StageStat stats[] = {
                BattleBattler::StageStat::Attack, BattleBattler::StageStat::Defense,
                BattleBattler::StageStat::SpecialAttack, BattleBattler::StageStat::SpecialDefense,
                BattleBattler::StageStat::Speed};
            std::vector<std::string> lines;
            auto outcome = applyStatChange(data, *battler, pkmn, stats[Utils::randuint(0, 4)], ripen ? 4 : 2, lines, false);
            if (outcome.changed)
            {
                for (auto const& line : lines)
                {
                    if (!result.message.empty())
                        result.message += " ";
                    result.message += line;
                }
                result.ejected = outcome.ejected;
                consume();
                return result;
            }
        }
        if (battler && itemId == "CUSTAPBERRY")
        {
            battler->custapBerry = true;
            result.message = (boost::format(lc::translate("%1% will move first with its Custap Berry!")) % pkmn->getDisplayName()).str();
            consume();
            return result;
        }
        if (battler && itemId == "MICLEBERRY")
        {
            battler->micleBerry = true;
            result.message = (boost::format(lc::translate("%1%'s Micle Berry boosted its accuracy!")) % pkmn->getDisplayName()).str();
            consume();
            return result;
        }
    }

    if (ignorePinch || pkmn->getHP() * 2 <= pkmn->getTotalHP())
    {
        if (itemId == "ORANBERRY" && healMsg(ripen ? 20 : 10))
        {
            consume();
            return result;
        }
        if (itemId == "BERRYJUICE" && healMsg(20))
        {
            consume();
            return result;
        }
        if (itemId == "SITRUSBERRY" && healMsg(ripen ? 60 : 30))
        {
            consume();
            return result;
        }
    }
    return result;
}

ItemUtils::HeldItemTickResult ItemUtils::eatBerry(Pkmn::PkmnPtr const& eater, BattleBattler* battler, ConfusionStatus* confusion,
                                                  Item::ItemPtr const& berry, SingleBattleSceneData& data)
{
    HeldItemTickResult result;
    if (!eater || eater->isKO() || !berry || !berry->getDefinition())
        return result;
    std::string itemId = berry->getDefinition()->getId();
    std::string name   = berry->getDefinition()->getName();
    auto        saved  = eater->getHeldItem();
    std::string savedConsumed = battler ? battler->consumedItemId : "";
    bool        savedUnburden = battler && battler->unburdenActive;
    eater->setHeldItem(berry);
    result = consumePinchBerry(eater, battler, confusion, data, true);
    if (!result.used)
    {
        if (curesStatus(itemId, eater->getStatusCondition()))
        {
            eater->setStatusCondition(Pkmn::StatusCondition::NONE);
            result.message = (boost::format(lc::translate("%1%'s %2% cured its status problem!")) % eater->getDisplayName() % name).str();
            result.used = true;
        }
        else if (ItemUtils::curesConfusion(itemId) && confusion && confusion->active)
        {
            confusion->clear();
            result.message = (boost::format(lc::translate("%1%'s %2% snapped it out of confusion!")) % eater->getDisplayName() % name).str();
            result.used = true;
        }
        else if (itemId == "LEPPABERRY")
        {
            for (auto& move : eater->getMoves())
            {
                if (move && move->getDefinition() && move->getCurrentPP() == 0 && move->getDefinition()->getTotalPP() > 0)
                {
                    move->setCurrentPP(std::min<size_t>(10, move->getDefinition()->getTotalPP()));
                    result.message = (boost::format(lc::translate("%1% restored PP to its %2% using its Leppa Berry!"))
                                      % eater->getDisplayName() % move->getDefinition()->getName())
                                         .str();
                    result.used = true;
                    break;
                }
            }
        }
    }
    eater->setHeldItem(saved);
    if (battler)
    {
        battler->consumedItemId = savedConsumed;
        battler->unburdenActive = savedUnburden;
        if (battler->healBlockTurns <= 0 && itemId.find("BERRY") != std::string::npos && !battler->abilitySuppressed
            && battler->abilityId == "CHEEKPOUCH")
            eater->setHP(std::min(eater->getTotalHP(), eater->getHP() + std::max<size_t>(1, eater->getTotalHP() / 3)));
    }
    return result;
}

ItemUtils::HeldItemTickResult ItemUtils::tickHeldItem(Pkmn::PkmnPtr const& pkmn,
                                                      BattleBattler*       battler,
                                                      ConfusionStatus*     confusion,
                                                      SingleBattleSceneData& data)
{
    HeldItemTickResult result;
    if (battler && (battler->itemsBlocked || battler->embargoTurns > 0))
        return result;
    if (battler && !battler->abilitySuppressed && battler->abilityId == "KLUTZ")
        return result;
    if (!pkmn || pkmn->isKO() || !pkmn->getHeldItem() || !pkmn->getHeldItem()->getDefinition())
        return result;

    auto        held   = pkmn->getHeldItem();
    std::string itemId = held->getDefinition()->getId();

    auto consume = [&]() {
        if (battler)
        {
            battler->consumedItemId = itemId;
            if (!battler->abilitySuppressed && battler->abilityId == "UNBURDEN")
                battler->unburdenActive = true;
            if (battler->healBlockTurns <= 0 && !battler->abilitySuppressed && battler->abilityId == "CHEEKPOUCH"
                && itemId.find("BERRY") != std::string::npos)
                pkmn->setHP(std::min(pkmn->getTotalHP(), pkmn->getHP() + std::max<size_t>(1, pkmn->getTotalHP() / 3)));
        }
        pkmn->setHeldItem(nullptr);
        result.used = true;
    };

    auto healMsg = [&](size_t amount) {
        size_t maxHp = pkmn->getTotalHP();
        size_t hp    = pkmn->getHP();
        size_t neu   = std::min(maxHp, hp + amount);
        if (neu == hp)
            return false;
        pkmn->setHP(neu);
        result.message =
            (boost::format(lc::translate("%1% restored HP using its %2%!")) % pkmn->getDisplayName()
             % held->getDefinition()->getName())
                .str();
        return true;
    };

    // Leftovers / Black Sludge every turn
    if (itemId == "LEFTOVERS")
    {
        if (battler && battler->healBlockTurns > 0)
            return result;
        if (healMsg(std::max<size_t>(1, pkmn->getTotalHP() / 16)))
            result.used = true; // not consumed
        return result;
    }
    if (itemId == "BLACKSLUDGE")
    {
        if (isPoisonType(pkmn, battler))
        {
            if (battler && battler->healBlockTurns > 0)
                return result;
            if (healMsg(std::max<size_t>(1, pkmn->getTotalHP() / 16)))
                result.used = true;
        }
        else if (!(battler && !battler->abilitySuppressed && battler->abilityId == "MAGICGUARD"))
        {
            pkmn->decreaseHP(std::max<size_t>(1, pkmn->getTotalHP() / 8));
            result.message =
                (boost::format(lc::translate("%1% was hurt by %2%!")) % pkmn->getDisplayName()
                 % held->getDefinition()->getName())
                    .str();
            result.used    = true;
            result.fainted = pkmn->isKO();
        }
        return result;
    }

    // Pinch berries (≤25% HP, or ≤50% with Gluttony) and Oran / Sitrus at half.
    {
        auto pinch = consumePinchBerry(pkmn, battler, confusion, data);
        if (pinch.used)
            return pinch;
    }

    // Status berries
    auto status = pkmn->getStatusCondition();
    if (curesStatus(itemId, status))
    {
        pkmn->setStatusCondition(Pkmn::StatusCondition::NONE);
        result.message =
            (boost::format(lc::translate("%1%'s %2% cured its status problem!")) % pkmn->getDisplayName()
             % held->getDefinition()->getName())
                .str();
        consume();
        return result;
    }
    if (curesConfusion(itemId) && confusion && confusion->active)
    {
        confusion->clear();
        result.message =
            (boost::format(lc::translate("%1%'s %2% snapped it out of confusion!")) % pkmn->getDisplayName()
             % held->getDefinition()->getName())
                .str();
        consume();
        return result;
    }

    // Leppa Berry: restore PP of a depleted move
    if (itemId == "LEPPABERRY")
    {
        for (auto& move : pkmn->getMoves())
        {
            if (move && move->getDefinition() && move->getCurrentPP() == 0
                && move->getDefinition()->getTotalPP() > 0)
            {
                move->setCurrentPP(std::min<size_t>(10, move->getDefinition()->getTotalPP()));
                result.message =
                    (boost::format(lc::translate("%1% restored PP to its %2% using its Leppa Berry!"))
                     % pkmn->getDisplayName() % move->getDefinition()->getName())
                        .str();
                consume();
                return result;
            }
        }
    }

    if (itemId == "WHITEHERB")
        return consumeWhiteHerb(pkmn, battler);

    if (itemId == "FLAMEORB" && pkmn->getStatusCondition() == Pkmn::StatusCondition::NONE)
    {
        size_t toxic = battler ? battler->toxicCounter : 0;
        bool leafGuard = battler && !battler->abilitySuppressed && battler->abilityId == "LEAFGUARD"
                      && data.fieldWeather() == Map::Weather::HARSH_SUNLIGHT
                      && activeHeldId(pkmn, battler) != "UTILITYUMBRELLA";
        if (!leafGuard && !(battler && abilityBlocksStatus(pkmn, *battler, Pkmn::StatusCondition::BURN, false))
            && applyStatusCondition(pkmn, Pkmn::StatusCondition::BURN, toxic, false, battler))
        {
            if (battler)
                battler->toxicCounter = toxic;
            result.message = (boost::format(lc::translate("%1% was burned by its Flame Orb!")) % pkmn->getDisplayName()).str();
            result.used = true;
            return result;
        }
    }
    if (itemId == "TOXICORB" && pkmn->getStatusCondition() == Pkmn::StatusCondition::NONE)
    {
        size_t toxic = battler ? battler->toxicCounter : 0;
        bool leafGuard = battler && !battler->abilitySuppressed && battler->abilityId == "LEAFGUARD"
                      && data.fieldWeather() == Map::Weather::HARSH_SUNLIGHT
                      && activeHeldId(pkmn, battler) != "UTILITYUMBRELLA";
        if (!leafGuard
            && !(battler && abilityBlocksStatus(pkmn, *battler, Pkmn::StatusCondition::BADLY_POISON, false))
            && applyStatusCondition(pkmn, Pkmn::StatusCondition::BADLY_POISON, toxic, false, battler))
        {
            if (battler)
                battler->toxicCounter = toxic;
            result.message = (boost::format(lc::translate("%1% was badly poisoned by its Toxic Orb!")) % pkmn->getDisplayName()).str();
            result.used = true;
            return result;
        }
    }
    if (itemId == "STICKYBARB")
    {
        if (battler && !battler->abilitySuppressed && battler->abilityId == "MAGICGUARD")
            return result;
        pkmn->decreaseHP(std::max<size_t>(1, pkmn->getTotalHP() / 8));
        result.message = (boost::format(lc::translate("%1% was hurt by its Sticky Barb!")) % pkmn->getDisplayName()).str();
        result.used = true;
        result.fainted = pkmn->isKO();
        return result;
    }
    if (itemId == "MENTALHERB" && battler && battler->infatuated)
    {
        battler->infatuated = false;
        result.message = (boost::format(lc::translate("%1%'s Mental Herb cured its condition!")) % pkmn->getDisplayName()).str();
        consume();
        return result;
    }

    return result;
}

namespace
{
std::string heldId(Pkmn::PkmnPtr const& pkmn, BattleBattler const* battler = nullptr)
{
    return ItemUtils::activeHeldId(pkmn, battler);
}
} // namespace

void ItemUtils::consumeMentalHerb(Pkmn::PkmnPtr const& pkmn, BattleBattler* battler, std::vector<std::string>& messages)
{
    if (!pkmn || !battler)
        return;
    if (activeHeldId(pkmn, battler) != "MENTALHERB")
        return;
    if (!battler->infatuated)
        return;
    battler->infatuated = false;
    messages.push_back(
        (boost::format(lc::translate("%1%'s Mental Herb cured its condition!")) % pkmn->getDisplayName()).str());
    noteHeldItemLoss(pkmn, battler, "MENTALHERB");
    pkmn->setHeldItem(nullptr);
}

std::string ItemUtils::activeHeldId(Pkmn::PkmnPtr const& pkmn, BattleBattler const* battler)
{
    if (battler && (battler->itemsBlocked || battler->embargoTurns > 0))
        return {};
    if (battler && !battler->abilitySuppressed && battler->abilityId == "KLUTZ")
        return {};
    if (!pkmn || !pkmn->getHeldItem() || !pkmn->getHeldItem()->getDefinition())
        return {};
    return pkmn->getHeldItem()->getDefinition()->getId();
}

std::string ItemUtils::plateTypeForItem(std::string const& itemId)
{
    auto endsWith = [&](char const* suffix) {
        size_t n = std::char_traits<char>::length(suffix);
        return itemId.size() >= n && itemId.compare(itemId.size() - n, n, suffix) == 0;
    };
    if (!endsWith("PLATE") && !endsWith("DRIVE") && !endsWith("MEMORY"))
        return {};
    if (itemId.find("FLAME") != std::string::npos || itemId.find("BURN") != std::string::npos || itemId.find("FIRE") != std::string::npos)
        return "FIRE";
    if (itemId.find("SPLASH") != std::string::npos || itemId.find("DOUSE") != std::string::npos || itemId.find("WATER") != std::string::npos)
        return "WATER";
    if (itemId.find("ZAP") != std::string::npos || itemId.find("SHOCK") != std::string::npos || itemId.find("ELECTRIC") != std::string::npos)
        return "ELECTRIC";
    if (itemId.find("MEADOW") != std::string::npos || itemId.find("GRASS") != std::string::npos)
        return "GRASS";
    if (itemId.find("ICICLE") != std::string::npos || itemId.find("CHILL") != std::string::npos || itemId.find("ICE") != std::string::npos)
        return "ICE";
    if (itemId.find("FIST") != std::string::npos || itemId.find("FIGHTING") != std::string::npos)
        return "FIGHTING";
    if (itemId.find("TOXIC") != std::string::npos || itemId.find("POISON") != std::string::npos)
        return "POISON";
    if (itemId.find("EARTH") != std::string::npos || itemId.find("GROUND") != std::string::npos)
        return "GROUND";
    if (itemId.find("SKY") != std::string::npos || itemId.find("FLYING") != std::string::npos)
        return "FLYING";
    if (itemId.find("MIND") != std::string::npos || itemId.find("PSYCHIC") != std::string::npos)
        return "PSYCHIC";
    if (itemId.find("INSECT") != std::string::npos || itemId.find("BUG") != std::string::npos)
        return "BUG";
    if (itemId.find("STONE") != std::string::npos || itemId.find("ROCK") != std::string::npos)
        return "ROCK";
    if (itemId.find("SPOOKY") != std::string::npos || itemId.find("GHOST") != std::string::npos)
        return "GHOST";
    if (itemId.find("DRACO") != std::string::npos || itemId.find("DRAGON") != std::string::npos)
        return "DRAGON";
    if (itemId.find("DREAD") != std::string::npos || itemId.find("DARK") != std::string::npos)
        return "DARK";
    if (itemId.find("IRON") != std::string::npos || itemId.find("STEEL") != std::string::npos)
        return "STEEL";
    if (itemId.find("PIXIE") != std::string::npos || itemId.find("FAIRY") != std::string::npos)
        return "FAIRY";
    return {};
}

float ItemUtils::heldPowerMultiplier(Pkmn::PkmnPtr const& pkmn, MoveDef::MoveDefPtr const& move, BattleBattler const* battler)
{
    if (!pkmn || !move)
        return 1.f;
    std::string item = heldId(pkmn, battler);
    if (item == "LIFEORB")
        return 1.3f;
    if (item == "MUSCLEBAND" && move->getCategory() == MoveDef::PHYSICAL)
        return 1.1f;
    if (item == "WISEGLASSES" && move->getCategory() == MoveDef::SPECIAL)
        return 1.1f;
    return 1.f;
}

float ItemUtils::heldTypePowerMultiplier(Pkmn::PkmnPtr const& pkmn, std::string const& type, BattleBattler const* battler)
{
    if (!pkmn || type.empty())
        return 1.f;
    std::string item = heldId(pkmn, battler);
    auto matches    = [&](char const* id, char const* itemType) { return item == id && type == itemType; };
    if (matches("SEAINCENSE", "WATER"))
        return 1.05f;
    if (matches("CHARCOAL", "FIRE") || matches("MYSTICWATER", "WATER") || matches("MIRACLESEED", "GRASS")
        || matches("MAGNET", "ELECTRIC") || matches("NEVERMELTICE", "ICE") || matches("BLACKBELT", "FIGHTING")
        || matches("POISONBARB", "POISON") || matches("SOFTSAND", "GROUND") || matches("SHARPBEAK", "FLYING")
        || matches("TWISTEDSPOON", "PSYCHIC") || matches("SILVERPOWDER", "BUG") || matches("HARDSTONE", "ROCK")
        || matches("SPELLTAG", "GHOST") || matches("DRAGONFANG", "DRAGON") || matches("BLACKGLASSES", "DARK")
        || matches("METALCOAT", "STEEL") || matches("SILKSCARF", "NORMAL"))
        return 1.1f;
    return 1.f;
}

float ItemUtils::heldSpeedMultiplier(Pkmn::PkmnPtr const& pkmn, BattleBattler const* battler)
{
    std::string item = heldId(pkmn, battler);
    std::string species = pkmn && pkmn->getDefinition() ? pkmn->getDefinition()->getId() : "";
    float mult = 1.f;
    if (item == "CHOICESCARF")
        mult *= 1.5f;
    if (item == "QUICKPOWDER" && species == "DITTO")
        mult *= 2.f;
    if (item == "IRONBALL")
        mult *= 0.5f;
    if (item == "MACHOBRACE" || item == "POWERWEIGHT" || item == "POWERBRACER" || item == "POWERBELT"
        || item == "POWERLENS" || item == "POWERBAND" || item == "POWERANKLET")
        mult *= 0.5f;
    return mult;
}

float ItemUtils::heldAccuracyMultiplier(Pkmn::PkmnPtr const& pkmn, bool targetAlreadyMoved, BattleBattler const* battler)
{
    std::string item = heldId(pkmn, battler);
    if (item == "WIDELENS")
        return 1.1f;
    if (item == "ZOOMLENS" && targetAlreadyMoved)
        return 1.2f;
    return 1.f;
}

int ItemUtils::screenTurnsForHolder(Pkmn::PkmnPtr const& user, BattleBattler const* battler)
{
    return heldId(user, battler) == "LIGHTCLAY" ? 8 : 5;
}

bool ItemUtils::airBalloonBlocksGround(Pkmn::PkmnPtr const&      target,
                                       std::string const&        moveType,
                                       std::vector<std::string>& messages,
                                       BattleBattler const*      battler)
{
    if (!target || moveType != "GROUND" || heldId(target, battler) != "AIRBALLOON")
        return false;
    (void)messages;
    return true;
}

void ItemUtils::applyDamagingHitItems(Pkmn::PkmnPtr const&      target,
                                      BattleBattler&           battler,
                                      float                    effectiveness,
                                      std::string const&       moveType,
                                      bool&                    switchHolder,
                                      bool&                    forceAttackerOut,
                                      std::vector<std::string>& messages,
                                      SingleBattleSceneData&   data)
{
    std::string item = heldId(target, &battler);
    if (item.empty() || !target)
        return;
    auto consume = [&]() {
        if (!battler.abilitySuppressed && battler.abilityId == "UNBURDEN")
            battler.unburdenActive = true;
        battler.consumedItemId = item;
        target->setHeldItem(nullptr);
    };
    if (item == "WEAKNESSPOLICY" && effectiveness > 1.f)
    {
        auto attack = applyStatChange(data, battler, target, BattleBattler::StageStat::Attack, 2, messages, false);
        auto special = applyStatChange(data, battler, target, BattleBattler::StageStat::SpecialAttack, 2, messages, false);
        if (attack.ejected || special.ejected)
            switchHolder = true;
        consume();
        return;
    }
    if (item == "EJECTBUTTON")
    {
        messages.push_back(
            (boost::format(lc::translate("%1% is switched out by its Eject Button!")) % target->getDisplayName()).str());
        consume();
        switchHolder = true;
        return;
    }
    if (item == "REDCARD")
    {
        messages.push_back(
            (boost::format(lc::translate("%1%'s Red Card forced the attacker out!")) % target->getDisplayName()).str());
        consume();
        forceAttackerOut = true;
        return;
    }
    if (item == "AIRBALLOON")
    {
        messages.push_back(
            (boost::format(lc::translate("%1%'s Air Balloon popped!")) % target->getDisplayName()).str());
        consume();
        return;
    }
    auto boost = [&](BattleBattler::StageStat stat) {
        auto outcome = applyStatChange(data, battler, target, stat, 1, messages, false);
        if (outcome.ejected)
            switchHolder = true;
        consume();
    };
    if (item == "ABSORBBULB" && moveType == "WATER")
        boost(BattleBattler::StageStat::SpecialAttack);
    else if (item == "CELLBATTERY" && moveType == "ELECTRIC")
        boost(BattleBattler::StageStat::Attack);
    else if (item == "SNOWBALL" && moveType == "ICE")
        boost(BattleBattler::StageStat::Attack);
    else if (item == "LUMINOUSMOSS" && moveType == "WATER")
        boost(BattleBattler::StageStat::SpecialDefense);
}

int ItemUtils::weatherTurnsForHolder(Pkmn::PkmnPtr const& user, Map::Weather weather, BattleBattler const* battler)
{
    std::string item = heldId(user, battler);
    bool        rock = (weather == Map::Weather::RAIN && item == "DAMPROCK")
              || (weather == Map::Weather::HARSH_SUNLIGHT && item == "HEATROCK")
              || (weather == Map::Weather::SANDSTORM && item == "SMOOTHROCK")
              || (weather == Map::Weather::HAIL && item == "ICYROCK");
    return rock ? 8 : 5;
}

size_t ItemUtils::reduceIncomingDamage(Pkmn::PkmnPtr const&      target,
                                      std::string const&        moveType,
                                      float                     effectiveness,
                                      size_t                    damage,
                                      std::vector<std::string>& messages,
                                      BattleBattler*            battler)
{
    if (!target || damage == 0)
        return damage;
    std::string item = heldId(target, battler);
    auto consume = [&]() {
        target->setHeldItem(nullptr);
        if (!battler)
            return;
        if (!battler->abilitySuppressed && battler->abilityId == "UNBURDEN")
            battler->unburdenActive = true;
        battler->consumedItemId = item;
    };
    if (item == "FOCUSBAND" && damage >= target->getHP() && target->getHP() > 0 && Utils::randuint(1, 100) <= 10)
    {
        messages.push_back(
            (boost::format(lc::translate("%1% hung on using its Focus Band!")) % target->getDisplayName()).str());
        return target->getHP() - 1;
    }
    if (item == "FOCUSSASH" && target->getHP() == target->getTotalHP() && damage >= target->getHP())
    {
        consume();
        messages.push_back(
            (boost::format(lc::translate("%1% hung on using its Focus Sash!")) % target->getDisplayName()).str());
        return target->getHP() - 1;
    }
    if (item == "CHILANBERRY" && moveType == "NORMAL")
    {
        consume();
        messages.push_back(
            (boost::format(lc::translate("%1%'s berry weakened the damage!")) % target->getDisplayName()).str());
        bool ripen = battler && !battler->abilitySuppressed && battler->abilityId == "RIPEN";
        return std::max<size_t>(1, damage / (ripen ? 4 : 2));
    }
    auto resist = [&](char const* id, char const* type) { return item == id && moveType == type; };
    if (effectiveness > 1.f
        && (resist("OCCABERRY", "FIRE") || resist("PASSHOBERRY", "WATER") || resist("WACANBERRY", "ELECTRIC")
            || resist("RINDOBERRY", "GRASS") || resist("YACHEBERRY", "ICE") || resist("CHOPLEBERRY", "FIGHTING")
            || resist("KEBIABERRY", "POISON") || resist("SHUCABERRY", "GROUND") || resist("COBABERRY", "FLYING")
            || resist("PAYAPABERRY", "PSYCHIC") || resist("TANGABERRY", "BUG") || resist("CHARTIBERRY", "ROCK")
            || resist("KASIBBERRY", "GHOST") || resist("HABANBERRY", "DRAGON") || resist("COLBURBERRY", "DARK")
            || resist("BABIRIBERRY", "STEEL") || resist("ROSELIBERRY", "FAIRY")))
    {
        consume();
        messages.push_back(
            (boost::format(lc::translate("%1%'s berry weakened the damage!")) % target->getDisplayName()).str());
        bool ripen = battler && !battler->abilitySuppressed && battler->abilityId == "RIPEN";
        return std::max<size_t>(1, damage / (ripen ? 4 : 2));
    }
    return damage;
}

bool ItemUtils::applyContactItemEffects(Pkmn::PkmnPtr const&      attacker,
                                        Pkmn::PkmnPtr const&      defender,
                                        bool                      contact,
                                        bool                      attackerDealtDamage,
                                        size_t                    damageDealt,
                                        std::vector<std::string>& messages,
                                        BattleBattler const*      attackerBattler,
                                        BattleBattler const*      defenderBattler,
                                        bool                      suppressLifeOrb,
                                        bool                      skipPostMoveItems)
{
    bool fainted = false;
    auto magicGuard = [](BattleBattler const* battler) {
        return battler && !battler->abilitySuppressed && battler->abilityId == "MAGICGUARD";
    };
    // Shell Bell / Life Orb are once per move; Rocky Helmet / Sticky Barb are per contact hit.
    if (!skipPostMoveItems)
    {
        if (attacker && attackerDealtDamage && heldId(attacker, attackerBattler) == "SHELLBELL" && damageDealt > 0
            && !attacker->isKO() && !(attackerBattler && attackerBattler->healBlockTurns > 0))
        {
            size_t heal = std::max<size_t>(1, damageDealt / 8);
            attacker->setHP(std::min(attacker->getTotalHP(), attacker->getHP() + heal));
            messages.push_back(
                (boost::format(lc::translate("%1% restored HP with its Shell Bell!")) % attacker->getDisplayName()).str());
        }
        if (attacker && attackerDealtDamage && heldId(attacker, attackerBattler) == "LIFEORB" && !attacker->isKO()
            && !suppressLifeOrb && !magicGuard(attackerBattler))
        {
            attacker->decreaseHP(std::max<size_t>(1, attacker->getTotalHP() / 10));
            messages.push_back(
                (boost::format(lc::translate("%1% was hurt by its Life Orb!")) % attacker->getDisplayName()).str());
            fainted = attacker->isKO();
        }
    }
    if (contact && attacker && defender && !attacker->isKO() && heldId(defender, defenderBattler) == "ROCKYHELMET"
        && !magicGuard(attackerBattler))
    {
        attacker->decreaseHP(std::max<size_t>(1, attacker->getTotalHP() / 6));
        messages.push_back(
            (boost::format(lc::translate("%1% was hurt by the Rocky Helmet!")) % attacker->getDisplayName()).str());
        fainted = fainted || attacker->isKO();
    }
    if (contact && attacker && defender && !attacker->isKO() && heldId(defender, defenderBattler) == "STICKYBARB"
        && heldId(attacker, attackerBattler).empty()
        && !(defenderBattler && !defenderBattler->abilitySuppressed && defenderBattler->abilityId == "STICKYHOLD"))
    {
        BattleBattler* defBattler = const_cast<BattleBattler*>(defenderBattler);
        BattleBattler* atkBattler = const_cast<BattleBattler*>(attackerBattler);
        noteHeldItemLoss(defender, defBattler, "STICKYBARB");
        attacker->setHeldItem(defender->getHeldItem());
        defender->setHeldItem(nullptr);
        if (atkBattler)
            atkBattler->unburdenActive = false;
        messages.push_back(
            (boost::format(lc::translate("%1% picked up the Sticky Barb!")) % attacker->getDisplayName()).str());
    }
    return fainted;
}

void ItemUtils::addPkmnToPlayer(Pkmn::PkmnPtr const& pkmn)
{
    int   partyFirstAvailableIndex = -1;
    auto& player                   = Game::instance()->data.player;
    auto& storage                  = Game::instance()->data.storage;
    for (size_t i = 0; i < player.pkmns.size(); ++i)
    {
        if (!player.pkmns[i])
        {
            partyFirstAvailableIndex = static_cast<int>(i);
            break;
        }
    }

    if (partyFirstAvailableIndex >= 0)
        player.pkmns[partyFirstAvailableIndex] = pkmn;
    else
        storage.addPkmn(pkmn);
}
