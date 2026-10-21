#include "singlebattlescene.h"

#include "game.h"
#include "move.h"
#include "abilitybattle.h"
#include "moveeffects.h"
#include "statuscondition.h"
#include "trainerscenedata.h"
#include "scenes/bagscene.h"
#include "scenes/evolutionscene.h"
#include "scenes/newmovesscene.h"
#include "scenes/pkmnsscene.h"
#include "utils.h"
#include "animations/weather/blizzardanimation.h"
#include "animations/weather/harshsunlightanimation.h"
#include "animations/weather/rainanimation.h"
#include "animations/weather/sandstormanimation.h"

#include <algorithm>
#include <cctype>
#include <vector>

namespace
{
void restoreTransformedMoves(Pkmn::PkmnPtr const& pkmn, BattleBattler& battler)
{
    if (!battler.transformActive || !pkmn)
        return;
    auto& slots = pkmn->getMoves();
    for (int i = 0; i < 4; ++i)
    {
        if (battler.transformMoveIds[i].empty())
        {
            slots[i].reset();
            continue;
        }
        auto original = Game::instance()->data.moveDefFor(battler.transformMoveIds[i]);
        if (!original)
            continue;
        auto move = std::make_shared<Move>(original);
        move->setCurrentPP(battler.transformMovePP[i]);
        slots[i] = move;
    }
}

void applyBatonPass(BattleBattler& battler, ConfusionStatus& confusion, BatonPassState& pass)
{
    if (!pass.pending)
        return;
    battler.stages            = pass.stages;
    battler.substituteHP      = pass.substituteHP;
    battler.focusEnergy       = pass.focusEnergy;
    battler.perishCount       = pass.perishCount;
    battler.seeded            = pass.seeded;
    battler.ingrain           = pass.ingrain;
    battler.aquaRing          = pass.aquaRing;
    confusion                 = pass.confusion;
    battler.airborneTurns     = pass.airborneTurns;
    battler.telekinesis       = pass.telekinesis;
    battler.stockpile         = pass.stockpile;
    battler.stockpileDef      = pass.stockpileDef;
    battler.stockpileSpDef    = pass.stockpileSpDef;
    battler.attackOverride    = pass.attackOverride;
    battler.defenseOverride   = pass.defenseOverride;
    battler.spAttackOverride  = pass.spAttackOverride;
    battler.spDefenseOverride = pass.spDefenseOverride;
    battler.speedOverride     = pass.speedOverride;
    pass                      = {};
}
} // namespace

SingleBattleScene::SingleBattleScene(SDL_Renderer* renderer) : Scene(renderer)
{
    battleBackground = std::make_unique<BattleBackground>(renderer);
    singleBattleUi   = std::make_unique<SingleBattleUi>(renderer);
}

SingleBattleScene::~SingleBattleScene()
{
}

void SingleBattleScene::initPhases()
{
    startPhase         = std::make_unique<SingleBattleStartPhase>(*this, data, renderer);
    weatherPhase       = std::make_unique<SingleBattleWeatherPhase>(*this, data, renderer);
    actionsPhase       = std::make_unique<SingleBattleActionsPhase>(*this, data, renderer);
    movesPhase         = std::make_unique<SingleBattleMovesPhase>(*this, data, renderer);
    playerPkmnsPhase   = std::make_unique<SingleBattlePlayerPkmnsPhase>(*this, data, renderer);
    opponentPkmnsPhase = std::make_unique<SingleBattleOpponentPkmnsPhase>(*this, data, renderer);
    opponentItemsPhase = std::make_unique<SingleBattleOpponentItemsPhase>(*this, data, renderer);
    endPhase           = std::make_unique<SingleBattleEndPhase>(*this, data, renderer);
}

