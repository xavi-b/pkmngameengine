#include "itemutils.h"

#include "game.h"
#include "utils.h"

#include <algorithm>
#include <cctype>
#include <limits>

namespace
{

size_t healingAmountForItem(std::string const& itemId)
{
    if (itemId == "POTION")
        return 20;
    if (itemId == "SUPERPOTION")
        return 50;
    if (itemId == "HYPERPOTION")
        return 200;
    if (itemId == "FRESHWATER")
        return 30;
    if (itemId == "SODAPOP")
        return 50;
    if (itemId == "LEMONADE")
        return 70;
    if (itemId == "MOOMOOMILK")
        return 100;
    if (itemId == "BERRYJUICE")
        return 20;
    if (itemId == "SWEETHEART")
        return 20;
    if (itemId == "ORANBERRY")
        return 10;
    if (itemId == "SITRUSBERRY")
        return 30;
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
        || itemId == "BIGMALASADA")
        return true;
    if ((itemId == "AWAKENING" || itemId == "CHESTOBERRY") && status == Pkmn::StatusCondition::SLEEP)
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
} // namespace

bool ItemUtils::consumePlayerItem(Player& player, Item::ItemPtr const& item)
{
    if (!item || !item->getDefinition())
        return false;
    auto& pocketItems = player.items[item->getDefinition()->getPocket()];
    auto  it          = std::find(pocketItems.begin(), pocketItems.end(), item);
    if (it == pocketItems.end())
        return false;
    (*it)->decreaseCount(1);
    if ((*it)->getCount() == 0)
        pocketItems.erase(it);
    return true;
}

bool ItemUtils::useItemOnPkmn(Item::ItemPtr const& item, Pkmn::PkmnPtr const& target, bool inBattle)
{
    if (!item || !item->getDefinition() || !target)
        return false;

    std::string itemId = item->getDefinition()->getId();
    bool        used   = false;

    size_t healing = healingAmountForItem(itemId);
    if (healing > 0 && !target->isKO())
    {
        size_t maxHp = target->getTotalHP();
        size_t newHp = itemId == "MAXPOTION" || itemId == "FULLRESTORE" || itemId == "MAXHONEY"
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

    return used;
}

bool ItemUtils::useCaptureBall(Item::ItemPtr const& ball, Pkmn::PkmnPtr const& foePkmn)
{
    if (!ball || !ball->getDefinition() || !foePkmn || foePkmn->isKO())
        return false;

    std::string itemId    = ball->getDefinition()->getId();
    float       ballBonus = 1.0f;
    if (itemId == "GREATBALL")
        ballBonus = 1.5f;
    else if (itemId == "ULTRABALL")
        ballBonus = 2.0f;
    else if (itemId == "MASTERBALL")
        return true;

    size_t maxHp     = std::max<size_t>(1, foePkmn->getTotalHP());
    size_t hp        = foePkmn->getHP();
    size_t catchRate = foePkmn->getDefinition()->getCatchRate();
    float  a         = ((3.0f * maxHp - 2.0f * hp) * catchRate * ballBonus) / (3.0f * maxHp);
    float  chance    = std::clamp(a / 255.0f, 0.0f, 1.0f);
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
        used = true; // TODO: implement special direct effects
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

    if (used)
        consumePlayerItem(player, item);
    return used;
}

bool ItemUtils::allowFaintedTarget(Item::ItemPtr const& item)
{
    if (!item || !item->getDefinition())
        return false;
    return item->getDefinition()->getId() == "REVIVE" || item->getDefinition()->getId() == "MAXREVIVE"
        || item->getDefinition()->getId() == "REVIVALHERB";
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
            partyFirstAvailableIndex = i;
            break;
        }
    }

    if (partyFirstAvailableIndex >= 0)
        player.pkmns[partyFirstAvailableIndex] = pkmn;
    else
        storage.addPkmn(pkmn);
}