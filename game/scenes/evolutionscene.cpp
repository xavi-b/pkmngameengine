#include "evolutionscene.h"

EvolutionScene::EvolutionScene(SDL_Renderer*              renderer,
                               Pkmn::PkmnPtr const&       pkmn,
                               PkmnDef::PkmnDefPtr const& evolutionDef)
    : Scene(renderer), pkmn(pkmn), evolutionDef(evolutionDef)
{
    speech = std::make_unique<TextSpeech>(renderer);
}

EvolutionScene::~EvolutionScene()
{
}

void EvolutionScene::init()
{
    std::string pkmnName = pkmn ? pkmn->getDisplayName() : "#ERROR";
    speech->setTexts({pkmnName + " is evolving !", "Press A to confirm evolution.", "Press B to cancel evolution."});
    speech->start();
}

void EvolutionScene::update(Inputs const* inputs)
{
    speech->update(inputs);

    if (inputs->A)
    {
        pkmn->setDefinition(evolutionDef);
        leave = true;
    }
    else if (inputs->B)
    {
        leave = true;
    }
}

void EvolutionScene::draw(Fps const* fps, RenderSizes rs)
{
    speech->draw(fps, rs);
}

bool EvolutionScene::popScene() const
{
    return leave;
}

std::string EvolutionScene::name()
{
    return "EvolutionScene";
}
