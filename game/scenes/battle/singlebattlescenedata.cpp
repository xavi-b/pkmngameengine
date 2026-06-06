#include "singlebattlescenedata.h"

#include "utils.h"

std::string battleStateToString(BattleState state)
{
    switch (state)
    {
    case START:
        return "START";
    case WEATHER:
        return "WEATHER";
    case ACTIONS:
        return "ACTIONS";
    case MOVES:
        return "MOVES";
    case BAG:
        return "BAG";
    case PKMNS:
        return "PKMNS";
    case PLAYER_MOVES:
        return "PLAYER_MOVES";
    case PLAYER_ITEMS:
        return "PLAYER_ITEMS";
    case PLAYER_PKMNS:
        return "PLAYER_PKMNS";
    case PLAYER_RUN:
        return "PLAYER_RUN";
    case OPPONENT_PKMNS:
        return "OPPONENT_PKMNS";
    case OPPONENT_ITEMS:
        return "OPPONENT_ITEMS";
    case OPPONENT_MOVES:
        return "OPPONENT_MOVES";
    case OPPONENT_RUN:
        return "OPPONENT_RUN";
    case EXPERIENCE:
        return "EXPERIENCE";
    case END:
        return "END";
    }
    return "???";
}

void SingleBattleSceneData::chooseOpponentMove()
{
    auto const& moves              = opponentPkmn->getMoves();
    auto const  nonNullMovesResult = std::find(moves.begin(), moves.end(), nullptr);
    int const   nonNullMovesCount  = std::distance(moves.begin(), nonNullMovesResult);
    if (nonNullMovesCount > 0)
    {
        size_t randomMove = Utils::randuint(0, nonNullMovesCount - 1);
        encounterMove     = moves.at(randomMove);
    }
    else
    {
        encounterMove = nullptr;
    }
}