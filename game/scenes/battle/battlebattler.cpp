#include "battlebattler.h"

#include <algorithm>

void BattleBattler::clear()
{
    stages.fill(0);
    flinched             = false;
    protect              = false;
    protectStreak        = 0;
    protectKind          = ProtectKind::None;
    lastDamageCategory   = MoveDef::Category::STATUS;
    endure               = false;
    charging             = false;
    recharging           = false;
    semiKind             = SemiKind::None;
    semiInvulnerable     = false;
    stockpile            = 0;
    stockpileDef         = 0;
    stockpileSpDef       = 0;
    toxicCounter         = 0;
    bindingTurns         = 0;
    bindingBand          = false;
    perishCount          = -1;
    seeded               = false;
    cursed               = false;
    trapped              = false;
    meanLooked           = false;
    jawLocked            = false;
    noRetreat            = false;
    minimized            = false;
    focusEnergy          = 0;
    laserFocus           = false;
    chargeBoost          = false;
    tarShot              = false;
    destinyBond          = false;
    destinyBondLock      = false;
    mist                 = false;
    safeguard            = false;
    disableTurns         = 0;
    disabledMoveId.clear();
    encoreTurns          = 0;
    encoreMoveId.clear();
    tauntTurns           = 0;
    torment              = 0;
    healBlockTurns       = 0;
    embargoTurns         = 0;
    itemsBlocked         = false;
    substituteHP         = 0;
    yawnTurns            = 0;
    flashFireBoost       = false;
    furyCutterPower      = 0;
    furyCutterMoveId.clear();
    rolloutPower         = 0;
    rolloutMoveId.clear();
    defenseCurl          = false;
    multiTurnLock        = 0;
    rampageMoveId.clear();
    uproarTurns          = 0;
    uproarMoveId.clear();
    bideActive           = false;
    bideDamage           = 0;
    bideTurns            = 0;
    rampageConfuseAtEnd  = false;
    infatuated           = false;
    movedThisTurn        = false;
    tookDamageThisTurn   = false;
    statsLoweredThisTurn = false;
    statsRaisedThisTurn  = false;
    lastMoveFailed       = false;
    lastDamage           = 0;
    lastHitType.clear();
    lastMoveId.clear();
    choiceLock.clear();
    usedMoveIds.clear();
    meFirstBoost = false;
    consumedBerry  = false;
    unburdenActive = false;
    consumedItemId.clear();
    movesBecomeElectric = false;
    typesOverride.clear();
    roosting = false;
    roostTypes.clear();
    weightFactor = 1.f;
    abilityId.clear();
    abilitySuppressed = false;
    bladeForme        = false;
    truantLoaf        = false;
    lockOn            = false;
    magicCoat         = false;
    rage              = false;
    airborneTurns     = 0;
    telekinesis       = false;
    nightmare         = false;
    ingrain           = false;
    aquaRing          = false;
    octolock          = false;
    imprison          = false;
    grudge            = false;
    powder            = false;
    beakBlast         = false;
    shellTrap         = false;
    skyDropLocked     = false;
    hangry            = false;
    pirouette         = false;
    gulpGulping       = false;
    gulpGorging       = false;
    groundedByMove    = false;
    soundBlockTurns   = 0;
    ignoreEvasion     = false;
    negateGhostImmunity = false;
    negateDarkImmunity  = false;
    slowStartTurns    = 0;
    iceFaceBroken     = false;
    disguiseBroken    = false;
    battleBondActive  = false;
    spriteFormeSuffix.clear();
    illusionActive     = false;
    illusionDisguise.reset();
    switchedInThisTurn = false;
    turnsOnField       = 0;
    activeMoveActions  = 0;
    custapBerry        = false;
    micleBerry         = false;
    metronomeCount     = 0;
    metronomeMoveId.clear();
    recycledItemId.clear();
    mimicActive       = false;
    mimicSlot         = -1;
    mimicOriginalId.clear();
    mimicOriginalPP   = 0;
    attackOverride    = -1;
    defenseOverride   = -1;
    spAttackOverride  = -1;
    spDefenseOverride = -1;
    speedOverride     = -1;
    transformActive   = false;
    transformDisguise.reset();
    transformMoveIds.fill({});
    transformMovePP.fill(0);
}

void BattleBattler::clearOnSwitch()
{
    clear();
}

int BattleBattler::getStage(StageStat stat) const
{
    return stages[static_cast<size_t>(stat)];
}

bool BattleBattler::changeStage(StageStat stat, int delta)
{
    if (delta == 0)
        return false;
    int& stage = stages[static_cast<size_t>(stat)];
    int  old   = stage;
    stage      = std::clamp(stage + delta, -6, 6);
    if (stage > old)
        statsRaisedThisTurn = true;
    return stage != old;
}

void BattleBattler::resetStages()
{
    stages.fill(0);
}

void BattleBattler::invertStages()
{
    for (int& stage : stages)
        stage = -stage;
}

void BattleBattler::copyStagesFrom(BattleBattler const& other)
{
    stages = other.stages;
}

void BattleBattler::swapStagesWith(BattleBattler& other)
{
    stages.swap(other.stages);
}

float BattleBattler::stageMultiplier(StageStat stat) const
{
    int stage = getStage(stat);
    if (stat == StageStat::Accuracy || stat == StageStat::Evasion)
    {
        // Gen III Acc/Eva: (3+n)/3 or 3/(3-n)
        if (stage >= 0)
            return (3.0f + stage) / 3.0f;
        return 3.0f / (3.0f - stage);
    }
    // Gen III Atk/Def/SpA/SpD/Spe: (2+n)/2 or 2/(2-n)
    if (stage >= 0)
        return (2.0f + stage) / 2.0f;
    return 2.0f / (2.0f - stage);
}

float BattleBattler::accuracyStageMultiplier() const
{
    return stageMultiplier(StageStat::Accuracy);
}

float BattleBattler::evasionStageMultiplier() const
{
    return stageMultiplier(StageStat::Evasion);
}

size_t BattleBattler::effectiveStat(size_t baseStat, StageStat stat, bool critical, bool attacker) const
{
    int stage = getStage(stat);
    if (critical)
    {
        // Gen III crits ignore positive defensive stages and negative offensive stages.
        if (attacker && stage < 0)
            stage = 0;
        if (!attacker && stage > 0)
            stage = 0;
    }

    float mult;
    if (stage >= 0)
        mult = (2.0f + stage) / 2.0f;
    else
        mult = 2.0f / (2.0f - stage);

    size_t value = static_cast<size_t>(baseStat * mult);
    return std::max<size_t>(1, value);
}
