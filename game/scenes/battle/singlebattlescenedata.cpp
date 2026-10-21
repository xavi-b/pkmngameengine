#include "singlebattlescenedata.h"

#include "game.h"
#include "itemutils.h"
#include "moveeffects.h"
#include "statuscondition.h"
#include "utils.h"

#include <algorithm>

bool opponentOutspeeds(SingleBattleSceneData const& data)
{
    size_t ps = battleSpeed(data.playerPkmn, &data.playerBattler, data.fieldWeather(), false);
    size_t os = battleSpeed(data.opponentPkmn, &data.opponentBattler, data.fieldWeather(), false);
    return os > ps;
}

std::string battleStateToString(BattleState state)
{
    switch (state)
    {
    case START:
        return "START";
    case WEATHER:
        return "WEATHER";
    case ACTIONS:
        return "ACTIONS";
    case MOVES:
        return "MOVES";
    case BAG:
        return "BAG";
    case PKMNS:
        return "PKMNS";
    case PLAYER_MOVES:
        return "PLAYER_MOVES";
    case PLAYER_ITEMS:
        return "PLAYER_ITEMS";
    case PLAYER_PKMNS:
        return "PLAYER_PKMNS";
    case PLAYER_RUN:
        return "PLAYER_RUN";
    case OPPONENT_PKMNS:
        return "OPPONENT_PKMNS";
    case OPPONENT_ITEMS:
        return "OPPONENT_ITEMS";
    case OPPONENT_MOVES:
        return "OPPONENT_MOVES";
    case OPPONENT_RUN:
        return "OPPONENT_RUN";
    case EXPERIENCE:
        return "EXPERIENCE";
    case END:
        return "END";
    }
    return "???";
}

