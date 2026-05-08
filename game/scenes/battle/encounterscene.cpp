#include "encounterscene.h"

#include "utils.h"

EncounterScene::EncounterScene(SDL_Renderer* renderer) : SingleBattleScene(renderer)
{
}

EncounterScene::~EncounterScene()
{
}

std::string EncounterScene::name()
{
    return "EncounterScene";
}

void EncounterScene::setEncounterPkmn(Pkmn::PkmnPtr const& newEncounterPkmn)
{
    setOpponentPkmn(newEncounterPkmn);
}

void EncounterScene::chooseOpponentAction()
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

std::string EncounterScene::encounterStartText() const
{
    boost::format pkmnEncounterText = boost::format(lc::translate("A wild %1% appears !"))
                                    % (opponentPkmn ? opponentPkmn->getDisplayName() : "#ERROR");
    return pkmnEncounterText.str();
}

std::string EncounterScene::opponentMoveText(Move::MovePtr const& move) const
{
    boost::format moveText = boost::format(lc::translate("Wild %1% uses %2% !"))
                           % (opponentPkmn ? opponentPkmn->getDisplayName() : "#ERROR")
                           % (move && move->getDefinition() ? move->getDefinition()->getName() : "#ERROR");
    return moveText.str();
}

std::string EncounterScene::opponentRunText() const
{
    boost::format runText =
        boost::format(lc::translate("Wild %1% fled !")) % (opponentPkmn ? opponentPkmn->getDisplayName() : "#ERROR");
    return runText.str();
}

bool EncounterScene::canCaptureOpponent() const
{
    return true;
}

bool EncounterScene::canPlayerRun() const
{
    return true;
}

bool EncounterScene::tryPlayerRun()
{
    ++runAttemps;
    bool   run       = true;
    size_t wildSpeed = opponentPkmn ? opponentPkmn->getStats()[PkmnDef::SPEED] : 0;

    if (wildSpeed > 0)
    {
        // https://bulbapedia.bulbagarden.net/wiki/Escape#Generation_III_and_IV
        size_t playerSpeed = playerPkmn ? playerPkmn->getStats()[PkmnDef::SPEED] : 0;
        size_t odds        = ((playerSpeed * 128 / wildSpeed) + 30 * runAttemps) % 256;
        size_t random      = Utils::randuint(0, 255);
        run                = random < odds;
    }

    return run;
}

float EncounterScene::battleExperienceMultiplier() const
{
    return 1.0f;
}

void EncounterScene::onOpponentPkmnDefeated()
{
}

bool EncounterScene::onExperienceResolvedNextPkmn()
{
    return true;
}