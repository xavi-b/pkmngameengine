#include "encounterscene.h"

#include "bagscene.h"
#include "game.h"
#include "pkmnsscene.h"
#include "utils.h"

EncounterScene::EncounterScene(SDL_Renderer* renderer) : BattleScene(renderer)
{
    pkmnEncounterSpeech = std::make_unique<TextSpeech>(renderer);
    firstPkmnSpeech     = std::make_unique<TextSpeech>(renderer);
}

EncounterScene::~EncounterScene()
{
}

void EncounterScene::init()
{
    BattleScene::init();

    boost::format whatShouldDoText =
        boost::format(lc::translate("What should %1% do ?")) % Game::instance()->data.player.name;
    battleSpeech->setTexts({whatShouldDoText.str()});
    battleSpeech->init();

    boost::format pkmnEncounterText = boost::format(lc::translate("A wild %1% appears !"))
                                    % (encounterPkmn ? encounterPkmn->getDisplayName() : "#ERROR");
    pkmnEncounterSpeech->setTexts({pkmnEncounterText.str()});
    pkmnEncounterSpeech->init();

    boost::format firstPkmnText =
        boost::format(lc::translate("%1% go !")) % (playerPkmn ? playerPkmn->getDisplayName() : "#ERROR");
    firstPkmnSpeech->setTexts({firstPkmnText.str()});
    firstPkmnSpeech->init();
}

void EncounterScene::draw(Fps const* fps, RenderSizes rs)
{
    battleBackground->draw(fps, rs);
    singleBattleUi->draw(fps, rs);

    BattleScene::draw(fps, rs);
}

std::string EncounterScene::name()
{
    return "EncounterScene";
}

void EncounterScene::setEncounterPkmn(Pkmn::PkmnPtr const& newEncounterPkmn)
{
    encounterPkmn = newEncounterPkmn;
    singleBattleUi->setFoePkmn(newEncounterPkmn);
}

void EncounterScene::setPlayerPkmn(Pkmn::PkmnPtr const& newPlayerPkmn)
{
    playerPkmn = newPlayerPkmn;
    singleBattleUi->setPlayerPkmn(newPlayerPkmn);
}

void EncounterScene::chooseOpponentAction()
{
    // TODO: randomize with weights ?
    // TODO: special cases like Latias/Latios
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
        state = OPPONENT_RUN;
        break;
    case BattleActions::MOVES:
    default: {
        auto const& moves              = encounterPkmn->getMoves();
        auto const  nonNullMovesResult = std::find(moves.begin(), moves.end(), nullptr);
        int const   nonNullMovesCount  = std::distance(moves.begin(), nonNullMovesResult);
        size_t      randomMove         = Utils::randint(0, nonNullMovesCount - 1);
        encounterMove                  = moves.at(randomMove);
        break;
    }
    }
}

void EncounterScene::update_START(Inputs const* inputs)
{
    // TODO: separate singlebattleui ? foe + player

    // TODO: Open animation
    // Eye opening animation
    // Pkmn + player entrance -> singlebattleui
    // Show pkmn databox -> singlebattleui

    if (pkmnEncounterSpeech)
    {
        pkmnEncounterSpeech->update(inputs);
        if (pkmnEncounterSpeech->shouldClose())
        {
            pkmnEncounterSpeech.release();
        }
        else
        {
            return;
        }
    }

    // TODO: Pkmn animation
    // Throw player pkmn -> singlebattleui
    // Show player databox -> singlebattleui

    if (firstPkmnSpeech)
    {
        firstPkmnSpeech->update(inputs);
        if (firstPkmnSpeech->shouldClose())
        {
            firstPkmnSpeech.release();
        }
        else
        {
            return;
        }
    }
}

void EncounterScene::draw_START(Fps const* fps, RenderSizes rs)
{
    // TODO: Open animation

    if (pkmnEncounterSpeech)
    {
        pkmnEncounterSpeech->draw(fps, rs);
        return;
    }

    // TODO: Pkmn animation

    if (firstPkmnSpeech)
    {
        firstPkmnSpeech->draw(fps, rs);
        return;
    }

    battleActions->reset();
    state = ACTIONS;
}

void EncounterScene::update_ACTIONS(Inputs const* inputs)
{
    // TODO: Weather animation
    // TODO: Weather text

    battleSpeech->update(inputs);
    if (battleSpeech->mayClose())
    {
        if (battleActions->isFinished())
        {
            switch (battleActions->selectedAction())
            {
            case BattleActions::MOVES: {
                moveSelection->reset();
                moveSelection->setPkmn(playerPkmn);
                state = MOVES;
                break;
            }
            case BattleActions::BAG:
                state = BAG;
                break;
            case BattleActions::PKMNS:
                state = PKMNS;
                break;
            case BattleActions::RUN:
                state = PLAYER_RUN;
                break;
            default:
                break;
            }
        }
        else
        {
            battleActions->update(inputs);
        }
    }
}

