#include "encounterscene.h"

#include "game.h"
#include "itemutils.h"
#include "scenes/bagscene.h"
#include "scenes/evolutionscene.h"
#include "scenes/pkmnsscene.h"
#include "utils.h"

#include <cctype>

SingleBattleScene::SingleBattleScene(SDL_Renderer* renderer) : BattleScene(renderer)
{
    pkmnEncounterSpeech = std::make_unique<TextSpeech>(renderer);
    firstPkmnSpeech     = std::make_unique<TextSpeech>(renderer);
}

SingleBattleScene::~SingleBattleScene()
{
}

void SingleBattleScene::init()
{
    BattleScene::init();
    participatingPlayerPkmns.clear();
    if (playerPkmn)
        participatingPlayerPkmns.insert(playerPkmn);

    boost::format whatShouldDoText =
        boost::format(lc::translate("What should %1% do ?")) % Game::instance()->data.player.name;
    battleSpeech->setTexts({whatShouldDoText.str()});
    battleSpeech->init();

    boost::format pkmnEncounterText = boost::format(lc::translate("A wild %1% appears !"))
                                    % (encounterPkmn ? encounterPkmn->getDisplayName() : "#ERROR");
    pkmnEncounterSpeech->setTexts({pkmnEncounterText.str()});
    pkmnEncounterSpeech->start();

    boost::format firstPkmnText =
        boost::format(lc::translate("%1% go !")) % (playerPkmn ? playerPkmn->getDisplayName() : "#ERROR");
    firstPkmnSpeech->setTexts({firstPkmnText.str()});
    firstPkmnSpeech->start();
}

void SingleBattleScene::draw(Fps const* fps, RenderSizes rs)
{
    battleBackground->draw(fps, rs);
    singleBattleUi->draw(fps, rs);

    BattleScene::draw(fps, rs);
}

std::string SingleBattleScene::name()
{
    return "SingleBattleScene";
}

void SingleBattleScene::setEncounterPkmn(Pkmn::PkmnPtr const& newEncounterPkmn)
{
    encounterPkmn = newEncounterPkmn;
    singleBattleUi->setFoePkmn(newEncounterPkmn);
}

void SingleBattleScene::setPlayerPkmn(Pkmn::PkmnPtr const& newPlayerPkmn)
{
    playerPkmn = newPlayerPkmn;
    singleBattleUi->setPlayerPkmn(newPlayerPkmn);
    if (newPlayerPkmn)
        participatingPlayerPkmns.insert(newPlayerPkmn);
}

void SingleBattleScene::chooseOpponentAction()
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
        if (nonNullMovesCount > 0)
        {
            size_t randomMove      = Utils::randuint(0, nonNullMovesCount - 1);
            encounterMove          = moves.at(randomMove);
            opponentMoveSpeech     = std::make_unique<TextSpeech>(renderer);
            boost::format moveText = boost::format(lc::translate("Wild %1% uses %2% !"))
                                   % encounterPkmn->getDisplayName() % encounterMove->getDefinition()->getName();
            opponentMoveSpeech->setTexts({moveText.str()});
            opponentMoveSpeech->start();
        }
        else
        {
            encounterMove = nullptr;
        }
        break;
    }
    }
}

void SingleBattleScene::update_START(Inputs const* inputs)
{
    if (!eyeAnimation->isFinished())
    {
        eyeAnimation->incrementTicks();
    }

    // TODO: separate singlebattleui ? foe + player

    // TODO: Open animation
    // Pkmn + player entrance -> singlebattleui
    // Show pkmn databox -> singlebattleui

    if (pkmnEncounterSpeech)
    {
        pkmnEncounterSpeech->update(inputs);
        if (pkmnEncounterSpeech->shouldClose())
        {
            pkmnEncounterSpeech.release();
        }
    }

    // TODO: Pkmn animation
    // Throw player pkmn -> singlebattleui
    // Show player databox -> singlebattleui

    else if (firstPkmnSpeech)
    {
        firstPkmnSpeech->update(inputs);
        if (firstPkmnSpeech->shouldClose())
        {
            firstPkmnSpeech.release();

            state = WEATHER;
        }
    }
}

