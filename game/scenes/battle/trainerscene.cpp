#include "trainerscene.h"

#include "utils.h"

TrainerScene::TrainerScene(SDL_Renderer* renderer) : SingleBattleScene(renderer)
{
}

TrainerScene::~TrainerScene()
{
}

std::string TrainerScene::name()
{
    return "TrainerScene";
}

void TrainerScene::setOpponentTrainer(Trainer::TrainerPtr const& newOpponentTrainer)
{
    opponentTrainer = newOpponentTrainer;
    setOpponentPkmn(nextUsableOpponentPkmn());
}

void TrainerScene::init()
{
    if (!opponentPkmn)
        setOpponentPkmn(nextUsableOpponentPkmn());

    SingleBattleScene::init();

    if (!opponentPkmn)
    {
        endSpeech = std::make_unique<TextSpeech>(renderer);
        endSpeech->setTexts({lc::translate("The battle is over ! You won !")});
        endSpeech->start();
        state = EXPERIENCE;
    }
}

void TrainerScene::chooseOpponentAction()
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

std::string TrainerScene::encounterStartText() const
{
    if (opponentTrainer && !opponentTrainer->name.empty())
    {
        boost::format encounterText =
            boost::format(lc::translate("Trainer %1% wants to battle !")) % opponentTrainer->name;
        return encounterText.str();
    }

    return lc::translate("A trainer wants to battle !");
}

std::string TrainerScene::opponentMoveText(Move::MovePtr const& move) const
{
    boost::format moveText = boost::format(lc::translate("Foe %1% uses %2% !"))
                           % (opponentPkmn ? opponentPkmn->getDisplayName() : "#ERROR")
                           % (move && move->getDefinition() ? move->getDefinition()->getName() : "#ERROR");
    return moveText.str();
}

bool TrainerScene::canCaptureOpponent() const
{
    return false;
}

bool TrainerScene::canPlayerRun() const
{
    return false;
}

bool TrainerScene::tryPlayerRun()
{
    return false;
}

std::string TrainerScene::opponentRunText() const
{
    return lc::translate("The trainer withdrew !");
}

float TrainerScene::battleExperienceMultiplier() const
{
    return 1.5f;
}

void TrainerScene::onOpponentPkmnDefeated()
{
    shouldSwitchOpponentPkmn = nextUsableOpponentPkmn() != nullptr;
}

bool TrainerScene::onExperienceResolvedNextPkmn()
{
    if (!shouldSwitchOpponentPkmn)
        return false;

    auto nextPkmn            = nextUsableOpponentPkmn();
    shouldSwitchOpponentPkmn = false;
    if (!nextPkmn)
        return false;

    setOpponentPkmn(nextPkmn);
    encounterMove.reset();
    opponentMoveSpeech.reset();
    battleActions->reset();
    state = WEATHER;
    return true;
}

Pkmn::PkmnPtr TrainerScene::nextUsableOpponentPkmn() const
{
    if (!opponentTrainer)
        return nullptr;

    for (auto const& pkmn : opponentTrainer->pkmns)
    {
        if (pkmn && !pkmn->isKO() && pkmn != opponentPkmn)
            return pkmn;
    }

    return nullptr;
}