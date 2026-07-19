#ifndef SINGLEBATTLESCENEDATA_H
#define SINGLEBATTLESCENEDATA_H

#include "battles/battleactions.h"
#include "battlebattler.h"
#include "confusionstatus.h"
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

    ConfusionStatus playerConfusion;
    ConfusionStatus opponentConfusion;

    BattleBattler playerBattler;
    BattleBattler opponentBattler;

    // Side field effects
    int  playerSpikes      = 0;
    int  opponentSpikes    = 0;
    int  playerToxicSpikes = 0;
    int  opponentToxicSpikes = 0;
    bool playerStealthRock   = false;
    bool opponentStealthRock = false;
    bool playerStickyWeb     = false;
    bool opponentStickyWeb   = false;
    bool trickRoom           = false;
    int  trickRoomTurns       = 0;
    bool gravity             = false;
    int  gravityTurns        = 0;
    bool wonderRoom          = false;
    bool magicRoom           = false;
    int  terrainTurns        = 0; // 0 = none; positive remaining
    enum class Terrain
    {
        None,
        Electric,
        Grassy,
        Misty,
        Psychic
    } terrain = Terrain::None;

    BattleActions::Type             opponentAction              = BattleActions::Type::MOVES;
    Pkmn::PkmnPtr                   newSelectedPkmn             = nullptr;
    Item::ItemPtr                   selectedItem                = nullptr;
    Pkmn::PkmnPtr                   itemTargetPkmn              = nullptr;
    bool                            itemUseResultUsed           = false;
    bool                            itemUseResultCaptureSuccess = false;
    bool                            itemUseResultEndsBattle     = false;
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