void SingleBattleScene::draw_START(Fps const* fps, RenderSizes rs)
{
    // TODO: Open animation

    if (pkmnEncounterSpeech)
    {
        pkmnEncounterSpeech->draw(fps, rs);
    }

    // TODO: Pkmn animation

    else if (firstPkmnSpeech)
    {
        firstPkmnSpeech->draw(fps, rs);
    }

    if (eyeAnimation->isRunning())
    {
        eyeAnimation->draw(fps, rs);
    }
}

void SingleBattleScene::update_WEATHER(Inputs const* inputs)
{
    if (weather == Map::Weather::NONE)
    {
        battleActions->reset();
        state = ACTIONS;
    }

    if (weatherAnimation)
    {
        if (!weatherAnimation->isStarted())
        {
            weatherAnimation->start();
        }

        if (weatherAnimation->isRunning())
        {
            weatherAnimation->incrementTicks();
        }

        weatherSpeech->update(inputs);

        if (weatherSpeech->shouldClose())
        {
            weatherSpeech->setTexts({weatherAnimation->getContinuingText()});
            weatherSpeech->reset();
            weatherSpeech->start();
            weatherAnimation->reset();

            battleActions->reset();
            state = ACTIONS;
        }
    }
}

void SingleBattleScene::draw_WEATHER(Fps const* fps, RenderSizes rs)
{
    if (weatherAnimation)
    {
        if (weatherAnimation->isRunning())
        {
            weatherAnimation->draw(fps, rs);
        }

        if (!weatherSpeech->shouldClose())
        {
            weatherSpeech->draw(fps, rs);
        }
    }
}

void SingleBattleScene::update_ACTIONS(Inputs const* inputs)
{
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

void SingleBattleScene::draw_ACTIONS(Fps const* fps, RenderSizes rs)
{
    battleSpeech->draw(fps, rs);
    battleActions->draw(fps, rs);
}

void SingleBattleScene::update_MOVES(Inputs const* inputs)
{
    if (noPpLeftSpeech)
    {
        noPpLeftSpeech->update(inputs);
        if (noPpLeftSpeech->shouldClose())
            noPpLeftSpeech.release();
        return;
    }

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
            playerMoveSpeech       = std::make_unique<TextSpeech>(renderer);
            boost::format moveText = boost::format(lc::translate("%1% uses %2% !")) % playerPkmn->getDisplayName()
                                   % playerMove->getDefinition()->getName();
            playerMoveSpeech->setTexts({moveText.str()});
            playerMoveSpeech->start();

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
                    else if (playerSpeed < encounterSpeed)
                        playerFirst = false;
                    else
                        playerFirst = Utils::randuint(0, 1);
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
            noPpLeftSpeech = std::make_unique<TextSpeech>(renderer);
            noPpLeftSpeech->setTexts({lc::translate("There's no PP left for this move !")});
            noPpLeftSpeech->start();
            moveSelection->reset();
        }
    }
    else
    {
        moveSelection->update(inputs);
    }
}

void SingleBattleScene::draw_MOVES(Fps const* fps, RenderSizes rs)
{
    moveSelection->draw(fps, rs);
    if (noPpLeftSpeech)
        noPpLeftSpeech->draw(fps, rs);
}

void SingleBattleScene::update_PLAYER_MOVES(Inputs const* inputs)
{
    if (playerMoveSpeech)
    {
        playerMoveSpeech->update(inputs);
        if (playerMoveSpeech->shouldClose())
            playerMoveSpeech.release();
        return;
    }

    playerMove->decreasePP();
    // TODO: Move animation
    auto category = playerMove->getDefinition()->getCategory();
    if (category != MoveDef::STATUS)
    {
        // TODO: Damage animation
        encounterPkmn->decreaseHP(computeDamage(playerPkmn, encounterPkmn, playerMove));

        if (encounterPkmn->isKO())
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
            float  a = 1.0; // 1.5 for Trainer, 1.0 for Wild
            size_t b = encounterPkmn->getDefinition()->getBaseExp();
            bool   hasLuckyEgg =
                playerPkmn->getHeldItem() && playerPkmn->getHeldItem()->getDefinition()->getId() == "LUCKYEGG";
            float  e = hasLuckyEgg ? 1.5 : 1.0;
            size_t L = encounterPkmn->getLevel();
            float  t = encounterPkmn->getFirstTrainerId() == Game::instance()->data.player.id ? 1.5 : 1.0;

            std::vector<Pkmn::PkmnPtr> partyCopy;
            partyCopy.push_back(playerPkmn);
            for (auto const& pkmn : Game::instance()->data.player.pkmns)
            {
                if (pkmn && !pkmn->isKO() && pkmn != playerPkmn)
                    partyCopy.push_back(pkmn);
            }

            auto isParticipant = [this](Pkmn::PkmnPtr const& pkmn) {
                return participatingPlayerPkmns.count(pkmn) > 0;
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
                    expGained[pkmn] += expFromBattle;
            }

            state = EXPERIENCE;
            return;
        }
    }

    if (playerFirst)
        state = OPPONENT_MOVES;
    else
        state = WEATHER;
}