SingleBattlePhase* SingleBattleScene::currentPhase()
{
    switch (data->state)
    {
    case START:
        return startPhase.get();
    case WEATHER:
        return weatherPhase.get();
    case ACTIONS:
        return actionsPhase.get();
    case MOVES:
        return movesPhase.get();
    case PLAYER_MOVES:
        return playerMovesPhase.get();
    case PLAYER_ITEMS:
        return playerItemsPhase.get();
    case PLAYER_PKMNS:
        return playerPkmnsPhase.get();
    case PLAYER_RUN:
        return playerRunPhase.get();
    case OPPONENT_PKMNS:
        return opponentPkmnsPhase.get();
    case OPPONENT_ITEMS:
        return opponentItemsPhase.get();
    case OPPONENT_MOVES:
        return opponentMovesPhase.get();
    case OPPONENT_RUN:
        return opponentRunPhase.get();
    case EXPERIENCE:
        return experiencePhase.get();
    case END:
        return endPhase.get();
    case BAG:
    case PKMNS:
        return nullptr;
    }
    return nullptr;
}

void SingleBattleScene::init()
{
    data->state = START;
    data->expGained.clear();
    data->participatingPlayerPkmns.clear();
    data->playerConfusion.clear();
    data->opponentConfusion.clear();
    data->playerBattler.clear();
    data->opponentBattler.clear();
    data->playerSpikes        = 0;
    data->opponentSpikes      = 0;
    data->playerReflect       = 0;
    data->opponentReflect     = 0;
    data->playerLightScreen   = 0;
    data->opponentLightScreen = 0;
    data->playerAuroraVeil     = 0;
    data->opponentAuroraVeil   = 0;
    data->playerSafeguard      = 0;
    data->opponentSafeguard    = 0;
    data->playerMist           = 0;
    data->opponentMist         = 0;
    data->playerWideGuard      = false;
    data->opponentWideGuard    = false;
    data->playerQuickGuard     = false;
    data->opponentQuickGuard   = false;
    data->playerCraftyShield   = false;
    data->opponentCraftyShield = false;
    data->playerMatBlock       = false;
    data->opponentMatBlock     = false;
    data->strongWinds         = false;
    data->playerWishTurns     = 0;
    data->opponentWishTurns   = 0;
    data->playerWishHeal      = 0;
    data->opponentWishHeal    = 0;
    data->battleTurn          = 0;
    if (data->playerPkmn)
        data->participatingPlayerPkmns.insert(data->playerPkmn);

    boost::format firstPkmnText =
        boost::format(lc::translate("%1% go !"))
        % (data->playerPkmn ? battleDisplayName(data->playerPkmn, data->playerBattler) : "#ERROR");
    startPhase->init(encounterStartText(), firstPkmnText.str());
    actionsPhase->init();
    movesPhase->init();
}

void SingleBattleScene::update(Inputs const* inputs)
{
    if (Game::instance()->isDebug() && data->previousState != data->state)
        std::cout << name() << ": " << battleStateToString(data->state) << std::endl;

    data->previousState = data->state;

    if (auto* phase = currentPhase())
        phase->update(inputs);

    if (singleBattleUi)
        singleBattleUi->incrementPresentation();

    if (fieldWeather)
    {
        if (!fieldWeather->isStarted())
            fieldWeather->start();
        fieldWeather->incrementTicks();
    }

    if (data->previousState != data->state)
    {
        if (auto* phase = currentPhase())
            phase->preupdate();
    }
}

void SingleBattleScene::draw(Fps const* fps, RenderSizes rs)
{
    battleBackground->draw(fps, rs);
    if (data && singleBattleUi)
    {
        auto statusOf = [](Pkmn::PkmnPtr const& pkmn) {
            return pkmn ? pkmn->getStatusCondition() : Pkmn::StatusCondition::NONE;
        };
        singleBattleUi->setOverlays(data->playerConfusion.active,
                                    data->opponentConfusion.active,
                                    statusOf(data->playerPkmn),
                                    statusOf(data->opponentPkmn));
        singleBattleUi->setSemiInvulnerable(data->playerBattler.semiInvulnerable,
                                            data->opponentBattler.semiInvulnerable);
        auto shown = [](BattleBattler const& battler) {
            return battler.transformDisguise ? battler.transformDisguise : battler.illusionDisguise;
        };
        singleBattleUi->syncDisguise(shown(data->playerBattler), shown(data->opponentBattler));
        singleBattleUi->syncFormeSprites(data->playerBattler.spriteFormeSuffix,
                                         data->opponentBattler.spriteFormeSuffix);
    }
    singleBattleUi->draw(fps, rs);
    if (fieldWeather)
        fieldWeather->draw(fps, rs);

    if (auto* phase = currentPhase())
        phase->draw(fps, rs);
}