void EncounterScene::draw_ACTIONS(Fps const* fps, RenderSizes rs)
{
    // TODO: Weather animation
    // TODO: Weather text

    battleSpeech->draw(fps, rs);
    battleActions->draw(fps, rs);
}

void EncounterScene::update_MOVES(Inputs const* inputs)
{
    if (moveSelection->shouldQuit())
    {
        battleActions->reset();
        state = ACTIONS;
    }
    else if (moveSelection->isSelected())
    {
        playerMove = playerPkmn->getMoves().at(moveSelection->selectedIndex());
        if (playerMove && playerMove->getCurrentPP() > 0)
        {
            chooseOpponentAction();

            if (opponentAction == BattleActions::Type::MOVES)
            {
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
            }
            else
            {
                playerFirst = false;
            }

            if (playerFirst)
            {
                state = PLAYER_MOVES;
            }
            else
            {
                switch (opponentAction)
                {
                case BattleActions::BAG:
                    state = OPPONENT_ITEMS;
                    break;
                case BattleActions::PKMNS:
                    state = OPPONENT_PKMNS;
                    break;
                case BattleActions::RUN:
                    state = OPPONENT_RUN;
                    break;
                case BattleActions::MOVES:
                default:
                    state = OPPONENT_MOVES;
                    break;
                }
            }
        }
        else
        {
            // TODO: no PP left text
            moveSelection->reset();
        }
    }
    else
    {
        moveSelection->update(inputs);
    }
}

void EncounterScene::draw_MOVES(Fps const* fps, RenderSizes rs)
{
    moveSelection->draw(fps, rs);
}

void EncounterScene::update_P_MOVES(Inputs const* /*inputs*/)
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
                size_t expLeftToNextLevel = playerPkmn->expToNextLevel() - playerPkmn->getExp();
                if (expFromBattle > expLeftToNextLevel)
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

                    expFromBattle -= expLeftToNextLevel;
                }
                else
                {
                    playerPkmn->increaseExp(expFromBattle);
                    expFromBattle = 0;
                }
            }

            // TODO: Successful recap + xp
            state = END;
            return;
        }
    }

    if (playerFirst)
    {
        state = OPPONENT_MOVES;
    }
    else
    {
        battleActions->reset();
        state = ACTIONS;
    }
}

void EncounterScene::draw_P_MOVES(Fps const* /*fps*/, RenderSizes /*rs*/)
{
}

void EncounterScene::update_P_ITEMS(Inputs const* /*inputs*/)
{
    // Open BagScene

    bool selected = true;
    if (selected)
    {
        // Item computation
        // Item animation
        // If pkball
        //    Capture computation
        //    Pkdex scene/animation

        chooseOpponentAction();

        playerFirst = true;

        switch (opponentAction)
        {
        case BattleActions::BAG:
            state = OPPONENT_ITEMS;
            break;
        case BattleActions::PKMNS:
            state = OPPONENT_PKMNS;
            break;
        case BattleActions::RUN:
            state = OPPONENT_RUN;
            break;
        case BattleActions::MOVES:
        default:
            state = OPPONENT_MOVES;
            break;
        }
    }
}

void EncounterScene::draw_P_ITEMS(Fps const* /*fps*/, RenderSizes /*rs*/)
{
}

void EncounterScene::update_P_PKMNS(Inputs const* /*inputs*/)
{
    setPlayerPkmn(newSelectedPkmn);
    newSelectedPkmn.reset();

    // Pkmn computation
    // Pkmn animation
    // Pkmn text

    chooseOpponentAction();

    playerFirst = true;

    switch (opponentAction)
    {
    case BattleActions::BAG:
        state = OPPONENT_ITEMS;
        break;
    case BattleActions::PKMNS:
        state = OPPONENT_PKMNS;
        break;
    case BattleActions::RUN:
        state = OPPONENT_RUN;
        break;
    case BattleActions::MOVES:
    default:
        state = OPPONENT_MOVES;
        break;
    }
}

void EncounterScene::draw_P_PKMNS(Fps const* /*fps*/, RenderSizes /*rs*/)
{
}

