#include "battlebattler.h"

#include <algorithm>

void BattleBattler::clear()
{
    stages.fill(0);
    flinched             = false;
    protect              = false;
    endure               = false;
    charging             = false;
    recharging           = false;
    semiInvulnerable     = false;
    stockpile            = 0;
    toxicCounter         = 0;
    bindingTurns         = 0;
    perishCount          = -1;
    seeded               = false;
    trapped              = false;
    minimized            = false;
    focusEnergy          = false;
    destinyBond          = false;
    mist                 = false;
    safeguard            = false;
    disableTurns         = 0;
    encoreTurns          = 0;
    tauntTurns           = 0;
    torment              = 0;
    healBlockTurns       = 0;
    embargoTurns         = 0;
    substituteHP         = 0;
    yawnPending          = false;
    flashFireBoost       = false;
    furyCutterPower      = 0;
    rolloutPower         = 0;
    multiTurnLock        = 0;
    bideActive           = false;
    bideDamage           = 0;
    bideTurns            = 0;
    rampageConfuseAtEnd  = false;
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