bool SingleBattleScene::popScene() const
{
    return data->state == END && endPhase && endPhase->isFinished();
}

std::string SingleBattleScene::name()
{
    return "SingleBattleScene";
}

SingleBattleScene::DamageRoll SingleBattleScene::computeDamage(Pkmn::PkmnPtr const& attacker,
                                                                  Pkmn::PkmnPtr const& defender,
                                                                  Move::MovePtr const& move,
                                                                  Map::Weather const&  weather,
                                                                  BattleBattler const* attackerBattler,
                                                                  BattleBattler const* defenderBattler,
                                                                  size_t               powerOverride,
                                                                  bool                 forceNoCrit,
                                                                  bool                 forceCrit,
                                                                  float                screenMultiplier,
                                                                  bool                 wonderRoom,
                                                                  std::string const&   typeOverride,
                                                                  DamageAdjustments    adjustments)
{
    DamageRoll roll;
    if (!attacker || !defender || !move)
        return roll;

    auto category = move->getDefinition()->battleCategory();
    auto specialType = [](std::string const& type) {
        return type == "FIRE" || type == "WATER" || type == "GRASS" || type == "ELECTRIC" || type == "PSYCHIC"
            || type == "ICE" || type == "DRAGON" || type == "DARK";
    };
    if (!typeOverride.empty() && typeOverride != "TYPELESS" && category != MoveDef::STATUS)
        category = specialType(typeOverride) ? MoveDef::SPECIAL : MoveDef::PHYSICAL;
    if (adjustments.forcePhysical)
        category = MoveDef::PHYSICAL;
    else if (adjustments.forceSpecial)
        category = MoveDef::SPECIAL;
    if (category == MoveDef::STATUS)
        return roll;

    // https://bulbapedia.bulbagarden.net/wiki/Damage#Generation_III
    float  level = attacker->getLevel();
    size_t power = powerOverride > 0 ? powerOverride : move->getDefinition()->getPower();
    bool usePhysicalDefense = adjustments.usePhysicalDefense || category == MoveDef::PHYSICAL;
    (void)wonderRoom;
    size_t a     = category == MoveDef::PHYSICAL ? attacker->getStats()[PkmnDef::ATTACK]
                                                 : attacker->getStats()[PkmnDef::SPECIAL_ATTACK];
    if (adjustments.useTargetAttack)
        a = defender->getStats()[PkmnDef::ATTACK];
    else if (adjustments.useUserDefense)
        a = attacker->getStats()[PkmnDef::DEFENSE];
    else if (attackerBattler)
    {
        int override = category == MoveDef::PHYSICAL ? attackerBattler->attackOverride : attackerBattler->spAttackOverride;
        if (override >= 0)
            a = static_cast<size_t>(override);
    }
    size_t d     = usePhysicalDefense ? defender->getStats()[PkmnDef::DEFENSE]
                                      : defender->getStats()[PkmnDef::SPECIAL_DEFENSE];
    if (defenderBattler)
    {
        int override = usePhysicalDefense ? defenderBattler->defenseOverride : defenderBattler->spDefenseOverride;
        if (override >= 0)
            d = static_cast<size_t>(override);
    }

    auto heldOf = [](Pkmn::PkmnPtr const& mon, BattleBattler const* battler) {
        if (battler && (battler->itemsBlocked || battler->embargoTurns > 0))
            return std::string();
        if (battler && !battler->abilitySuppressed && battler->abilityId == "KLUTZ")
            return std::string();
        if (!mon || !mon->getHeldItem() || !mon->getHeldItem()->getDefinition())
            return std::string();
        return mon->getHeldItem()->getDefinition()->getId();
    };
    auto speciesOf = [](Pkmn::PkmnPtr const& mon) {
        return mon && mon->getDefinition() ? mon->getDefinition()->getId() : std::string();
    };
    bool usingAttack = adjustments.useTargetAttack
                    || (category == MoveDef::PHYSICAL && !adjustments.useUserDefense);
    bool usingSpecial = !adjustments.useTargetAttack && !adjustments.useUserDefense && category == MoveDef::SPECIAL;
    if (usingAttack || usingSpecial)
    {
        Pkmn::PkmnPtr        statMon     = adjustments.useTargetAttack ? defender : attacker;
        BattleBattler const* statBattler = adjustments.useTargetAttack ? defenderBattler : attackerBattler;
        std::string          item        = heldOf(statMon, statBattler);
        std::string          species     = speciesOf(statMon);
        if (usingAttack)
        {
            if (item == "CHOICEBAND")
                a = std::max<size_t>(1, a * 3 / 2);
            if (item == "THICKCLUB" && (species == "CUBONE" || species == "MAROWAK"))
                a *= 2;
            if (item == "LIGHTBALL" && species == "PIKACHU")
                a *= 2;
            if (statBattler && !statBattler->abilitySuppressed
                && (statBattler->abilityId == "HUGEPOWER" || statBattler->abilityId == "PUREPOWER"))
                a *= 2;
        }
        else if (item == "LIGHTBALL" && species == "PIKACHU")
            a *= 2;
    }
    if (adjustments.blockCrit)
        forceNoCrit = true;
    bool criticalRoll = forceCrit;
    if (!criticalRoll && !forceNoCrit)
    {
        int stage = attackerBattler ? attackerBattler->focusEnergy : 0;
        if (move->getDefinition()->hasFlag(MoveDef::Flag::HighCriticalHitRate))
            ++stage;
        bool itemLive = attackerBattler && !attackerBattler->itemsBlocked && attackerBattler->embargoTurns <= 0
                     && (attackerBattler->abilitySuppressed || attackerBattler->abilityId != "KLUTZ");
        if (itemLive && attacker && attacker->getHeldItem() && attacker->getHeldItem()->getDefinition())
        {
            std::string held = attacker->getHeldItem()->getDefinition()->getId();
            if (held == "SCOPELENS")
                ++stage;
            else if (held == "LUCKYPUNCH" && speciesOf(attacker) == "CHANSEY")
                stage += 2;
            else if (held == "STICK" && speciesOf(attacker) == "FARFETCHD")
                stage += 2;
        }
        size_t denom = 16;
        if (stage == 1)
            denom = 8;
        else if (stage == 2)
            denom = 4;
        else if (stage == 3)
            denom = 3;
        else if (stage >= 4)
            denom = 2;
        criticalRoll = Utils::randuint(0, denom - 1) == 0;
    }
    if (heldOf(defender, defenderBattler) == "METALPOWDER" && speciesOf(defender) == "DITTO" && usePhysicalDefense)
        d *= 2;
    if (heldOf(defender, defenderBattler) == "DEEPSEASCALE" && speciesOf(defender) == "CLAMPERL" && !usePhysicalDefense)
        d *= 2;
    if (heldOf(attacker, attackerBattler) == "DEEPSEATOOTH" && speciesOf(attacker) == "CLAMPERL" && category != MoveDef::PHYSICAL)
        a *= 2;
    auto soulDew = [&](Pkmn::PkmnPtr const& mon, BattleBattler const* battler) {
        if (heldOf(mon, battler) != "SOULDEW")
            return false;
        std::string species = speciesOf(mon);
        return species == "LATIAS" || species == "LATIOS";
    };
    if (soulDew(attacker, attackerBattler) && category == MoveDef::SPECIAL && !adjustments.useUserDefense
        && !adjustments.useTargetAttack)
        a = std::max<size_t>(1, a * 3 / 2);
    if (soulDew(defender, defenderBattler) && !usePhysicalDefense)
        d = std::max<size_t>(1, d * 3 / 2);

    if (!adjustments.ignoreAttackStages && (attackerBattler || (adjustments.useTargetAttack && defenderBattler)))
    {
        auto atkStat = adjustments.useUserDefense
                           ? BattleBattler::StageStat::Defense
                           : (adjustments.useTargetAttack || category == MoveDef::PHYSICAL
                                  ? BattleBattler::StageStat::Attack
                                  : BattleBattler::StageStat::SpecialAttack);
        BattleBattler const* stageBattler = adjustments.useTargetAttack ? defenderBattler : attackerBattler;
        if (stageBattler)
            a = stageBattler->effectiveStat(a, atkStat, criticalRoll, true);
    }
    if (defenderBattler && !adjustments.ignoreDefenseStages)
    {
        auto defStat = usePhysicalDefense ? BattleBattler::StageStat::Defense
                                          : BattleBattler::StageStat::SpecialDefense;
        d            = defenderBattler->effectiveStat(d, defStat, criticalRoll, false);
    }

    if (d == 0)
        d = 1;
    if (move->getDefinition()->getFunctionCode() == MoveDef::FunctionCode::UserFaintsExplosive)
        d = std::max<size_t>(1, d / 2);
    bool guts = attackerBattler && !attackerBattler->abilitySuppressed && attackerBattler->abilityId == "GUTS";
    float burn =
        (category == MoveDef::PHYSICAL && attacker->getStatusCondition() == Pkmn::StatusCondition::BURN && !guts) ? 0.5 : 1.0;
    float  screen            = criticalRoll ? 1.f : screenMultiplier;
    size_t targets           = 1;
    std::string attackType = typeOverride.empty() ? move->getDefinition()->getType() : typeOverride;
    float  weatherMultiplier = 1.0;
    if (weather == Map::Weather::RAIN)
    {
        if (attackType == "WATER")
            weatherMultiplier = 1.5;
        else if (attackType == "FIRE")
            weatherMultiplier = 0.5;
    }
    else if (weather == Map::Weather::HARSH_SUNLIGHT)
    {
        if (attackType == "FIRE")
            weatherMultiplier = 1.5;
        else if (attackType == "WATER")
            weatherMultiplier = 0.5;
    }
    float  ff     = 1.0;
    float  damage = (((2 * level / 5 + 2) * power * a / d) / 50 + 2) * burn * screen * targets * weatherMultiplier * ff;
    size_t stockpile = 1;
    float  critical  = criticalRoll ? 2.0 : 1.0;
    size_t doubledmg = 1;
    size_t charge    = 1;
    if (attackerBattler && attackerBattler->chargeBoost && attackType == "ELECTRIC")
        charge = 2;
    if (defenderBattler && defenderBattler->minimized
        && move->getDefinition()->hasFlag(MoveDef::Flag::TramplesMinimize))
        doubledmg *= 2;
    size_t hh             = 1;
    auto        pkmnTypes = activeTypes(attacker, attackerBattler);
    float       stab      = std::find(pkmnTypes.begin(), pkmnTypes.end(), attackType) != pkmnTypes.end()
                              ? (adjustments.stabBonus > 0.f ? adjustments.stabBonus : 1.5f)
                              : 1.0;
    float typeEffectiveness = 1.0;
    for (auto const& defenderTypeId : activeTypes(defender, defenderBattler))
    {
        auto defenderType = Game::instance()->data.typeFor(defenderTypeId);
        if (!defenderType)
            continue;

        if (move->getDefinition()->getFunctionCode() == MoveDef::FunctionCode::FreezeTargetSuperEffectiveAgainstWater
            && defenderTypeId == "WATER")
        {
            typeEffectiveness *= 2.0;
            continue;
        }

        auto const& immunities = defenderType->getImmunities();
        bool scrappy = attackerBattler && !attackerBattler->abilitySuppressed && attackerBattler->abilityId == "SCRAPPY"
                    && defenderTypeId == "GHOST" && (attackType == "NORMAL" || attackType == "FIGHTING");
        bool ringTarget = heldOf(defender, defenderBattler) == "RINGTARGET";
        bool pierceGround = adjustments.pierceGroundImmunity && attackType == "GROUND";
        if (!scrappy && !ringTarget && !pierceGround
            && std::find(immunities.begin(), immunities.end(), attackType) != immunities.end())
            typeEffectiveness *= 0.0;

        auto const& weaknesses = defenderType->getWeaknesses();
        if (std::find(weaknesses.begin(), weaknesses.end(), attackType) != weaknesses.end())
            typeEffectiveness *= 2.0;

        auto const& resistances = defenderType->getResistances();
        if (std::find(resistances.begin(), resistances.end(), attackType) != resistances.end())
            typeEffectiveness *= 0.5;

        if (defenderBattler && defenderBattler->negateGhostImmunity && attackType == "NORMAL"
            && defenderTypeId == "GHOST")
            typeEffectiveness = 1.f;
        if (defenderBattler && defenderBattler->negateDarkImmunity && attackType == "PSYCHIC"
            && defenderTypeId == "DARK")
            typeEffectiveness = 1.f;
    }
    if (adjustments.includeFlying && attackType != "FLYING")
    {
        for (auto const& defenderTypeId : activeTypes(defender, defenderBattler))
        {
            auto defenderType = Game::instance()->data.typeFor(defenderTypeId);
            if (!defenderType)
                continue;
            auto const& immunities = defenderType->getImmunities();
            if (std::find(immunities.begin(), immunities.end(), "FLYING") != immunities.end())
                typeEffectiveness *= 0.0;
            auto const& weaknesses = defenderType->getWeaknesses();
            if (std::find(weaknesses.begin(), weaknesses.end(), "FLYING") != weaknesses.end())
                typeEffectiveness *= 2.0;
            auto const& resistances = defenderType->getResistances();
            if (std::find(resistances.begin(), resistances.end(), "FLYING") != resistances.end())
                typeEffectiveness *= 0.5;
        }
    }
    float random = Utils::randuint(85, 100) / 100.0;
    damage *= stockpile * critical * doubledmg * charge * hh * stab * typeEffectiveness * random * adjustments.multiplier;
    if (damage < 1.0 && typeEffectiveness > 0.0)
        damage = 1.0;
    roll.amount        = static_cast<size_t>(damage);
    roll.critical      = criticalRoll;
    roll.effectiveness = typeEffectiveness;
    return roll;
}

