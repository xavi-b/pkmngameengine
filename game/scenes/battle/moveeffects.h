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

bool isTwoTurnChargingMove(MoveDef::FunctionCode code);
bool isRechargeMove(MoveDef::FunctionCode code);

#endif // MOVEEFFECTS_H