void EncounterScene::update_P_RUN(Inputs const* inputs)
{
    if (runSpeech)
    {
        runSpeech->update(inputs);
        if (runSpeech->shouldClose())
        {
            runSpeech.release();
            state = END;
        }
        return;
    }

    if (failedRunSpeech)
    {
        failedRunSpeech->update(inputs);
        if (failedRunSpeech->shouldClose())
        {
            failedRunSpeech.release();

            chooseOpponentAction();

            playerFirst = true;

            switch (opponentAction)
            {
            case BattleActions::BAG:
                state = OPPONENT_ITEMS;
                break;
            case BattleActions::PKMNS:
                state = OPPONENT_PKMNS;
                break;
            case BattleActions::RUN:
                state = OPPONENT_RUN;
                break;
            case BattleActions::MOVES:
            default:
                state = OPPONENT_MOVES;
                break;
            }
        }
        return;
    }

    ++runAttemps;
    bool   run       = true;
    size_t wildSpeed = encounterPkmn->getStats()[PkmnDef::SPEED];

    if (wildSpeed > 0)
    {
        // https://bulbapedia.bulbagarden.net/wiki/Escape#Generation_III_and_IV
        size_t playerSpeed = playerPkmn->getStats()[PkmnDef::SPEED];
        size_t odds        = ((playerSpeed * 128 / wildSpeed) + 30 * runAttemps) % 256;
        size_t random      = Utils::randint(0, 255);
        run                = random < odds;
    }

    if (Game::instance()->isDebug())
        std::cout << __PRETTY_FUNCTION__ << " run: " << run << std::endl;

    if (run)
    {
        runSpeech = std::make_unique<TextSpeech>(renderer);
        runSpeech->setTexts({lc::translate("Got away safely !")});
        runSpeech->init();
    }
    else
    {
        failedRunSpeech = std::make_unique<TextSpeech>(renderer);
        failedRunSpeech->setTexts({lc::translate("Can't get away !")});
        failedRunSpeech->init();
    }
}

void EncounterScene::draw_P_RUN(Fps const* fps, RenderSizes rs)
{
    if (runSpeech)
    {
        runSpeech->draw(fps, rs);
        return;
    }

    if (failedRunSpeech)
    {
        failedRunSpeech->draw(fps, rs);
        return;
    }
}

void EncounterScene::update_O_PKMNS(Inputs const* /*inputs*/)
{
    // Should never reach
}

void EncounterScene::draw_O_PKMNS(Fps const* /*fps*/, RenderSizes /*rs*/)
{
    // Should never reach
}

void EncounterScene::update_O_ITEMS(Inputs const* /*inputs*/)
{
    // Attached berry
    // Item computation
    // Item animation

    battleActions->reset();
    state = ACTIONS;
}

void EncounterScene::draw_O_ITEMS(Fps const* /*fps*/, RenderSizes /*rs*/)
{
}

void EncounterScene::update_O_MOVES(Inputs const* /*inputs*/)
{
    if (Game::instance()->isDebug())
        std::cout << __PRETTY_FUNCTION__ << " Move: " << encounterMove->getDefinition()->getId() << std::endl;

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
    {
        battleActions->reset();
        state = ACTIONS;
    }
    else
        state = PLAYER_MOVES;
}

void EncounterScene::draw_O_MOVES(Fps const* /*fps*/, RenderSizes /*rs*/)
{
}

void EncounterScene::update_O_RUN(Inputs const* inputs)
{
    if (!runSpeech)
    {
        runSpeech             = std::make_unique<TextSpeech>(renderer);
        boost::format runText = boost::format(lc::translate("Wild %1% fled !"))
                              % (encounterPkmn ? encounterPkmn->getDisplayName() : "#ERROR");
        runSpeech->setTexts({runText.str()});
        runSpeech->init();
    }

    runSpeech->update(inputs);
    if (runSpeech->shouldClose())
    {
        runSpeech.release();
        state = END;
    }
}

void EncounterScene::draw_O_RUN(Fps const* fps, RenderSizes rs)
{
    if (runSpeech)
    {
        runSpeech->draw(fps, rs);
    }
}

void EncounterScene::update_END(Inputs const* /*inputs*/)
{
    if (!fadeOutAnimation->isStarted())
    {
        fadeOutAnimation->start();
    }
    else
    {
        fadeOutAnimation->incrementTicks();
    }
}

void EncounterScene::draw_END(Fps const* fps, RenderSizes rs)
{
    fadeOutAnimation->draw(fps, rs);
}

bool EncounterScene::pushScene() const
{
    return state == BAG || state == PKMNS;
}

void EncounterScene::popReset()
{
    switch (state)
    {
    case BAG: {
        // TODO: If selected
        // state = PLAYER_ITEMS;
        // TODO: If not selected
        battleActions->reset();
        state = ACTIONS;
        break;
    }
    case PKMNS: {
        if (!newSelectedPkmn || newSelectedPkmn == playerPkmn)
        {
            if (Game::instance()->isDebug())
                std::cout << __PRETTY_FUNCTION__ << " no new selected pkmn" << std::endl;

            newSelectedPkmn.reset();
            battleActions->reset();
            state = ACTIONS;
        }
        else
        {
            if (Game::instance()->isDebug())
                std::cout << __PRETTY_FUNCTION__ << " new selected pkmn: " << newSelectedPkmn->getDisplayName()
                          << std::endl;

            state = PLAYER_PKMNS;
        }
        break;
    }
    default:
        break;
    }
}

std::unique_ptr<Scene> EncounterScene::nextScene()
{
    switch (state)
    {
    case BAG:
        return std::make_unique<BagScene>(renderer);
    case PKMNS:
        newSelectedPkmn = playerPkmn;
        return std::make_unique<PkmnsScene>(renderer, newSelectedPkmn);
    default:
        return nullptr;
    }
}