void SingleBattleScene::draw_PLAYER_MOVES(Fps const* fps, RenderSizes rs)
{
    if (playerMoveSpeech)
    {
        playerMoveSpeech->draw(fps, rs);
        return;
    }
}

void SingleBattleScene::update_PLAYER_ITEMS(Inputs const* inputs)
{
    if (itemUseSpeech)
    {
        itemUseSpeech->update(inputs);
        if (!itemUseSpeech->shouldClose())
            return;

        itemUseSpeech.release();

        if (!itemUseResultUsed)
        {
            battleActions->reset();
            state = ACTIONS;
            return;
        }

        if (itemUseResultCaptureSuccess)
        {
            state = END;
            return;
        }

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
        return;
    }

    bool selected = selectedItem != nullptr;
    if (selected)
    {
        bool                     used           = false;
        bool                     captureSuccess = false;
        std::vector<std::string> itemMessages;
        auto                     def = selectedItem->getDefinition();
        if (def)
        {
            std::string itemName = def->getName();
            switch (def->getBattleUse())
            {
            case ItemDef::BattleUse::OnPkmn: {
                auto target = itemTargetPkmn ? itemTargetPkmn : playerPkmn;
                used        = ItemUtils::useItemOnPkmn(selectedItem, target, true);
                itemTargetPkmn.reset();
                if (used && target)
                {
                    boost::format itemText = boost::format(lc::translate("%1% used %2% on %3% !"))
                                           % Game::instance()->data.player.name % itemName % target->getDisplayName();
                    itemMessages.push_back(itemText.str());
                }
                break;
            }
            case ItemDef::BattleUse::OnMove:
                // TODO: Implement
                used = false;
                break;
            case ItemDef::BattleUse::OnFoe: {
                used = true;
                boost::format throwText =
                    boost::format(lc::translate("%1% threw %2% !")) % Game::instance()->data.player.name % itemName;
                itemMessages.push_back(throwText.str());
                captureSuccess = ItemUtils::useCaptureBall(selectedItem, encounterPkmn);
                if (captureSuccess)
                    ItemUtils::addPkmnToPlayer(encounterPkmn);
                if (captureSuccess)
                    itemMessages.push_back(lc::translate("Gotcha ! The Pkmn was caught !"));
                else
                    itemMessages.push_back(lc::translate("Oh no ! The Pkmn broke free !"));
                break;
            }
            case ItemDef::BattleUse::Direct:
            case ItemDef::BattleUse::OnBattler:
            case ItemDef::BattleUse::None:
            default:
                used = false;
                break;
            }
        }

        if (!used)
        {
            battleActions->reset();
            selectedItem.reset();
            state = ACTIONS;
            return;
        }

        ItemUtils::consumePlayerItem(Game::instance()->data.player, selectedItem);
        selectedItem.reset();
        itemUseResultUsed           = used;
        itemUseResultCaptureSuccess = captureSuccess;
        itemUseSpeech               = std::make_unique<TextSpeech>(renderer);
        itemUseSpeech->setTexts(itemMessages.empty() ? std::vector<std::string>{lc::translate("Item used !")}
                                                     : itemMessages);
        itemUseSpeech->start();
    }
    else
    {
        battleActions->reset();
        state = ACTIONS;
    }
}

