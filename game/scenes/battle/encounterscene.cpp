#include "encounterscene.h"

#include "encounterscenedata.h"
#include "phases/encounter/encounterexperiencephase.h"
#include "phases/encounter/encounteropponentmovesphase.h"
#include "phases/encounter/encounteropponentrunphase.h"
#include "phases/encounter/encounterplayeritemsphase.h"
#include "phases/encounter/encounterplayermovesphase.h"
#include "phases/encounter/encounterplayerrunphase.h"
#include "utils.h"

EncounterScene::EncounterScene(SDL_Renderer* renderer) : SingleBattleScene(renderer)
{
    data = std::make_shared<EncounterSceneData>();
    initPhases();
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

std::string EncounterScene::encounterStartText() const
{
    boost::format pkmnEncounterText = boost::format(lc::translate("A wild %1% appears !"))
                                    % (data->opponentPkmn ? data->opponentPkmn->getDisplayName() : "#ERROR");
    return pkmnEncounterText.str();
}

void EncounterScene::initPhases()
{
    SingleBattleScene::initPhases();

    playerRunPhase     = std::make_unique<EncounterPlayerRunPhase>(*this, data, renderer);
    playerItemsPhase   = std::make_unique<EncounterPlayerItemsPhase>(*this, data, renderer);
    playerMovesPhase   = std::make_unique<EncounterPlayerMovesPhase>(*this, data, renderer);
    opponentMovesPhase = std::make_unique<EncounterOpponentMovesPhase>(*this, data, renderer);
    opponentRunPhase   = std::make_unique<EncounterOpponentRunPhase>(*this, data, renderer);
    experiencePhase    = std::make_unique<EncounterExperiencePhase>(*this, data, renderer);
}