namespace
{
float effectivenessAgainst(Pkmn::PkmnPtr const& pkmn, BattleBattler const& battler, std::string const& attackType)
{
    float effectiveness = 1.f;
    for (auto const& defenderTypeId : activeTypes(pkmn, &battler))
    {
        auto defenderType = Game::instance()->data.typeFor(defenderTypeId);
        if (!defenderType)
            continue;
        auto const& immunities = defenderType->getImmunities();
        if (std::find(immunities.begin(), immunities.end(), attackType) != immunities.end())
            effectiveness *= 0.f;
        auto const& weaknesses = defenderType->getWeaknesses();
        if (std::find(weaknesses.begin(), weaknesses.end(), attackType) != weaknesses.end())
            effectiveness *= 2.f;
        auto const& resistances = defenderType->getResistances();
        if (std::find(resistances.begin(), resistances.end(), attackType) != resistances.end())
            effectiveness *= 0.5f;
    }
    return effectiveness;
}

bool stagesFlat(BattleBattler const& battler)
{
    return std::all_of(battler.stages.begin(), battler.stages.end(), [](int stage) { return stage == 0; });
}

bool inflictsMajorStatus(MoveDef::FunctionCode code)
{
    using FC = MoveDef::FunctionCode;
    switch (code)
    {
    case FC::BadPoisonTarget:
    case FC::BurnFlinchTarget:
    case FC::BurnTarget:
    case FC::BurnTargetIfTargetStatsRaisedThisTurn:
    case FC::FreezeFlinchTarget:
    case FC::FreezeTarget:
    case FC::FreezeTargetAlwaysHitsInHail:
    case FC::FreezeTargetSuperEffectiveAgainstWater:
    case FC::ParalyzeBurnOrFreezeTarget:
    case FC::ParalyzeFlinchTarget:
    case FC::ParalyzeTarget:
    case FC::ParalyzeTargetAlwaysHitsInRainHitsTargetInSky:
    case FC::ParalyzeTargetIfNotTypeImmune:
    case FC::PoisonTarget:
    case FC::PoisonTargetLowerTargetSpeed1:
    case FC::SleepTarget:
    case FC::SleepTargetChangeUserMeloettaForm:
    case FC::SleepTargetIfUserDarkrai:
    case FC::SleepTargetNextTurn:
        return true;
    default:
        return false;
    }
}

bool statusBlockedByType(MoveDef::FunctionCode code, Pkmn::PkmnPtr const& target, BattleBattler const& battler)
{
    using FC = MoveDef::FunctionCode;
    switch (code)
    {
    case FC::BurnFlinchTarget:
    case FC::BurnTarget:
    case FC::BurnTargetIfTargetStatsRaisedThisTurn:
        return hasType(target, "FIRE", &battler);
    case FC::PoisonTarget:
    case FC::PoisonTargetLowerTargetSpeed1:
    case FC::BadPoisonTarget:
        return hasType(target, "POISON", &battler) || hasType(target, "STEEL", &battler);
    case FC::ParalyzeFlinchTarget:
    case FC::ParalyzeTarget:
    case FC::ParalyzeTargetAlwaysHitsInRainHitsTargetInSky:
    case FC::ParalyzeTargetIfNotTypeImmune:
    case FC::ParalyzeBurnOrFreezeTarget:
        return hasType(target, "ELECTRIC", &battler);
    case FC::StartLeechSeedTarget:
        return hasType(target, "GRASS", &battler);
    default:
        return false;
    }
}

float statusChoiceScore(SingleBattleSceneData const& data, MoveDef::MoveDefPtr const& def)
{
    using FC = MoveDef::FunctionCode;
    auto code = def->getFunctionCode();
    if (code == FC::RaiseAlliesAtkDef1 || code == FC::RaisePlusMinusUserAndAlliesAtkSpAtk1
        || code == FC::RaisePlusMinusUserAndAlliesDefSpDef1 || code == FC::DoesNothingFailsIfNoAlly
        || code == FC::UserSwapsPositionsWithAlly)
        return 0.f;

    if (data.playerPkmn && statusBlockedByType(code, data.playerPkmn, data.playerBattler))
        return 0.f;
    if (inflictsMajorStatus(code) && data.playerPkmn
        && data.playerPkmn->getStatusCondition() != Pkmn::StatusCondition::NONE)
        return 0.f;

    if (code == FC::ConfuseTarget || code == FC::ConfuseTargetAlwaysHitsInRainHitsTargetInSky)
        return data.playerConfusion.active ? 0.f : 40.f;
    if (code == FC::AttractTarget)
    {
        if (data.playerBattler.infatuated || !data.playerPkmn || !data.opponentPkmn)
            return 0.f;
        auto playerGender = data.playerPkmn->getGender();
        auto foeGender    = data.opponentPkmn->getGender();
        if (playerGender == Pkmn::Gender::GENDERLESS || foeGender == Pkmn::Gender::GENDERLESS || playerGender == foeGender)
            return 0.f;
        return 40.f;
    }

    if (code == FC::AddSpikesToFoeSide && data.playerSpikes > 0)
        return 0.f;

    std::string name = MoveDef::FunctionCodeToString(code);
    if (name.rfind("RaiseUser", 0) == 0)
        return stagesFlat(data.opponentBattler) ? 40.f : 1.f;
    return 40.f;
}

} // namespace

Move::MovePtr struggleMove()
{
    auto def = std::make_shared<MoveDef>();
    def->setId("STRUGGLE");
    def->setName("Struggle");
    def->setType(""); // typeless — hits Ghost, no STAB
    def->setCategory(MoveDef::Category::PHYSICAL);
    def->setPower(50);
    def->setAccuracy(0);
    def->setTotalPP(1);
    def->setTarget(MoveDef::Target::NearFoe);
    def->setFunctionCode(MoveDef::FunctionCode::RecoilQuarterOfDamageDealt);
    auto move = std::make_shared<Move>(def);
    move->setCurrentPP(1);
    return move;
}

