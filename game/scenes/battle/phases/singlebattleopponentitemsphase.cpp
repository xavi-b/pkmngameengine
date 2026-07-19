#include "singlebattleopponentitemsphase.h"

#include "../singlebattlescene.h"
#include "itemutils.h"
#include "textspeech.h"

#include <boost/format.hpp>
#include <boost/locale.hpp>

namespace lc = boost::locale;

SingleBattleOpponentItemsPhase::SingleBattleOpponentItemsPhase(SingleBattleScene&                     scene,
                                                               std::shared_ptr<SingleBattleSceneData> data,
                                                               SDL_Renderer*                          renderer)
    : SingleBattlePhase(scene, data, renderer)
{
}

void SingleBattleOpponentItemsPhase::preupdate()
{
    speech.release();
    // Held berries / leftovers are handled in weather residuals.
    // Trainers may also "use" a held status berry mid-turn when afflicted — apply once here.
    if (data->opponentPkmn && !data->opponentPkmn->isKO())
    {
        auto tick = ItemUtils::tickHeldItem(data->opponentPkmn, &data->opponentBattler, &data->opponentConfusion);
        if (!tick.message.empty())
        {
            speech = std::make_unique<TextSpeech>(renderer);
            speech->setTexts({tick.message});
            speech->start();
            return;
        }
    }
}

void SingleBattleOpponentItemsPhase::update(Inputs const* inputs)
{
    if (speech)
    {
        speech->update(inputs);
        if (!speech->shouldClose())
            return;
        speech.release();
    }

    if (data->playerFirst)
        data->state = WEATHER;
    else
        data->state = PLAYER_MOVES;
}

void SingleBattleOpponentItemsPhase::draw(Fps const* fps, RenderSizes rs)
{
    if (speech)
        speech->draw(fps, rs);
}
