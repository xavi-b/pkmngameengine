#include "trainerscenedata.h"

#include "game.h"
#include "itemutils.h"
#include "statuscondition.h"

#include <algorithm>

namespace
{
bool moveCanHitPlayer(Pkmn::PkmnPtr const& player, BattleBattler const& playerBattler, Move::MovePtr const& move)
{
    if (!move || !move->getDefinition() || move->getCurrentPP() == 0 || !player)
        return false;
    auto def = move->getDefinition();
    if (def->getCategory() == MoveDef::STATUS || def->getPower() == 0)
        return false;
    float effectiveness = 1.f;
    for (auto const& typeId : activeTypes(player, &playerBattler))
    {
        auto type = Game::instance()->data.typeFor(typeId);
        if (!type)
            continue;
        auto const& immunities = type->getImmunities();
        if (std::find(immunities.begin(), immunities.end(), def->getType()) != immunities.end())
            effectiveness = 0.f;
    }
    return effectiveness > 0.f;
}

bool heldItemIsBetterNow(Pkmn::PkmnPtr const& pkmn)
{
    if (!pkmn || !pkmn->getHeldItem() || !pkmn->getHeldItem()->getDefinition())
        return false;
    std::string item = pkmn->getHeldItem()->getDefinition()->getId();
    auto status = pkmn->getStatusCondition();
    bool cures = item == "LUMBERRY"
              || (item == "PECHABERRY" && (status == Pkmn::StatusCondition::POISON || status == Pkmn::StatusCondition::BADLY_POISON))
              || (item == "RAWSTBERRY" && status == Pkmn::StatusCondition::BURN)
              || (item == "CHESTOBERRY" && status == Pkmn::StatusCondition::SLEEP)
              || (item == "CHERIBERRY" && status == Pkmn::StatusCondition::PARALYSIS)
              || (item == "ASPEARBERRY" && status == Pkmn::StatusCondition::FREEZE);
    if (cures)
        return true;
    bool halfHp = item == "ORANBERRY" || item == "SITRUSBERRY";
    if (halfHp)
        return pkmn->getHP() * 2 <= pkmn->getTotalHP();
    bool figy = item == "FIGYBERRY" || item == "WIKIBERRY" || item == "MAGOBERRY" || item == "AGUAVBERRY"
             || item == "IAPAPABERRY";
    if (!figy)
        return false;
    bool gluttony = pkmn->getAbilityId() == "GLUTTONY";
    return gluttony ? pkmn->getHP() * 2 <= pkmn->getTotalHP() : pkmn->getHP() * 4 <= pkmn->getTotalHP();
}
} // namespace

void TrainerSceneData::chooseOpponentAction()
{
    opponentAction   = BattleActions::Type::MOVES;
    nextOpponentPkmn = nullptr;

    bool playerUnnerve = !playerBattler.abilitySuppressed
                      && (playerBattler.abilityId == "UNNERVE" || playerBattler.abilityId == "ASONECHILLINGNEIGH"
                          || playerBattler.abilityId == "ASONEGRIMNEIGH");
    if (!playerUnnerve && heldItemIsBetterNow(opponentPkmn))
    {
        opponentAction = BattleActions::Type::BAG;
        return;
    }

    bool hasHittingMove = false;
    if (opponentPkmn)
    {
        for (auto const& move : opponentPkmn->getMoves())
        {
            if (moveCanHitPlayer(playerPkmn, playerBattler, move))
                hasHittingMove = true;
        }
    }
    bool shedShell = ItemUtils::activeHeldId(opponentPkmn, &opponentBattler) == "SHEDSHELL";
    if (!hasHittingMove && opponentTrainer && opponentPkmn && !fairyLock && (!opponentBattler.trapped || shedShell))
    {
        for (auto const& mon : opponentTrainer->pkmns)
        {
            if (!mon || mon->isKO() || mon == opponentPkmn)
                continue;
            nextOpponentPkmn           = mon;
            opponentAction             = BattleActions::Type::PKMNS;
            opponentVoluntarySwitch    = true;
            return;
        }
    }

    chooseOpponentMove();
}