#ifndef BATTLEBATTLER_H
#define BATTLEBATTLER_H

#include "movedef.h"
#include "pkmn.h"
#include "pkmndef.h"

#include <array>
#include <cstddef>
#include <string>
#include <vector>

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
    enum class ProtectKind
    {
        None,
        Normal,
        KingsShield,
        BanefulBunker,
        Obstruct,
        SpikyShield
    };

    bool protect             = false;
    int  protectStreak       = 0;
    ProtectKind protectKind  = ProtectKind::None;
    MoveDef::Category lastDamageCategory = MoveDef::Category::STATUS;
    bool endure              = false;
    bool charging            = false; // two-turn move charging
    bool recharging          = false;
    enum class SemiKind
    {
        None,
        Sky,
        Underground,
        Underwater
    };
    SemiKind semiKind        = SemiKind::None;
    bool semiInvulnerable    = false;
    int  stockpile           = 0;
    int  stockpileDef        = 0;
    int  stockpileSpDef      = 0;
    size_t toxicCounter      = 0;
    int  bindingTurns        = 0;
    bool bindingBand         = false;
    int  perishCount         = -1; // -1 = inactive
    bool seeded              = false;
    bool cursed              = false;
    bool trapped             = false;
    bool meanLooked          = false; // Mean Look / Block / Spider Web
    bool jawLocked           = false; // Jaw Lock (both sides)
    bool noRetreat           = false; // No Retreat self-trap (cannot re-use)
    bool minimized           = false;
    int  focusEnergy         = 0; // stages from Focus Energy, Dire Hit, or Lansat
    bool laserFocus          = false;
    bool chargeBoost         = false;
    bool tarShot             = false;
    bool destinyBond         = false;
    bool destinyBondLock     = false; // end of the turn it was used; the next Destiny Bond fails until another move
    bool mist                = false;
    bool safeguard           = false;
    int         disableTurns = 0;
    std::string disabledMoveId;
    int         encoreTurns = 0;
    std::string encoreMoveId;
    int  tauntTurns          = 0;
    int  torment             = 0;
    int  healBlockTurns      = 0;
    int  embargoTurns        = 0;
    bool itemsBlocked        = false;
    int  substituteHP        = 0;
    int  yawnTurns           = 0; // 2 when Yawn lands; sleep at 1, on the following end of turn
    bool flashFireBoost      = false;
    int         furyCutterPower = 0;
    std::string furyCutterMoveId;
    int         rolloutPower = 0;
    std::string rolloutMoveId;
    bool        defenseCurl  = false;
    int         multiTurnLock = 0; // Outrage/Thrash or Rollout remaining
    std::string rampageMoveId;
    int         uproarTurns  = 0;
    std::string uproarMoveId;
    bool bideActive          = false;
    int  bideDamage          = 0;
    int  bideTurns           = 0;
    bool rampageConfuseAtEnd = false;
    bool infatuated          = false;

    // Cleared at the start of each turn. lastDamage and lastMove* survive the turn.
    bool        movedThisTurn         = false;
    bool        tookDamageThisTurn    = false;
    bool        statsLoweredThisTurn  = false;
    bool        statsRaisedThisTurn   = false;
    bool        lastMoveFailed        = false;
    int         lastDamage            = 0;
    std::string lastHitType;
    std::string lastMoveId;
    std::string choiceLock;
    std::vector<std::string> usedMoveIds;
    bool        meFirstBoost = false;
    bool        consumedBerry = false;
    bool        unburdenActive = false;
    std::string consumedItemId;
    bool        movesBecomeElectric = false;
    std::vector<std::string> typesOverride;
    bool                     roosting = false;
    std::vector<std::string> roostTypes;
    float                    weightFactor = 1.f;
    std::string abilityId;
    bool        abilitySuppressed = false;
    bool        bladeForme        = false;
    bool        truantLoaf        = false;
    bool        lockOn            = false;
    bool        magicCoat         = false;
    bool        rage              = false;
    int         airborneTurns     = 0;
    bool        telekinesis       = false;
    bool        nightmare         = false;
    bool        ingrain           = false;
    bool        aquaRing          = false;
    bool        octolock          = false;
    bool        imprison          = false;
    bool        grudge            = false;
    bool        powder            = false;
    bool        beakBlast         = false;
    bool        shellTrap         = false;
    bool        skyDropLocked     = false;
    bool        hangry            = false;
    bool        pirouette         = false;
    bool        gulpGulping       = false;
    bool        gulpGorging       = false;
    bool        groundedByMove    = false;
    int         soundBlockTurns   = 0;
    bool        ignoreEvasion     = false;
    bool        negateGhostImmunity = false;
    bool        negateDarkImmunity  = false;
    int         slowStartTurns    = 0;
    bool        iceFaceBroken     = false;
    bool        disguiseBroken    = false;
    bool        battleBondActive  = false; // Ash-Greninja after Battle Bond
    std::string spriteFormeSuffix;         // "_1" / "_2" / … for battle graphics
    bool          illusionActive = false;
    Pkmn::PkmnPtr illusionDisguise;
    bool        switchedInThisTurn = false;
    int         turnsOnField       = 0;
    // Moves attempted since send-out (Fake Out / First Impression first-action gate).
    int         activeMoveActions  = 0;
    bool        custapBerry        = false;
    bool        micleBerry         = false;
    int         metronomeCount     = 0;
    std::string metronomeMoveId;
    std::string recycledItemId;
    bool        mimicActive       = false;
    int         mimicSlot         = -1;
    std::string mimicOriginalId;
    int         mimicOriginalPP   = 0;
    int         attackOverride    = -1;
    int         defenseOverride   = -1;
    int         spAttackOverride  = -1;
    int         spDefenseOverride = -1;
    int         speedOverride     = -1;
    bool        transformActive   = false;
    Pkmn::PkmnPtr transformDisguise;
    std::array<std::string, 4> transformMoveIds{};
    std::array<size_t, 4>      transformMovePP{};

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