void SingleBattleScene::draw_PLAYER_ITEMS(Fps const* fps, RenderSizes rs)
{
    if (itemUseSpeech)
        itemUseSpeech->draw(fps, rs);
}

void SingleBattleScene::update_PLAYER_PKMNS(Inputs const* inputs)
{
    if (pkmnEnterSpeech)
    {
        pkmnEnterSpeech->update(inputs);
        if (pkmnEnterSpeech->shouldClose())
            pkmnEnterSpeech.release();
        return;
    }

    setPlayerPkmn(newSelectedPkmn);
    newSelectedPkmn.reset();

    // TODO: Pkmn animation

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

void SingleBattleScene::draw_PLAYER_PKMNS(Fps const* fps, RenderSizes rs)
{
    if (pkmnEnterSpeech)
    {
        pkmnEnterSpeech->draw(fps, rs);
        return;
    }
}

void SingleBattleScene::update_PLAYER_RUN(Inputs const* inputs)
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
        size_t random      = Utils::randuint(0, 255);
        run                = random < odds;
    }

    if (Game::instance()->isDebug())
        std::cout << __PRETTY_FUNCTION__ << " run: " << run << std::endl;

    if (run)
    {
        runSpeech = std::make_unique<TextSpeech>(renderer);
        runSpeech->setTexts({lc::translate("Got away safely !")});
        runSpeech->start();
    }
    else
    {
        failedRunSpeech = std::make_unique<TextSpeech>(renderer);
        failedRunSpeech->setTexts({lc::translate("Can't get away !")});
        failedRunSpeech->start();
    }
}

void SingleBattleScene::draw_PLAYER_RUN(Fps const* fps, RenderSizes rs)
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

void SingleBattleScene::update_OPPONENT_PKMNS(Inputs const* /*inputs*/)
{
    // Should never reach
}

void SingleBattleScene::draw_OPPONENT_PKMNS(Fps const* /*fps*/, RenderSizes /*rs*/)
{
    // Should never reach
}

void SingleBattleScene::update_OPPONENT_ITEMS(Inputs const* /*inputs*/)
{
    // TODO: Attached berry

    state = WEATHER;
}

void SingleBattleScene::draw_OPPONENT_ITEMS(Fps const* /*fps*/, RenderSizes /*rs*/)
{
}

void SingleBattleScene::update_OPPONENT_MOVES(Inputs const* inputs)
{
    if (!encounterMove || encounterMove->getCurrentPP() == 0)
    {
        state = playerFirst ? WEATHER : PLAYER_MOVES;
        return;
    }

    if (Game::instance()->isDebug())
        std::cout << __PRETTY_FUNCTION__ << " Move: " << encounterMove->getDefinition()->getId() << std::endl;

    if (opponentMoveSpeech)
    {
        opponentMoveSpeech->update(inputs);
        if (opponentMoveSpeech->shouldClose())
            opponentMoveSpeech.release();
        return;
    }

    if (pkmnFaintSpeech)
    {
        pkmnFaintSpeech->update(inputs);
        if (pkmnFaintSpeech->shouldClose())
        {
            pkmnFaintSpeech.release();
            state = PKMNS;
        }
        return;
    }

    if (endSpeech)
    {
        endSpeech->update(inputs);
        if (endSpeech->shouldClose())
        {
            endSpeech.release();
            state = END;
        }
        return;
    }

    encounterMove->decreasePP();
    // TODO: Move animation
    auto category = encounterMove->getDefinition()->getCategory();
    if (category != MoveDef::STATUS)
    {
        // TODO: Damage animation
        playerPkmn->decreaseHP(computeDamage(encounterPkmn, playerPkmn, encounterMove));

        if (playerPkmn->isKO())
        {
            bool hasNextPkmn = false;
            for (auto const& pkmn : Game::instance()->data.player.pkmns)
            {
                if (pkmn && !pkmn->isKO())
                {
                    hasNextPkmn = true;
                    break;
                }
            }

            if (hasNextPkmn)
            {
                pkmnFaintSpeech = std::make_unique<TextSpeech>(renderer);
                boost::format pkmnFaintText =
                    boost::format(lc::translate("%1% fainted !")) % playerPkmn->getDisplayName();
                pkmnFaintSpeech->setTexts({pkmnFaintText.str()});
                pkmnFaintSpeech->start();
            }
            else
            {
                endSpeech = std::make_unique<TextSpeech>(renderer);
                endSpeech->setTexts({lc::translate("The battle is over ! You lost !")});
                endSpeech->start();
            }

            playerPkmn = nullptr;

            return;
        }
    }

    if (playerFirst)
        state = WEATHER;
    else
        state = PLAYER_MOVES;
}

