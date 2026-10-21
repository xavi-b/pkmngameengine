#ifndef SINGLEBATTLESCENEDATA_H
#define SINGLEBATTLESCENEDATA_H

#include "battlebattler.h"
#include "battles/battleactions.h"
#include "confusionstatus.h"
#include "item.h"
#include "map.h"
#include "pkmn.h"

#include <map>
#include <set>
#include <string>
#include <vector>

enum BattleState
{
    START,
    WEATHER,
    ACTIONS,
    MOVES,
    BAG,
    PKMNS,
    PLAYER_MOVES,
    PLAYER_ITEMS,
    PLAYER_PKMNS,
    PLAYER_RUN,
    OPPONENT_PKMNS,
    OPPONENT_ITEMS,
    OPPONENT_MOVES,
    OPPONENT_RUN,
    EXPERIENCE,
    END
};

struct DelayedStrike
{
    int           turns        = 0;
    bool          hitsPlayer   = false;
    std::string   moveId;
    Pkmn::PkmnPtr user;
    bool          userIsPlayer = false;
    size_t        damage       = 0;
};

struct BatonPassState
{
    bool                                 pending = false;
    std::array<int, BattleBattler::StageStatCount> stages{};
    int                                  substituteHP = 0;
    int                                  focusEnergy  = 0;
    int                                  perishCount  = -1;
    bool                                 seeded       = false;
    bool                                 ingrain      = false;
    bool                                 aquaRing     = false;
    ConfusionStatus                      confusion;
    int                                  airborneTurns = 0;
    bool                                 telekinesis   = false;
    int                                  stockpile     = 0;
    int                                  stockpileDef  = 0;
    int                                  stockpileSpDef = 0;
    int                                  attackOverride    = -1;
    int                                  defenseOverride   = -1;
    int                                  spAttackOverride  = -1;
    int                                  spDefenseOverride = -1;
    int                                  speedOverride     = -1;
};

struct SingleBattleSceneData
{
    virtual ~SingleBattleSceneData() = default;

    BattleState state         = BattleState::START;
    BattleState previousState = BattleState::END;

    Map::Weather weather = Map::Weather::NONE;

    Pkmn::PkmnPtr opponentPkmn;
    Pkmn::PkmnPtr playerPkmn;
    size_t        runAttemps    = 0;
    Move::MovePtr encounterMove = nullptr;
    Move::MovePtr playerMove    = nullptr;
    std::string   lastMoveUsedId;
    bool          playerFirst   = true;

    ConfusionStatus playerConfusion;
    ConfusionStatus opponentConfusion;

    BattleBattler playerBattler;
    BattleBattler opponentBattler;

