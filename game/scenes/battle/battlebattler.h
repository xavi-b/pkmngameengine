#ifndef BATTLEBATTLER_H
#define BATTLEBATTLER_H

#include "pkmndef.h"

#include <array>
#include <cstddef>

// Per-side battle state (Gen III stat stages and volatiles used by move resolution).
struct BattleBattler
{
    enum class StageStat
    {
        Attack,
        Defense,
        SpecialAttack,
        SpecialDefense,
        Speed,
        Accuracy,
        Evasion,
        __SIZE
    };

    static constexpr size_t StageStatCount = static_cast<size_t>(StageStat::__SIZE);

    std::array<int, StageStatCount> stages = {};

    bool flinched            = false;
    bool protect             = false;
    bool endure              = false;
    bool charging            = false; // two-turn move charging
    bool recharging          = false;
    bool semiInvulnerable    = false;
    int  stockpile           = 0;
    size_t toxicCounter      = 0;
    int  bindingTurns        = 0;
    int  perishCount         = -1; // -1 = inactive
    bool seeded              = false;
    bool trapped             = false;
    bool minimized           = false;
    bool focusEnergy         = false;
    bool destinyBond         = false;
    bool mist                = false;
    bool safeguard           = false;
    int  disableTurns        = 0;
    int  encoreTurns         = 0;
    int  tauntTurns          = 0;
    int  torment             = 0;
    int  healBlockTurns      = 0;
    int  embargoTurns        = 0;
    int  substituteHP        = 0;
    bool yawnPending         = false;
    bool flashFireBoost      = false;
    int  furyCutterPower     = 0;
    int  rolloutPower        = 0;
    int  multiTurnLock       = 0; // Outrage/Thrash remaining
    bool bideActive          = false;
    int  bideDamage          = 0;
    int  bideTurns           = 0;
    bool rampageConfuseAtEnd = false;

    void clear();
    void clearOnSwitch();

    int  getStage(StageStat stat) const;
    bool changeStage(StageStat stat, int delta); // returns true if changed
    void resetStages();
    void invertStages();
    void copyStagesFrom(BattleBattler const& other);
    void swapStagesWith(BattleBattler& other);

    // Gen III multipliers: combat stats use 2/2 table; Acc/Eva use 3/3.
    float stageMultiplier(StageStat stat) const;
    float accuracyStageMultiplier() const;
    float evasionStageMultiplier() const;

    size_t effectiveStat(size_t baseStat, StageStat stat, bool critical = false, bool attacker = true) const;
};

#endif // BATTLEBATTLER_H