void SingleBattleScene::draw_OPPONENT_MOVES(Fps const* fps, RenderSizes rs)
{
    if (pkmnFaintSpeech)
    {
        pkmnFaintSpeech->draw(fps, rs);
        return;
    }

    if (opponentMoveSpeech)
    {
        opponentMoveSpeech->draw(fps, rs);
        return;
    }
}

void SingleBattleScene::update_OPPONENT_RUN(Inputs const* inputs)
{
    if (!runSpeech)
    {
        runSpeech             = std::make_unique<TextSpeech>(renderer);
        boost::format runText = boost::format(lc::translate("Wild %1% fled !"))
                              % (encounterPkmn ? encounterPkmn->getDisplayName() : "#ERROR");
        runSpeech->setTexts({runText.str()});
        runSpeech->start();
    }

    runSpeech->update(inputs);
    if (runSpeech->shouldClose())
    {
        runSpeech.release();
        state = END;
    }
}

void SingleBattleScene::draw_OPPONENT_RUN(Fps const* fps, RenderSizes rs)
{
    if (runSpeech)
    {
        runSpeech->draw(fps, rs);
    }
}

void SingleBattleScene::update_EXPERIENCE(Inputs const* inputs)
{
    // TODO: Split experience animation for each level
    if (false) // Experience animation is running
    {
        return;
    }

    if (experienceSpeech)
    {
        experienceSpeech->update(inputs);
        if (experienceSpeech->shouldClose())
            experienceSpeech.release();
        return;
    }

    if (moveToLearnSpeech)
    {
        moveToLearnSpeech->update(inputs);
        if (moveToLearnSpeech->shouldClose())
            moveToLearnSpeech.release();
        // TODO: Ask for new move
        // TODO: Go to NewMovesScene if ok
        shouldGoToNewMovesScene = true;
        return;
    }

    if (endSpeech)
    {
        endSpeech->update(inputs);
        if (endSpeech->shouldClose())
        {
            endSpeech.release();
            state = END;
        }
        return;
    }

    if (expGained.empty())
    {
        endSpeech = std::make_unique<TextSpeech>(renderer);
        endSpeech->setTexts({lc::translate("The battle is over ! You won !")});
        endSpeech->start();
        return;
    }

    auto grantExperience = [this](Pkmn::PkmnPtr& pkmn, size_t& expFromBattle) {
        while (expFromBattle > 0)
        {
            size_t expLeftToNextLevel = pkmn->expToNextLevel() - pkmn->getExp();
            if (expFromBattle > expLeftToNextLevel)
            {
                pkmn->incrementLevel();
                std::string evolutionId = canEvolve(pkmn);
                if (!evolutionId.empty())
                {
                    evolutionDef = Game::instance()->data.pkmnDefFor(evolutionId);
                    if (evolutionDef)
                        shouldBreakToEvolution = true;
                }

                for (auto const& moveToLearn : pkmn->getDefinition()->getMovesToLearn())
                {
                    if (moveToLearn.level == pkmn->getLevel())
                    {
                        shouldBreakToNewMove = true;
                        break;
                    }
                }

                expFromBattle -= expLeftToNextLevel;

                shouldBreakToLevelUp = true;

                return;
            }
            else
            {
                pkmn->increaseExp(expFromBattle);
                expFromBattle = 0;
            }
        }
    };

    expPkmn                        = expGained.begin()->first;
    size_t& remainingExpFromBattle = expGained.begin()->second;

    if (expFromBattle == 0)
    {
        expFromBattle    = remainingExpFromBattle;
        experienceSpeech = std::make_unique<TextSpeech>(renderer);
        boost::format experienceText =
            boost::format(lc::translate("%1% gained %2% XP !")) % expPkmn->getDisplayName() % expFromBattle;
        experienceSpeech->setTexts({experienceText.str()});
        experienceSpeech->start();
        return;
    }

    if (remainingExpFromBattle == 0)
    {
        expGained.erase(expPkmn);
        expPkmn       = nullptr;
        expFromBattle = 0;
        return;
    }

    grantExperience(expPkmn, remainingExpFromBattle);

    if (shouldBreakToLevelUp)
    {
        experienceSpeech = std::make_unique<TextSpeech>(renderer);
        boost::format experienceText =
            boost::format(lc::translate("%1% reached level %2% !")) % expPkmn->getDisplayName() % expPkmn->getLevel();
        experienceSpeech->setTexts({experienceText.str()});
        experienceSpeech->start();
        shouldBreakToLevelUp = false;
        return;
    }

    if (shouldBreakToEvolution)
    {
        return;
    }

    if (shouldBreakToNewMove)
    {
        for (auto const& moveToLearn : expPkmn->getDefinition()->getMovesToLearn())
        {
            if (moveToLearn.level == expPkmn->getLevel())
            {
                moveToLearnSpeech           = std::make_unique<TextSpeech>(renderer);
                MoveDef::MoveDefPtr moveDef = Game::instance()->data.moveDefFor(moveToLearn.id);
                if (moveDef)
                {
                    boost::format moveToLearnText =
                        boost::format(lc::translate("New move learned: %1%")) % moveDef->getName();
                    moveToLearnSpeech->setTexts({moveToLearnText.str()});
                    moveToLearnSpeech->start();
                    break;
                }
            }
        }
        shouldBreakToNewMove = false;
        return;
    }
}

