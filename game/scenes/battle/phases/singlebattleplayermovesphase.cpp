#include "singlebattleplayermovesphase.h"

#include "../singlebattlescene.h"
#include "game.h"
#include "textspeech.h"
#include "utils.h"

#include <boost/format.hpp>
#include <vector>

SingleBattlePlayerMovesPhase::SingleBattlePlayerMovesPhase(SingleBattleScene&                     scene,
                                                           std::shared_ptr<SingleBattleSceneData> data,
                                                           SDL_Renderer*                          renderer)
    : SingleBattlePhase(scene, data, renderer)
{
}

void SingleBattlePlayerMovesPhase::preupdate()
{
    if (!playerMoveSpeech && data->playerMove)
    {
        playerMoveSpeech       = std::make_unique<TextSpeech>(renderer);
        boost::format moveText = boost::format(lc::translate("%1% uses %2% !")) % data->playerPkmn->getDisplayName()
                               % data->playerMove->getDefinition()->getName();
        playerMoveSpeech->setTexts({moveText.str()});
        playerMoveSpeech->start();
    }
}

void SingleBattlePlayerMovesPhase::update(Inputs const* inputs)
{
    if (playerMoveSpeech)
    {
        playerMoveSpeech->update(inputs);
        if (playerMoveSpeech->shouldClose())
            playerMoveSpeech.release();
        return;
    }

    data->playerMove->decreasePP();
    // TODO: Move animation
    auto category = data->playerMove->getDefinition()->getCategory();
    if (category != MoveDef::STATUS)
    {
        // TODO: Damage animation
        data->opponentPkmn->decreaseHP(
            SingleBattleScene::computeDamage(data->playerPkmn, data->opponentPkmn, data->playerMove, data->weather));

        if (data->opponentPkmn->isKO())
        {
            auto hasExpShare = [](Pkmn::PkmnPtr const& pkmn) {
                if (!pkmn)
                    return false;
                auto heldItem = pkmn->getHeldItem();
                if (!heldItem || !heldItem->getDefinition())
                    return false;
                return heldItem->getDefinition()->getId() == "EXPSHARE";
            };

            // https://bulbapedia.bulbagarden.net/wiki/Experience#Experience_gain_in_battle
            float  a           = battleExperienceMultiplier();
            size_t b           = data->opponentPkmn->getDefinition()->getBaseExp();
            bool   hasLuckyEgg = data->playerPkmn->getHeldItem()
                              && data->playerPkmn->getHeldItem()->getDefinition()->getId() == "LUCKYEGG";
            float  e           = hasLuckyEgg ? 1.5 : 1.0;
            size_t L           = data->opponentPkmn->getLevel();
            float  t = data->opponentPkmn->getFirstTrainerId() == Game::instance()->data.player.id ? 1.5 : 1.0;

            std::vector<Pkmn::PkmnPtr> partyCopy;
            partyCopy.push_back(data->playerPkmn);
            for (auto const& pkmn : Game::instance()->data.player.pkmns)
            {
                if (pkmn && !pkmn->isKO() && pkmn != data->playerPkmn)
                    partyCopy.push_back(pkmn);
            }

            auto isParticipant = [this](Pkmn::PkmnPtr const& pkmn) {
                return data->participatingPlayerPkmns.count(pkmn) > 0;
            };

            size_t participatingAliveCount = 0;
            size_t expShareAliveCount      = 0;
            for (auto const& pkmn : partyCopy)
            {
                if (!pkmn || pkmn->isKO())
                    continue;
                if (isParticipant(pkmn))
                    ++participatingAliveCount;
                if (hasExpShare(pkmn))
                    ++expShareAliveCount;
            }

            bool  anyExpShare = expShareAliveCount > 0;
            float baseExp     = b * L / 7.0f * e * a * t;

            for (auto const& pkmn : partyCopy)
            {
                if (!pkmn || pkmn->isKO())
                    continue;

                bool  participated = isParticipant(pkmn);
                bool  expShare     = hasExpShare(pkmn);
                float gainedExp    = 0.0f;

                // S rule (Gen 3): without Exp. Share => participants/S.
                // With Exp. Share => participants get base/(2 * participants),
                // holders get base/(2 * holders).
                if (!anyExpShare)
                {
                    if (participated && participatingAliveCount > 0)
                        gainedExp = baseExp / participatingAliveCount;
                }
                else
                {
                    if (participated && participatingAliveCount > 0)
                        gainedExp += baseExp / (2.0f * participatingAliveCount);
                    if (expShare && expShareAliveCount > 0)
                        gainedExp += baseExp / (2.0f * expShareAliveCount);
                }

                size_t expFromBattle = static_cast<size_t>(gainedExp);
                if (expFromBattle > 0)
                    data->expGained[pkmn] += expFromBattle;
            }

            onOpponentPkmnDefeated();
            data->state = EXPERIENCE;
            return;
        }
    }

    if (data->playerFirst)
        data->state = OPPONENT_MOVES;
    else
        data->state = WEATHER;
}

void SingleBattlePlayerMovesPhase::draw(Fps const* fps, RenderSizes rs)
{
    if (playerMoveSpeech)
    {
        playerMoveSpeech->draw(fps, rs);
        return;
    }
}
