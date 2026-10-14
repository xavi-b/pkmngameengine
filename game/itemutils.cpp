#include "itemutils.h"

#include "game.h"
#include "scenes/battle/battlebattler.h"
#include "scenes/battle/confusionstatus.h"
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
    if ((itemId == "AWAKENING" || itemId == "CHESTOBERRY" || itemId == "POKEFLUTE")
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
    // Gen III base X items raise by 1 (modern text says "sharply").
    return 1;
}

bool applyXStat(std::string const& itemId, BattleBattler& battler)
{
    using SS = BattleBattler::StageStat;
    int delta = xItemStageDelta(itemId);

    if (itemId.rfind("XATTACK", 0) == 0)
        return battler.changeStage(SS::Attack, delta);
    if (itemId.rfind("XDEFENSE", 0) == 0 || itemId.rfind("XDEFEND", 0) == 0)
        return battler.changeStage(SS::Defense, delta);
    if (itemId.rfind("XSPATK", 0) == 0 || itemId.rfind("XSPECIAL", 0) == 0)
        return battler.changeStage(SS::SpecialAttack, delta);
    if (itemId.rfind("XSPDEF", 0) == 0)
        return battler.changeStage(SS::SpecialDefense, delta);
    if (itemId.rfind("XSPEED", 0) == 0)
        return battler.changeStage(SS::Speed, delta);
    if (itemId.rfind("XACCURACY", 0) == 0)
        return battler.changeStage(SS::Accuracy, delta);
    if (itemId == "MAXMUSHROOMS")
    {
        bool changed = false;
        changed |= battler.changeStage(SS::Attack, 1);
        changed |= battler.changeStage(SS::Defense, 1);
        changed |= battler.changeStage(SS::SpecialAttack, 1);
        changed |= battler.changeStage(SS::SpecialDefense, 1);
        changed |= battler.changeStage(SS::Speed, 1);
        return changed;
    }
    if (itemId.rfind("DIREHIT", 0) == 0)
    {
        if (battler.focusEnergy)
            return false;
        battler.focusEnergy = true;
        return true;
    }
    return false;
}

float ballBonusForId(std::string const& itemId)
{
    if (itemId == "MASTERBALL")
        return 255.0f;
    if (itemId == "ULTRABALL" || itemId == "SPORTBALL")
        return 2.0f;
    if (itemId == "GREATBALL" || itemId == "SAFARIBALL")
        return 1.5f;
    if (itemId == "NETBALL" || itemId == "DIVEBALL" || itemId == "NESTBALL" || itemId == "REPEATBALL"
        || itemId == "TIMERBALL" || itemId == "LUXURYBALL" || itemId == "PREMIERBALL" || itemId == "DUSKBALL"
        || itemId == "HEALBALL" || itemId == "QUICKBALL" || itemId == "CHERISHBALL" || itemId == "FASTBALL"
        || itemId == "LEVELBALL" || itemId == "LUREBALL" || itemId == "HEAVYBALL" || itemId == "LOVEBALL"
        || itemId == "FRIENDBALL" || itemId == "MOONBALL" || itemId == "DREAMBALL" || itemId == "BEASTBALL")
        return 1.5f; // Gen III best-effort: specialty balls approximate Great Ball
    return 1.0f;     // Poké Ball and unknown
}

bool isPoisonType(Pkmn::PkmnPtr const& pkmn)
{
    if (!pkmn || !pkmn->getDefinition())
        return false;
    for (auto const& t : pkmn->getDefinition()->getTypes())
    {
        if (t == "POISON" || t == "Poison")
            return true;
    }
    return false;
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
                                 ConfusionStatus*     confusion)
{
    if (!item || !item->getDefinition() || !target)
        return false;

    std::string itemId = item->getDefinition()->getId();
    bool        used   = false;

    if (applyXStat(itemId, battler))
        used = true;

    if (curesConfusion(itemId) && confusion && confusion->active)
    {
        confusion->clear();
        used = true;
    }

    // Red Flute cures infatuation — not tracked yet; treat as no-op unless confusion/status path
    if (itemId == "REDFLUTE")
        used = used || false;

    return used;
}