bool SingleBattleScene::opponentSentFromBall() const
{
    return false;
}

std::string SingleBattleScene::opponentTrainerGraphic() const
{
    return {};
}

std::array<SingleBattleUi::LineupBall, 6> SingleBattleScene::opponentLineup() const
{
    return {};
}

void SingleBattleScene::changeWeather(Map::Weather weather)
{
    weatherPhase->setWeather(weather);
}

void SingleBattleScene::applyFieldWeather(Map::Weather weather)
{
    fieldWeather.reset();
    if (weather == Map::Weather::NONE)
        return;

    switch (weather)
    {
    case Map::HARSH_SUNLIGHT:
        fieldWeather = std::make_unique<HarshSunlightAnimation>(renderer, false);
        break;
    case Map::HAIL:
        fieldWeather = std::make_unique<BlizzardAnimation>(renderer, false);
        break;
    case Map::SANDSTORM:
        fieldWeather = std::make_unique<SandstormAnimation>(renderer, false);
        break;
    case Map::RAIN:
        fieldWeather = std::make_unique<RainAnimation>(renderer, false);
        break;
    default:
        break;
    }
}

WeatherAnimation* SingleBattleScene::fieldWeatherAnim() const
{
    return fieldWeather.get();
}

void SingleBattleScene::setOpponentPkmn(Pkmn::PkmnPtr const& newOpponentPkmn, bool deferSwitchInAbility)
{
    if (data->opponentPkmn && newOpponentPkmn != data->opponentPkmn)
    {
        restoreTransformedMoves(data->opponentPkmn, data->opponentBattler);
        if (data->opponentBattler.mimicActive && data->opponentBattler.mimicSlot >= 0)
        {
            auto original = Game::instance()->data.moveDefFor(data->opponentBattler.mimicOriginalId);
            if (original)
            {
                auto restored = std::make_shared<Move>(original);
                restored->setCurrentPP(data->opponentBattler.mimicOriginalPP);
                data->opponentPkmn->getMoves()[data->opponentBattler.mimicSlot] = restored;
            }
        }
        abilityOnSwitchOut(*data, false);
        releaseSkyDrop(*data, false);
    }
    // Healing Wish / Lunar Dance apply after hazards (in the PKMNS phase), not here.
    data->opponentPkmn = newOpponentPkmn;
    data->opponentConfusion.clear();
    data->opponentBattler.clearOnSwitch();
    data->opponentBattler.itemsBlocked       = false;
    data->opponentBattler.switchedInThisTurn = true;
    assignBattleAbility(newOpponentPkmn, data->opponentBattler);
    applyBatonPass(data->opponentBattler, data->opponentConfusion, data->opponentBatonPass);
    if (!deferSwitchInAbility)
    {
        AbilityEvent event;
        event.playerSide = false;
        triggerAbility(*data, AbilityMoment::SwitchIn, event);
        data->pendingAbilityMessages.insert(data->pendingAbilityMessages.end(), event.messages.begin(), event.messages.end());
    }

    data->expGained.clear();
    data->participatingPlayerPkmns.clear();
    if (data->playerPkmn)
        data->participatingPlayerPkmns.insert(data->playerPkmn);

    singleBattleUi->setFoePkmn(newOpponentPkmn);
}

