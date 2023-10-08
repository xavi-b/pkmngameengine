#include "encounterscene.h"

#include "game.h"
#include "utils.h"

EncounterScene::EncounterScene(SDL_Renderer* renderer) : BattleScene(renderer)
{
}

EncounterScene::~EncounterScene()
{
}

void EncounterScene::draw(Fps const* fps, RenderSizes rs)
{
    battleBackground->draw(fps, rs);

    BattleScene::draw(fps, rs);
}

std::string EncounterScene::name()
{
    return "EncounterScene";
}

void EncounterScene::setEncounterPkmn(Pkmn::PkmnPtr const& newEncounterPkmn)
{
    encounterPkmn = newEncounterPkmn;
}

void EncounterScene::setPlayerPkmn(Pkmn::PkmnPtr const& newPlayerPkmn)
{
    playerPkmn = newPlayerPkmn;
}

void EncounterScene::update_START(Inputs const* inputs)
{
    // Open animation
    // Open text
    // Pkmn animation
    // Pkmn text
    battleSpeech->update(inputs);
    if (battleSpeech->mayClose())
        state = ACTIONS;
}

void EncounterScene::draw_START(Fps const* fps, RenderSizes rs)
{
    battleSpeech->draw(fps, rs);
}

void EncounterScene::update_ACTIONS(Inputs const* inputs)
{
    battleActions->update(inputs);

    if (battleActions->isFinished())
    {
        switch (battleActions->selectedAction())
        {
        case BattleActions::MOVES: {
            moveSelection->update(inputs);
            if (moveSelection->shouldQuit())
            {
                state = ACTIONS;
            }
            else if (moveSelection->isSelected())
            {
                playerMove = playerPkmn->getMoves().at(moveSelection->selectedIndex());
                if (playerMove && playerMove->getCurrentPP() > 0)
                {
                    moveSelection->reset();

                    auto const&                tempMoves = encounterPkmn->getMoves();
                    std::vector<Move::MovePtr> moves;
                    std::copy_if(tempMoves.cbegin(), tempMoves.cend(), moves.begin(), [=](auto const& e) {
                        return e && e->getCurrentPP() > 0;
                    });
                    size_t randomMove = Utils::randint(0, moves.size() - 1);
                    encounterMove     = moves.at(randomMove);

                    if (!encounterMove)
                        playerFirst = true;
                    else if (playerMove->getDefinition()->getPriority() > encounterMove->getDefinition()->getPriority())
                        playerFirst = true;
                    else if (playerMove->getDefinition()->getPriority() < encounterMove->getDefinition()->getPriority())
                        playerFirst = false;
                    else
                    {
                        size_t playerSpeed    = playerPkmn->getStats()[PkmnDef::SPEED];
                        size_t encounterSpeed = encounterPkmn->getStats()[PkmnDef::SPEED];
                        if (playerSpeed > encounterSpeed)
                            playerFirst = true;
                        else if (playerSpeed > encounterSpeed)
                            playerFirst = false;
                        else
                            playerFirst = Utils::randint(0, 1);
                    }

                    if (playerFirst)
                        state = P_MOVES;
                    else
                        state = O_MOVES;
                }
            }
            break;
        }
        case BattleActions::BAG:
            state = BAG;
            break;
        case BattleActions::PKMNS:
            state = PKMNS;
            break;
        case BattleActions::RUN:
            state = P_RUN;
            break;
        default:
            break;
        }
        battleActions->reset();
    }
}

void EncounterScene::draw_ACTIONS(Fps const* fps, RenderSizes rs)
{
    battleActions->draw(fps, rs);
}

