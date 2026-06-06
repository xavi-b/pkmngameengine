#include "trainerscene.h"

#include "phases/trainer/trainerexperiencephase.h"
#include "phases/trainer/traineropponentmovesphase.h"
#include "phases/trainer/traineropponentrunphase.h"
#include "phases/trainer/trainerplayeritemsphase.h"
#include "phases/trainer/trainerplayermovesphase.h"
#include "phases/trainer/trainerplayerrunphase.h"
#include "utils.h"

TrainerScene::TrainerScene(SDL_Renderer* renderer) : SingleBattleScene(renderer)
{
    data = std::make_shared<TrainerSceneData>();
    initPhases();
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
    auto trainerData             = std::static_pointer_cast<TrainerSceneData>(data);
    trainerData->opponentTrainer = newOpponentTrainer;
    setOpponentPkmn(nextUsableOpponentPkmn());
}

void TrainerScene::init()
{
    if (!data->opponentPkmn)
        setOpponentPkmn(nextUsableOpponentPkmn());

    SingleBattleScene::init();

    if (!data->opponentPkmn)
    {
        experiencePhase->showEndSpeech(lc::translate("The battle is over ! You won !"));
        data->state = EXPERIENCE;
    }
}

std::string TrainerScene::encounterStartText() const
{
    auto trainerData = std::static_pointer_cast<TrainerSceneData>(data);

    if (trainerData->opponentTrainer && !trainerData->opponentTrainer->name.empty())
    {
        boost::format encounterText =
            boost::format(lc::translate("Trainer %1% wants to battle !")) % trainerData->opponentTrainer->name;
        return encounterText.str();
    }

    return lc::translate("A trainer wants to battle !");
}

void TrainerScene::initPhases()
{
    SingleBattleScene::initPhases();

    playerRunPhase     = std::make_unique<TrainerPlayerRunPhase>(*this, data, renderer);
    playerItemsPhase   = std::make_unique<TrainerPlayerItemsPhase>(*this, data, renderer);
    playerMovesPhase   = std::make_unique<TrainerPlayerMovesPhase>(*this, data, renderer);
    opponentMovesPhase = std::make_unique<TrainerOpponentMovesPhase>(*this, data, renderer);
    opponentRunPhase   = std::make_unique<TrainerOpponentRunPhase>(*this, data, renderer);
    experiencePhase    = std::make_unique<TrainerExperiencePhase>(*this, data, renderer);
}

Pkmn::PkmnPtr TrainerScene::nextUsableOpponentPkmn() const
{
    auto trainerData = std::static_pointer_cast<TrainerSceneData>(data);

    if (!trainerData->opponentTrainer)
        return nullptr;

    for (auto const& pkmn : trainerData->opponentTrainer->pkmns)
    {
        if (pkmn && !pkmn->isKO() && pkmn != data->opponentPkmn)
            return pkmn;
    }

    return nullptr;
}
