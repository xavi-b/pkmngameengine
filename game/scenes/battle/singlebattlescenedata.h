#ifndef SINGLEBATTLESCENEDATA_H
#define SINGLEBATTLESCENEDATA_H

#include "battles/battleactions.h"
#include "item.h"
#include "map.h"
#include "pkmn.h"

#include <map>
#include <set>
#include <string>

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
    bool          playerFirst   = true;

    BattleActions::Type             opponentAction              = BattleActions::Type::MOVES;
    Pkmn::PkmnPtr                   newSelectedPkmn             = nullptr;
    Item::ItemPtr                   selectedItem                = nullptr;
    Pkmn::PkmnPtr                   itemTargetPkmn              = nullptr;
    bool                            itemUseResultUsed           = false;
    bool                            itemUseResultCaptureSuccess = false;
    std::set<Pkmn::PkmnPtr>         participatingPlayerPkmns;
    std::map<Pkmn::PkmnPtr, size_t> expGained;

    bool                shouldBreakToEvolution  = false;
    bool                shouldBreakToLevelUp    = false;
    bool                shouldBreakToNewMove    = false;
    bool                shouldGoToNewMovesScene = false;
    Pkmn::PkmnPtr       expPkmn                 = nullptr;
    size_t              expFromBattle           = 0;
    PkmnDef::PkmnDefPtr evolutionDef            = nullptr;

    virtual void chooseOpponentAction() = 0;
    void         chooseOpponentMove();
};

std::string battleStateToString(BattleState state);

#endif // SINGLEBATTLESCENEDATA_H