void SingleBattleScene::setPlayerPkmn(Pkmn::PkmnPtr const& newPlayerPkmn, bool deferSwitchInAbility)
{
    if (data->playerPkmn && newPlayerPkmn != data->playerPkmn)
    {
        restoreTransformedMoves(data->playerPkmn, data->playerBattler);
        if (data->playerBattler.mimicActive && data->playerBattler.mimicSlot >= 0)
        {
            auto original = Game::instance()->data.moveDefFor(data->playerBattler.mimicOriginalId);
            if (original)
            {
                auto restored = std::make_shared<Move>(original);
                restored->setCurrentPP(data->playerBattler.mimicOriginalPP);
                data->playerPkmn->getMoves()[data->playerBattler.mimicSlot] = restored;
            }
        }
        abilityOnSwitchOut(*data, true);
        releaseSkyDrop(*data, true);
    }
    // Healing Wish / Lunar Dance apply after hazards (in the PKMNS phase), not here.
    data->playerPkmn = newPlayerPkmn;
    data->playerConfusion.clear();
    data->playerBattler.clearOnSwitch();
    data->playerBattler.itemsBlocked       = false;
    data->playerBattler.switchedInThisTurn = true;
    assignBattleAbility(newPlayerPkmn, data->playerBattler);
    applyBatonPass(data->playerBattler, data->playerConfusion, data->playerBatonPass);
    if (!deferSwitchInAbility)
    {
        AbilityEvent event;
        event.playerSide = true;
        triggerAbility(*data, AbilityMoment::SwitchIn, event);
        data->pendingAbilityMessages.insert(data->pendingAbilityMessages.end(), event.messages.begin(), event.messages.end());
    }
    singleBattleUi->setPlayerPkmn(newPlayerPkmn);
    if (newPlayerPkmn)
        data->participatingPlayerPkmns.insert(newPlayerPkmn);
}