void SingleBattleScene::draw_EXPERIENCE(Fps const* fps, RenderSizes rs)
{
    // TODO: Experience animation

    if (experienceSpeech)
    {
        experienceSpeech->draw(fps, rs);
        return;
    }

    if (moveToLearnSpeech)
    {
        moveToLearnSpeech->draw(fps, rs);
        return;
    }

    if (endSpeech)
    {
        endSpeech->draw(fps, rs);
        return;
    }
}

void SingleBattleScene::update_END(Inputs const* /*inputs*/)
{
    if (!fadeOutAnimation->isStarted())
    {
        fadeOutAnimation->start();
    }

    if (!fadeOutAnimation->isFinished())
    {
        fadeOutAnimation->incrementTicks();
    }
}

void SingleBattleScene::draw_END(Fps const* fps, RenderSizes rs)
{
    fadeOutAnimation->draw(fps, rs);
}

bool SingleBattleScene::pushScene() const
{
    return shouldBreakToEvolution || state == BAG || state == PKMNS;
}

void SingleBattleScene::popReset()
{
    if (shouldBreakToEvolution)
    {
        shouldBreakToEvolution = false;
        if (expPkmn == playerPkmn)
            singleBattleUi->setPlayerPkmn(playerPkmn); // force loading new texture and redraw
        return;
    }

    switch (state)
    {
    case BAG: {
        state = PLAYER_ITEMS;
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

            pkmnEnterSpeech = std::make_unique<TextSpeech>(renderer);
            boost::format pkmnEnterText =
                boost::format(lc::translate("%1% enters the battle !")) % newSelectedPkmn->getDisplayName();
            pkmnEnterSpeech->setTexts({pkmnEnterText.str()});
            pkmnEnterSpeech->start();

            state = PLAYER_PKMNS;
        }
        break;
    }
    default:
        break;
    }
}

std::unique_ptr<Scene> SingleBattleScene::nextScene()
{
    if (shouldBreakToEvolution)
        return std::make_unique<EvolutionScene>(renderer, expPkmn, evolutionDef);

    // TODO
    // if (shouldGoToNewMovesScene)
    //     return std::make_unique<NewMovesScene>(renderer, expPkmn);

    switch (state)
    {
    case BAG:
        return std::make_unique<BagScene>(renderer, selectedItem, itemTargetPkmn);
    case PKMNS:
        newSelectedPkmn = playerPkmn;
        return std::make_unique<PkmnsScene>(renderer, newSelectedPkmn, playerPkmn == nullptr);
    default:
        return nullptr;
    }
}
