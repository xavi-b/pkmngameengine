#include "trainerscenedata.h"

void TrainerSceneData::chooseOpponentAction()
{
    // TODO: randomize with weights ?

    opponentAction = BattleActions::Type::MOVES;

    switch (opponentAction)
    {
    case BattleActions::BAG:
        state = OPPONENT_ITEMS;
        break;
    case BattleActions::PKMNS:
        state = OPPONENT_PKMNS;
        break;
    case BattleActions::RUN:
        // Cannot happen for trainers
    case BattleActions::MOVES:
    default: {
        chooseOpponentMove();
        break;
    }
    }
}