void EncounterScene::update_P_MOVES(Inputs const* /*inputs*/)
{
    if (playerMove && playerMove->getCurrentPP() > 0)
    {
        playerMove->decreasePP();
        // Move animation
        auto category = playerMove->getDefinition()->getCategory();
        if (category != MoveDef::STATUS)
        {
            // Damage computation
            // Damage animation
            encounterPkmn->decreaseHP(computeDamage(playerPkmn, playerMove));

            if (encounterPkmn->isKO())
            {
                // https://bulbapedia.bulbagarden.net/wiki/Experience#Experience_gain_in_battle
                float  a = 1.5;
                size_t b = encounterPkmn->getDefinition()->getBaseExp();
                // TODO
                float  e = 1.5;
                size_t L = encounterPkmn->getLevel();
                // TODO
                float s = 1.0;
                // TODO
                float  t             = 1.0;
                size_t expFromBattle = b * L / 7 * 1 / s * e * a * t;
                while (expFromBattle > 0)
                {
                    size_t exp = playerPkmn->expToNextLevel();
                    if (expFromBattle > exp)
                    {
                        playerPkmn->incrementLevel();
                        std::string evolutionId = canEvolve(playerPkmn);
                        if (!evolutionId.empty())
                        {
                            PkmnDef::PkmnDefPtr evolutionDef = Game::instance()->data.pkmnDefFor(evolutionId);
                            if (evolutionDef)
                            {
                                // TODO
                                // evolution (scene + can be cancelled)
                                bool success = true;
                                if (success)
                                {
                                    playerPkmn->setDefinition(evolutionDef);
                                }
                            }
                        }

                        // TODO
                        // new move

                        expFromBattle -= exp;
                    }
                    else
                    {
                        playerPkmn->increaseExp(exp);
                    }
                }
                state = END;
                return;
            }
        }

        if (playerFirst)
            state = ACTIONS;
        else
            state = P_MOVES;
    }
}

void EncounterScene::draw_P_MOVES(Fps const* fps, RenderSizes rs)
{
    moveSelection->draw(fps, rs);
}

void EncounterScene::update_P_ITEMS(Inputs const* /*inputs*/)
{
    // Open BagScene
    // Item computation
    // If pkball
    //    Capture computation
    //    Pkdex scene/animation
    state = O_MOVES;
}

void EncounterScene::draw_P_ITEMS(Fps const* /*fps*/, RenderSizes /*rs*/)
{
}

void EncounterScene::update_P_PKMNS(Inputs const* /*inputs*/)
{
    // Pkmn animation
    // Pkmn text
    state = O_MOVES;
}

void EncounterScene::draw_P_PKMNS(Fps const* /*fps*/, RenderSizes /*rs*/)
{
}

void EncounterScene::update_P_RUN(Inputs const* /*inputs*/)
{
    ++runAttemps;
    // https://bulbapedia.bulbagarden.net/wiki/Escape#Generation_III_and_IV
    size_t playerSpeed = playerPkmn->getStats()[PkmnDef::SPEED];
    size_t wildSpeed   = encounterPkmn->getStats()[PkmnDef::SPEED];
    size_t odds        = ((playerSpeed * 128 / wildSpeed) + 30 * runAttemps) % 256;
    size_t random      = Utils::randint(0, 255);
    bool   run         = random < odds;
    if (run)
    {
        // Succesful run text
        state = END;
    }
    else
    {
        // Failed run text
        state = ACTIONS;
    }
}

void EncounterScene::draw_P_RUN(Fps const* /*fps*/, RenderSizes /*rs*/)
{
}

void EncounterScene::update_O_PKMNS(Inputs const* /*inputs*/)
{
}

void EncounterScene::draw_O_PKMNS(Fps const* /*fps*/, RenderSizes /*rs*/)
{
}

void EncounterScene::update_O_ITEMS(Inputs const* /*inputs*/)
{
}

void EncounterScene::draw_O_ITEMS(Fps const* /*fps*/, RenderSizes /*rs*/)
{
}

void EncounterScene::update_O_MOVES(Inputs const* /*inputs*/)
{
    if (encounterMove && encounterMove->getCurrentPP() > 0)
    {
        encounterMove->decreasePP();
        // Move animation
        auto category = encounterMove->getDefinition()->getCategory();
        if (category != MoveDef::STATUS)
        {
            // Damage computation
            // Damage animation
            playerPkmn->decreaseHP(computeDamage(encounterPkmn, encounterMove));

            if (playerPkmn->isKO())
            {
                // TODO state PKMNS if at least one pkmn not KO
                state = END;
                return;
            }
        }

        if (playerFirst)
            state = ACTIONS;
        else
            state = P_MOVES;
    }
}

void EncounterScene::draw_O_MOVES(Fps const* /*fps*/, RenderSizes /*rs*/)
{
}