bool SingleBattleScene::consumeEjectSwitch()
{
    if (data->playerEjectSwitch)
    {
        data->playerEjectSwitch = false;
        if (data->playerPkmn && !data->playerPkmn->isKO())
        {
            data->state = PKMNS;
            return true;
        }
    }
    if (data->opponentEjectSwitch)
    {
        data->opponentEjectSwitch = false;
        auto trainerData = std::dynamic_pointer_cast<TrainerSceneData>(data);
        if (!trainerData || !trainerData->opponentTrainer || !data->opponentPkmn || data->opponentPkmn->isKO())
            return false;
        std::vector<Pkmn::PkmnPtr> candidates;
        for (auto const& pkmn : trainerData->opponentTrainer->pkmns)
        {
            if (pkmn && !pkmn->isKO() && pkmn != data->opponentPkmn)
                candidates.push_back(pkmn);
        }
        if (!candidates.empty())
        {
            trainerData->nextOpponentPkmn = candidates[Utils::randuint(0, candidates.size() - 1)];
            data->state                   = OPPONENT_PKMNS;
            return true;
        }
    }
    return false;
}

void SingleBattleScene::resolveOpponentFaint()
{
    if (playerMovesPhase)
        playerMovesPhase->resolveOpponentFaint();
}

bool SingleBattleScene::pushScene() const
{
    return data->shouldBreakToEvolution || data->shouldGoToNewMovesScene || data->state == BAG || data->state == PKMNS;
}

