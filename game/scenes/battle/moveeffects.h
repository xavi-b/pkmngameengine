#ifndef MOVEEFFECTS_H
#define MOVEEFFECTS_H

#include "battlebattler.h"
#include "movedef.h"
#include "move.h"
#include "pkmn.h"
#include "singlebattlescenedata.h"

#include <string>
#include <vector>

struct MoveEffectContext
{
    SingleBattleSceneData&   data;
    bool                     playerIsUser;
    Pkmn::PkmnPtr            user;
    Pkmn::PkmnPtr            target;
    BattleBattler&           userBattler;
    BattleBattler&           targetBattler;
    Move::MovePtr            move;
    MoveDef::MoveDefPtr      def;
    size_t                   damageDealt = 0;
    bool                     moveHit     = false;
    std::vector<std::string> messages;
    bool                     shouldSwitchUser   = false;
    bool                     shouldSwitchTarget = false;
    bool                     forcedSwitchTarget = false;
    bool                     forcedSwitchUser   = false;
    int                      callDepth          = 0;
    std::string              typeOverride;
    bool                     sheerForce         = false;
    bool                     statRose           = false;
    bool                     statRoseOnTarget   = false;
    bool                     healed             = false;
    bool                     hitSubstitute      = false;
    bool                     immune             = false;
    bool                     stagesStolen       = false;
    // Sleep Talk / Power Herb / sun: skip the charge turn and finish in one action.
    bool                     instantTwoTurn     = false;

    MoveEffectContext(SingleBattleSceneData&  dataIn,
                      bool                    playerIsUserIn,
                      Pkmn::PkmnPtr           userIn,
                      Pkmn::PkmnPtr           targetIn,
                      BattleBattler&          userBattlerIn,
                      BattleBattler&          targetBattlerIn,
                      Move::MovePtr           moveIn,
                      MoveDef::MoveDefPtr     defIn)
        : data(dataIn)
        , playerIsUser(playerIsUserIn)
        , user(std::move(userIn))
        , target(std::move(targetIn))
        , userBattler(userBattlerIn)
        , targetBattler(targetBattlerIn)
        , move(std::move(moveIn))
        , def(std::move(defIn))
    {
    }
};

// Adjust base power before damage for FunctionCodes that modify power.
size_t moveEffectPower(MoveEffectContext& ctx, size_t basePower);

// Apply post-hit / status FunctionCode effects. Returns false if the move failed its effect.
bool applyMoveFunctionCode(MoveEffectContext& ctx);

// Hit count for multi-hit moves (1 = single hit).
int moveHitCount(MoveDef::MoveDefPtr const& def);

int movePriority(MoveDef::MoveDefPtr const& def, Pkmn::PkmnPtr const& user, BattleBattler const& battler,
                 SingleBattleSceneData const& data);

bool grounded(Pkmn::PkmnPtr const& pkmn, BattleBattler const& battler, SingleBattleSceneData const& data);

struct DelayedStrikeResult
{
    bool hitsPlayer = false;
    bool fainted    = false;
    std::vector<std::string> messages;
};

// Rolls Future Sight or Doom Desire when the counter reaches 0. A direct HP loss.
DelayedStrikeResult resolveDelayedStrike(SingleBattleSceneData& data, DelayedStrike& slot);

// Natural Gift, Spit Up, and a rampage commit when the move is executed, including a miss.
void noteMoveCommit(MoveEffectContext& ctx);

// Misses, Focus Punch, terrain, and the other "before the hit" failures. Messages land on ctx.
bool moveBlockedBeforeHit(MoveEffectContext& ctx);

bool isTwoTurnChargingMove(MoveDef::FunctionCode code);
bool isRechargeMove(MoveDef::FunctionCode code);

// Gravity greys/blocks these FunctionCodes (Splash, Magnet Rise, Fly, …).
bool gravityBlocksMove(MoveDef::FunctionCode code);

// Arms Beak Blast / Shell Trap when the command is locked for the turn.
void armSelectSetupMove(BattleBattler& battler, Move::MovePtr const& move);

// Turn-start Focus Punch / Beak Blast / Shell Trap lines.
void appendSelectAnnounce(std::vector<std::string>& messages, Pkmn::PkmnPtr const& pkmn, Move::MovePtr const& move);

// Ends Encore when the locked move has no PP left.
void clearEncoreIfMoveDepleted(Pkmn::PkmnPtr const& pkmn, BattleBattler& battler);

// Clears a Sky Drop lock held by this side.
void releaseSkyDrop(SingleBattleSceneData& data, bool carrierIsPlayer);

// A prevented second turn (sleep, freeze, flinch, confusion self-hit) ends the charge.
void cancelTwoTurnCharge(SingleBattleSceneData& data, bool playerSide);

void armGulpMissile(MoveEffectContext& ctx);
void spitGulpMissile(MoveEffectContext& ctx);
void rollHitFlinch(MoveEffectContext& ctx);

struct StatChangeResult
{
    bool changed = false;
    int  applied = 0;
    bool raised  = false;
    bool ejected = false;
};

StatChangeResult applyStatChange(SingleBattleSceneData& data, BattleBattler& battler, Pkmn::PkmnPtr const& pkmn,
                                 BattleBattler::StageStat stat, int delta, std::vector<std::string>& messages,
                                 bool fromOpponent, bool pierceAbilities = false, bool checkMist = true,
                                 bool announce = true, bool reflected = false);

// Copies the target's types, ability, stages, stat overrides, and moves at 5 PP.
void copyTransformFrom(Pkmn::PkmnPtr const& user,
                       BattleBattler&       userBattler,
                       Pkmn::PkmnPtr const& target,
                       BattleBattler const& targetBattler);

#endif // MOVEEFFECTS_H
