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
    defeatedOpponentPkmnName = opponentPkmn ? opponentPkmn->getDisplayName() : "";
    nextOpponentPkmn         = nextUsableOpponentPkmn();
    shouldSwitchOpponentPkmn = nextOpponentPkmn != nullptr;
    opponentSwitchAnnounced  = false;
}

bool TrainerScene::onExperienceResolvedNextPkmn()
{
    if (!shouldSwitchOpponentPkmn)
        return false;

    if (!opponentSwitchAnnounced)
    {
        if (!nextOpponentPkmn)
            return false;

        std::vector<std::string> texts;
        if (!defeatedOpponentPkmnName.empty())
        {
            boost::format faintText = boost::format(lc::translate("Foe %1% fainted !")) % defeatedOpponentPkmnName;
            texts.push_back(faintText.str());
        }

        if (opponentTrainer && !opponentTrainer->name.empty())
        {
            boost::format sendOutText =
                boost::format(lc::translate("Trainer %1% sent out %2% !")) % opponentTrainer->name
                % nextOpponentPkmn->getDisplayName();
            texts.push_back(sendOutText.str());
        }
        else
        {
            boost::format sendOutText =
                boost::format(lc::translate("Foe sent out %1% !")) % nextOpponentPkmn->getDisplayName();
            texts.push_back(sendOutText.str());
        }

        experienceSpeech = std::make_unique<TextSpeech>(renderer);
        experienceSpeech->setTexts(texts);
        experienceSpeech->start();
        opponentSwitchAnnounced = true;
        return true;
    }

    shouldSwitchOpponentPkmn = false;
    if (!nextOpponentPkmn)
        return false;

    setOpponentPkmn(nextOpponentPkmn);
    nextOpponentPkmn.reset();
    defeatedOpponentPkmnName.clear();
    opponentSwitchAnnounced = false;
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