void SingleBattleScene::popReset()
{
    if (data->shouldGoToNewMovesScene)
    {
        data->shouldGoToNewMovesScene = false;
        data->moveToLearnId.clear();
        if (!data->pendingMoveIds.empty())
            data->shouldBreakToNewMove = true;
        else if (data->evolutionDef)
            data->shouldBreakToEvolution = true;
        if (data->expPkmn == data->playerPkmn)
            singleBattleUi->setPlayerPkmn(data->playerPkmn);
        return;
    }

    if (data->shouldBreakToEvolution)
    {
        data->shouldBreakToEvolution = false;
        data->evolutionDef.reset();
        if (data->expPkmn)
            MoveLearn::queueMoveIds(data->expPkmn, data->pendingMoveIds, data->offeredMoveIds);
        data->shouldBreakToNewMove = !data->pendingMoveIds.empty();
        if (data->expPkmn == data->playerPkmn)
            singleBattleUi->setPlayerPkmn(data->playerPkmn);
        return;
    }

    switch (data->state)
    {
    case BAG: {
        data->state = PLAYER_ITEMS;
        break;
    }
    case PKMNS: {
        if (!data->newSelectedPkmn || data->newSelectedPkmn == data->playerPkmn)
        {
            if (Game::instance()->isDebug())
                std::cout << __PRETTY_FUNCTION__ << " no new selected pkmn" << std::endl;

            data->newSelectedPkmn.reset();
            data->playerSwitchQueued    = false;
            data->playerVoluntarySwitch = false;
            data->opponentActionLocked  = false;
            data->state                 = ACTIONS;
        }
        else
        {
            if (Game::instance()->isDebug())
                std::cout << __PRETTY_FUNCTION__ << " new selected pkmn: " << data->newSelectedPkmn->getDisplayName()
                          << std::endl;

            boost::format pkmnEnterText =
                boost::format(lc::translate("%1% enters the battle !"))
                % predictedIllusionName(data->newSelectedPkmn, true, *data);
            playerPkmnsPhase->showEnterSpeech(pkmnEnterText.str());

            data->playerSwitchQueued = true;
            if (data->opponentAction == BattleActions::PKMNS && !data->opponentSwitchDone && opponentOutspeeds(*data))
                data->state = OPPONENT_PKMNS;
            else
                data->state = PLAYER_PKMNS;
        }
        break;
    }
    default:
        break;
    }
}

std::unique_ptr<Scene> SingleBattleScene::nextScene()
{
    if (data->shouldGoToNewMovesScene)
    {
        auto move = Game::instance()->data.moveDefFor(data->moveToLearnId);
        if (data->expPkmn && move)
            return std::make_unique<NewMovesScene>(renderer, data->expPkmn, move);

        data->shouldGoToNewMovesScene = false;
        data->moveToLearnId.clear();
        if (!data->pendingMoveIds.empty())
            data->shouldBreakToNewMove = true;
        else if (data->evolutionDef)
            data->shouldBreakToEvolution = true;
    }

    if (data->shouldBreakToEvolution)
        return std::make_unique<EvolutionScene>(renderer, data->expPkmn, data->evolutionDef);

    switch (data->state)
    {
    case BAG:
        return std::make_unique<BagScene>(renderer, data->selectedItem, data->itemTargetPkmn, BagScene::Mode::Battle);
    case PKMNS:
        data->newSelectedPkmn = data->playerPkmn;
        return std::make_unique<PkmnsScene>(renderer, data->newSelectedPkmn, data->playerPkmn == nullptr);
    default:
        return nullptr;
    }
}