void SingleBattleSceneData::chooseOpponentMove()
{
    if (!opponentPkmn)
    {
        encounterMove = nullptr;
        return;
    }

    clearEncoreIfMoveDepleted(opponentPkmn, opponentBattler);
    if (opponentBattler.recharging)
    {
        // Any move slot works; resolver short-circuits to the recharge message.
        for (auto const& move : opponentPkmn->getMoves())
        {
            if (move && move->getDefinition())
            {
                encounterMove = move;
                return;
            }
        }
        encounterMove = struggleMove();
        return;
    }
    if (opponentBattler.charging)
    {
        for (auto const& move : opponentPkmn->getMoves())
        {
            if (!move || !move->getDefinition())
                continue;
            if (!isTwoTurnChargingMove(move->getDefinition()->getFunctionCode()))
                continue;
            if (!opponentBattler.lastMoveId.empty()
                && opponentBattler.lastMoveId != move->getDefinition()->getId())
                continue;
            encounterMove = move;
            return;
        }
    }

    std::vector<Move::MovePtr> choices;
    float                      best = -1.f;
    for (auto const& move : opponentPkmn->getMoves())
    {
        if (!move || !move->getDefinition() || move->getCurrentPP() == 0)
            continue;
        auto def = move->getDefinition();
        if (opponentBattler.disableTurns > 0 && opponentBattler.disabledMoveId == def->getId())
            continue;
        if (opponentBattler.encoreTurns > 0 && !opponentBattler.encoreMoveId.empty()
            && opponentBattler.encoreMoveId != def->getId())
            continue;
        if (opponentBattler.multiTurnLock > 0 && !opponentBattler.rolloutMoveId.empty()
            && opponentBattler.rolloutMoveId != def->getId())
            continue;
        if (opponentBattler.multiTurnLock > 0 && !opponentBattler.rampageMoveId.empty()
            && opponentBattler.rampageMoveId != def->getId())
            continue;
        if (opponentBattler.bideActive
            && def->getFunctionCode() != MoveDef::FunctionCode::MultiTurnAttackBideThenReturnDoubleDamage)
            continue;
        if (opponentBattler.uproarTurns > 0 && !opponentBattler.uproarMoveId.empty()
            && opponentBattler.uproarMoveId != def->getId())
            continue;
        if (opponentBattler.torment && opponentBattler.lastMoveId == def->getId())
            continue;
        if (opponentBattler.tauntTurns > 0 && def->getCategory() == MoveDef::STATUS)
            continue;
        if (opponentBattler.soundBlockTurns > 0 && def->hasFlag(MoveDef::Flag::Sound))
            continue;
        bool gorilla = !opponentBattler.abilitySuppressed && opponentBattler.abilityId == "GORILLATACTICS";
        bool klutz   = !opponentBattler.abilitySuppressed && opponentBattler.abilityId == "KLUTZ";
        std::string held;
        if (!opponentBattler.itemsBlocked && !klutz)
            held = ItemUtils::activeHeldId(opponentPkmn, &opponentBattler);
        bool choiceLocked = gorilla || held == "CHOICEBAND" || held == "CHOICESPECS" || held == "CHOICESCARF";
        if (choiceLocked && !opponentBattler.choiceLock.empty() && opponentBattler.choiceLock != def->getId())
            continue;
        if (held == "ASSAULTVEST" && def->getCategory() == MoveDef::STATUS)
            continue;
        if (playerBattler.imprison && playerPkmn)
        {
            bool sealed = false;
            for (auto const& slot : playerPkmn->getMoves())
            {
                if (slot && slot->getDefinition() && slot->getDefinition()->getId() == def->getId())
                {
                    sealed = true;
                    break;
                }
            }
            if (sealed)
                continue;
        }
        float score = 1.f;
        if (def->getCategory() != MoveDef::STATUS && def->getPower() > 0)
        {
            if (!playerPkmn)
                score = static_cast<float>(def->getPower());
            else
            {
                float effectiveness = effectivenessAgainst(playerPkmn, playerBattler, def->getType());
                score = effectiveness <= 0.f ? 0.f : effectiveness * static_cast<float>(def->getPower());
            }
        }
        else
            score = statusChoiceScore(*this, def);
        if (score > best)
        {
            best = score;
            choices.clear();
            choices.push_back(move);
        }
        else if (score == best)
            choices.push_back(move);
    }

    if (choices.empty())
    {
        encounterMove = struggleMove();
        return;
    }
    encounterMove = choices.at(Utils::randuint(0, choices.size() - 1));
}