#include "encounterscenedata.h"

void EncounterSceneData::chooseOpponentAction()
{
    // TODO: randomize with weights ?
    // TODO: special cases like Latias/Latios
    opponentAction = BattleActions::Type::MOVES;

    switch (opponentAction)
    {
    case BattleActions::RUN:
        state = OPPONENT_RUN;
        break;
    case BattleActions::BAG:
    case BattleActions::PKMNS:
        // Cannot happen for encounters
    case BattleActions::MOVES:
    default: {
        chooseOpponentMove();
        break;
    }
    }
}