bool ItemUtils::useBattleItemDirect(Item::ItemPtr const& item,
                                    Pkmn::PkmnPtr const&  playerPkmn,
                                    BattleBattler&       playerBattler,
                                    bool&                endsBattle,
                                    std::string&         message)
{
    endsBattle = false;
    message.clear();
    if (!item || !item->getDefinition())
        return false;

    std::string itemId = item->getDefinition()->getId();

    if (itemId == "GUARDSPEC")
    {
        if (playerBattler.mist)
            return false;
        playerBattler.mist = true;
        message            = lc::translate("Your team became shrouded in mist!");
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

bool ItemUtils::useCaptureBall(Item::ItemPtr const& ball, Pkmn::PkmnPtr const& foePkmn)
{
    if (!ball || !ball->getDefinition() || !foePkmn || foePkmn->isKO())
        return false;

    auto def = ball->getDefinition();
    if (!def->hasFlag(ItemDef::Flag::PokeBall) && def->getBattleUse() != ItemDef::BattleUse::OnFoe)
        return false;

    std::string itemId = def->getId();
    if (itemId == "MASTERBALL")
        return true;

    float ballBonus = ballBonusForId(itemId);

    size_t maxHp     = std::max<size_t>(1, foePkmn->getTotalHP());
    size_t hp        = foePkmn->getHP();
    size_t catchRate = foePkmn->getDefinition()->getCatchRate();
    float  a         = ((3.0f * maxHp - 2.0f * hp) * catchRate * ballBonus) / (3.0f * maxHp);

    // Status modifiers (Gen III)
    switch (foePkmn->getStatusCondition())
    {
    case Pkmn::StatusCondition::SLEEP:
    case Pkmn::StatusCondition::FREEZE:
        a *= 2.0f;
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
    return Utils::randuint(0, 10000) <= static_cast<size_t>(chance * 10000.0f);
}

bool ItemUtils::useItemInField(Player& player, Item::ItemPtr const& item)
{
    if (!item || !item->getDefinition())
        return false;

    bool used = false;
    switch (item->getDefinition()->getFieldUse())
    {
    case ItemDef::FieldUse::Direct:
        used = true; // Repels / flutes / honey — field map effects TBD
        break;
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

ItemUtils::HeldItemTickResult ItemUtils::tickHeldItem(Pkmn::PkmnPtr const& pkmn,
                                                      BattleBattler*       battler,
                                                      ConfusionStatus*     confusion)
{
    HeldItemTickResult result;
    if (!pkmn || pkmn->isKO() || !pkmn->getHeldItem() || !pkmn->getHeldItem()->getDefinition())
        return result;

    auto        held   = pkmn->getHeldItem();
    std::string itemId = held->getDefinition()->getId();

    auto consume = [&]() {
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
        if (healMsg(std::max<size_t>(1, pkmn->getTotalHP() / 16)))
            result.used = true; // not consumed
        return result;
    }
    if (itemId == "BLACKSLUDGE")
    {
        if (isPoisonType(pkmn))
        {
            if (healMsg(std::max<size_t>(1, pkmn->getTotalHP() / 16)))
                result.used = true;
        }
        else
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

    // Pinch berries (≤50% HP)
    if (pkmn->getHP() * 2 <= pkmn->getTotalHP())
    {
        if (itemId == "ORANBERRY" && healMsg(10))
        {
            consume();
            return result;
        }
        if (itemId == "SITRUSBERRY")
        {
            // Gen III: restore 30 HP; later gens 25% — use Gen III
            if (healMsg(30))
            {
                consume();
                return result;
            }
        }
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

    (void)battler;
    return result;
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