    // Side field effects
    int  playerSpikes        = 0;
    int  opponentSpikes      = 0;
    int  playerReflect       = 0;
    int  opponentReflect     = 0;
    int  playerLightScreen   = 0;
    int  opponentLightScreen = 0;
    int  playerAuroraVeil    = 0;
    int  opponentAuroraVeil  = 0;
    int  playerSafeguard     = 0;
    int  opponentSafeguard   = 0;
    int  playerMist          = 0;
    int  opponentMist        = 0;
    bool playerWideGuard     = false;
    bool opponentWideGuard   = false;
    bool playerQuickGuard    = false;
    bool opponentQuickGuard  = false;
    bool playerCraftyShield  = false;
    bool opponentCraftyShield = false;
    bool playerMatBlock      = false;
    bool opponentMatBlock    = false;
    bool playerSnatch        = false;
    bool opponentSnatch      = false;
    int  weatherTurns        = 0;
    bool weatherFromMove     = false;
    bool doublePrize         = false;
    bool ionDeluge           = false;
    std::vector<std::string> pendingAbilityMessages;
    int  playerLuckyChant    = 0;
    int  opponentLuckyChant  = 0;
    int  mudSportTurns       = 0;
    int  waterSportTurns     = 0;
    bool fairyLock           = false;
    int  playerWishTurns     = 0;
    int  opponentWishTurns   = 0;
    size_t playerWishHeal    = 0;
    size_t opponentWishHeal  = 0;
    bool playerHealingWish   = false;
    bool opponentHealingWish = false;
    BatonPassState playerBatonPass;
    BatonPassState opponentBatonPass;
    bool playerLunarDance    = false;
    bool opponentLunarDance  = false;
    bool playerMustMoveLast  = false;
    bool opponentMustMoveLast = false;
    bool pendingPlayerReplace = false; // double-KO: replace player after foe handling
    bool replaceAfterResidual = false; // residual faint → new turn at ACTIONS after send-out
    int  fairyLockTurns       = 0;     // Gen 7+: binds through the next turn
    bool forceWildEnd         = false; // Roar / Dragon Tail vs a wild with no bench
    bool strongWinds          = false; // Delta Stream (no Map::Weather value)
    bool playerEjectSwitch    = false;
    bool opponentEjectSwitch  = false;
    // Gen 7+: Pursuit only on voluntary switch / Baton Pass, not pivots or ejects.
    bool playerVoluntarySwitch   = false;
    bool opponentVoluntarySwitch = false;
    bool playerSwitchQueued      = false;
    bool playerItemQueued        = false;
    bool opponentActionLocked    = false;
    bool opponentSwitchDone      = false;
    bool opponentItemDone        = false;
    DelayedStrike playerDelayed;
    DelayedStrike opponentDelayed;
    int  playerEchoedVoice      = 0;
    int  opponentEchoedVoice    = 0;
    bool playerEchoedThisTurn   = false;
    bool opponentEchoedThisTurn = false;
    std::string fusionMoveId;
    int  battleTurn          = 0; // increments when a turn opens for commands
    bool playerFaintedThisTurn  = false;
    bool opponentFaintedThisTurn = false;
    bool playerRetaliate        = false; // a party member fainted on the previous turn
    bool opponentRetaliate      = false;
    bool pursuitStrike          = false;
    bool captureAtNight      = false;
    bool captureInCave       = false;
    bool captureUnderwater   = false;
    bool captureFishing      = false;
    BattleActions::Type             opponentAction              = BattleActions::Type::MOVES;
    BattleActions::Type             playerAction                = BattleActions::Type::MOVES;
    Pkmn::PkmnPtr                   newSelectedPkmn             = nullptr;
    Item::ItemPtr                   selectedItem                = nullptr;
    Pkmn::PkmnPtr                   itemTargetPkmn              = nullptr;
    bool                            itemUseResultUsed           = false;
    bool                            itemUseResultCaptureSuccess = false;
    bool                            itemUseResultEndsBattle     = false;
    std::set<Pkmn::PkmnPtr>         participatingPlayerPkmns;
    std::map<Pkmn::PkmnPtr, size_t> expGained;

    bool                     shouldBreakToEvolution  = false;
    bool                     shouldBreakToLevelUp    = false;
    bool                     shouldBreakToNewMove    = false;
    bool                     shouldGoToNewMovesScene = false;
    Pkmn::PkmnPtr            expPkmn                 = nullptr;
    size_t                   expFromBattle           = 0;
    PkmnDef::PkmnDefPtr      evolutionDef            = nullptr;
    std::vector<std::string> pendingMoveIds;
    std::string              moveToLearnId;
    std::set<std::string>    offeredMoveIds;

    virtual void chooseOpponentAction() = 0;
    void         chooseOpponentMove();

    // Air Lock and Cloud Nine suppress weather effects. The stored weather still ticks.
    Map::Weather fieldWeather() const
    {
        auto suppresses = [](BattleBattler const& battler) {
            return !battler.abilitySuppressed && (battler.abilityId == "AIRLOCK" || battler.abilityId == "CLOUDNINE");
        };
        if (suppresses(playerBattler) || suppresses(opponentBattler))
            return Map::Weather::NONE;
        return weather;
    }
};

std::string battleStateToString(BattleState state);

// Trick Room reverses. True when the foe's non-move should resolve before the player's.
bool opponentOutspeeds(SingleBattleSceneData const& data);

Move::MovePtr struggleMove();

#endif // SINGLEBATTLESCENEDATA_H
