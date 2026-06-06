#include "singlebattlescene.h"

#include "game.h"
#include "scenes/bagscene.h"
#include "scenes/evolutionscene.h"
#include "scenes/pkmnsscene.h"
#include "utils.h"

#include <algorithm>
#include <cctype>

SingleBattleScene::SingleBattleScene(SDL_Renderer* renderer) : Scene(renderer)
{
    battleBackground = std::make_unique<BattleBackground>(renderer);
    singleBattleUi   = std::make_unique<SingleBattleUi>(renderer);
}

SingleBattleScene::~SingleBattleScene()
{
}

void SingleBattleScene::initPhases()
{
    startPhase         = std::make_unique<SingleBattleStartPhase>(*this, data, renderer);
    weatherPhase       = std::make_unique<SingleBattleWeatherPhase>(*this, data, renderer);
    actionsPhase       = std::make_unique<SingleBattleActionsPhase>(*this, data, renderer);
    movesPhase         = std::make_unique<SingleBattleMovesPhase>(*this, data, renderer);
    playerPkmnsPhase   = std::make_unique<SingleBattlePlayerPkmnsPhase>(*this, data, renderer);
    opponentPkmnsPhase = std::make_unique<SingleBattleOpponentPkmnsPhase>(*this, data, renderer);
    opponentItemsPhase = std::make_unique<SingleBattleOpponentItemsPhase>(*this, data, renderer);
    endPhase           = std::make_unique<SingleBattleEndPhase>(*this, data, renderer);
}

SingleBattlePhase* SingleBattleScene::currentPhase()
{
    switch (data->state)
    {
    case START:
        return startPhase.get();
    case WEATHER:
        return weatherPhase.get();
    case ACTIONS:
        return actionsPhase.get();
    case MOVES:
        return movesPhase.get();
    case PLAYER_MOVES:
        return playerMovesPhase.get();
    case PLAYER_ITEMS:
        return playerItemsPhase.get();
    case PLAYER_PKMNS:
        return playerPkmnsPhase.get();
    case PLAYER_RUN:
        return playerRunPhase.get();
    case OPPONENT_PKMNS:
        return opponentPkmnsPhase.get();
    case OPPONENT_ITEMS:
        return opponentItemsPhase.get();
    case OPPONENT_MOVES:
        return opponentMovesPhase.get();
    case OPPONENT_RUN:
        return opponentRunPhase.get();
    case EXPERIENCE:
        return experiencePhase.get();
    case END:
        return endPhase.get();
    case BAG:
    case PKMNS:
        return nullptr;
    }
    return nullptr;
}

void SingleBattleScene::init()
{
    data->state = START;
    data->expGained.clear();
    data->participatingPlayerPkmns.clear();
    if (data->playerPkmn)
        data->participatingPlayerPkmns.insert(data->playerPkmn);

    boost::format firstPkmnText =
        boost::format(lc::translate("%1% go !")) % (data->playerPkmn ? data->playerPkmn->getDisplayName() : "#ERROR");
    startPhase->init(encounterStartText(), firstPkmnText.str());
    actionsPhase->init();
    movesPhase->init();
}

void SingleBattleScene::update(Inputs const* inputs)
{
    if (Game::instance()->isDebug() && data->previousState != data->state)
        std::cout << name() << ": " << battleStateToString(data->state) << std::endl;

    data->previousState = data->state;

    if (auto* phase = currentPhase())
        phase->update(inputs);

    if (data->previousState != data->state)
    {
        if (auto* phase = currentPhase())
            phase->preupdate();
    }
}

void SingleBattleScene::draw(Fps const* fps, RenderSizes rs)
{
    battleBackground->draw(fps, rs);
    singleBattleUi->draw(fps, rs);

    if (auto* phase = currentPhase())
        phase->draw(fps, rs);
}

bool SingleBattleScene::popScene() const
{
    return data->state == END && endPhase && endPhase->isFinished();
}

std::string SingleBattleScene::name()
{
    return "SingleBattleScene";
}

size_t SingleBattleScene::computeDamage(Pkmn::PkmnPtr const& attacker,
                                        Pkmn::PkmnPtr const& defender,
                                        Move::MovePtr const& move,
                                        Map::Weather const&  weather)
{
    if (!attacker || !defender || !move)
        return 0;

    auto category = move->getDefinition()->getCategory();
    if (category == MoveDef::STATUS)
        return 0;

    // https://bulbapedia.bulbagarden.net/wiki/Damage#Generation_III
    float  level = attacker->getLevel();
    size_t power = move->getDefinition()->getPower();
    size_t a     = category == MoveDef::PHYSICAL ? attacker->getStats()[PkmnDef::ATTACK]
                                                 : attacker->getStats()[PkmnDef::SPECIAL_ATTACK];
    size_t d     = category == MoveDef::PHYSICAL ? defender->getStats()[PkmnDef::DEFENSE]
                                                 : defender->getStats()[PkmnDef::SPECIAL_DEFENSE];
    if (d == 0)
        d = 1;
    float burn =
        (category == MoveDef::PHYSICAL && attacker->getStatusCondition() == Pkmn::StatusCondition::BURN) ? 0.5 : 1.0;
    float  screen            = 1.0;
    size_t targets           = 1;
    float  weatherMultiplier = 1.0;
    if (weather == Map::Weather::RAIN)
    {
        if (move->getDefinition()->getType() == "WATER")
            weatherMultiplier = 1.5;
        else if (move->getDefinition()->getType() == "FIRE")
            weatherMultiplier = 0.5;
    }
    else if (weather == Map::Weather::HARSH_SUNLIGHT)
    {
        if (move->getDefinition()->getType() == "FIRE")
            weatherMultiplier = 1.5;
        else if (move->getDefinition()->getType() == "WATER")
            weatherMultiplier = 0.5;
    }
    float  ff     = 1.0;
    float  damage = (((2 * level / 5 + 2) * power * a / d) / 50 + 2) * burn * screen * targets * weatherMultiplier * ff;
    size_t stockpile = 1;
    // https://bulbapedia.bulbagarden.net/wiki/Critical_hit
    float       critical  = Utils::randuint(0, 15) == 0 ? 2.0 : 1.0;
    size_t      doubledmg = 1;
    size_t      charge    = 1;
    size_t      hh        = 1;
    auto const& pkmnTypes = attacker->getDefinition()->getTypes();
    float       stab =
        std::find(pkmnTypes.begin(), pkmnTypes.end(), move->getDefinition()->getType()) != pkmnTypes.end() ? 1.5 : 1.0;
    // https://bulbapedia.bulbagarden.net/wiki/Type#Type_effectiveness
    float typeEffectiveness = 1.0;
    for (auto const& defenderTypeId : defender->getDefinition()->getTypes())
    {
        auto defenderType = Game::instance()->data.typeFor(defenderTypeId);
        if (!defenderType)
            continue;

        auto const& immunities = defenderType->getImmunities();
        if (std::find(immunities.begin(), immunities.end(), move->getDefinition()->getType()) != immunities.end())
            typeEffectiveness *= 0.0;

        auto const& weaknesses = defenderType->getWeaknesses();
        if (std::find(weaknesses.begin(), weaknesses.end(), move->getDefinition()->getType()) != weaknesses.end())
            typeEffectiveness *= 2.0;

        auto const& resistances = defenderType->getResistances();
        if (std::find(resistances.begin(), resistances.end(), move->getDefinition()->getType()) != resistances.end())
            typeEffectiveness *= 0.5;
    }
    float random = Utils::randuint(85, 100) / 100.0;
    damage *= stockpile * critical * doubledmg * charge * hh * stab * typeEffectiveness * random;
    if (damage < 1.0)
        damage = 1.0;
    return static_cast<size_t>(damage);
}

std::string SingleBattleScene::canEvolve(Pkmn::PkmnPtr const& pkmn)
{
    auto stats = pkmn->getStats();

    std::vector<std::string> possibleEvolutions;

    auto evos = pkmn->getDefinition()->getEvolutions();
    for (auto const& e : evos)
    {
        switch (e.first)
        {
        case PkmnDef::AttackGreater:
            if (pkmn->getLevel() == size_t(std::stoi(e.second.data)))
            {
                if (stats[PkmnDef::ATTACK] > stats[PkmnDef::DEFENSE])
                    possibleEvolutions.push_back(e.second.pkmnId);
            }
            break;
        case PkmnDef::DefenseGreater:
            if (pkmn->getLevel() == size_t(std::stoi(e.second.data)))
            {
                if (stats[PkmnDef::ATTACK] < stats[PkmnDef::DEFENSE])
                    possibleEvolutions.push_back(e.second.pkmnId);
            }
            break;
        case PkmnDef::AtkDefEqual:
            if (pkmn->getLevel() == size_t(std::stoi(e.second.data)))
            {
                if (stats[PkmnDef::ATTACK] == stats[PkmnDef::DEFENSE])
                    possibleEvolutions.push_back(e.second.pkmnId);
            }
            break;
        case PkmnDef::Happiness:
            if (pkmn->getLevel() == size_t(std::stoi(e.second.data)))
            {
                if (pkmn->getHappiness() >= Pkmn::HighFriendship)
                    possibleEvolutions.push_back(e.second.pkmnId);
            }
            break;
        case PkmnDef::HasInParty:
            if (pkmn->getLevel() == size_t(std::stoi(e.second.data)))
            {
                bool hasInParty = false;
                for (auto const& pkmn : Game::instance()->data.player.pkmns)
                {
                    if (pkmn && pkmn->getDefinition()->getId() == e.second.data)
                    {
                        hasInParty = true;
                        break;
                    }
                }
                if (hasInParty)
                    possibleEvolutions.push_back(e.second.pkmnId);
            }
            break;
        case PkmnDef::HasMove:
            if (pkmn->getLevel() == size_t(std::stoi(e.second.data)))
            {
                bool hasMove = false;
                for (auto const& move : pkmn->getMoves())
                {
                    if (move->getDefinition()->getId() == e.second.data)
                    {
                        hasMove = true;
                        break;
                    }
                }
                if (hasMove)
                    possibleEvolutions.push_back(e.second.pkmnId);
            }
            break;
        case PkmnDef::Level:
            if (pkmn->getLevel() == size_t(std::stoi(e.second.data)))
                possibleEvolutions.push_back(e.second.pkmnId);
            break;
        case PkmnDef::LevelMale:
            if (pkmn->getLevel() == size_t(std::stoi(e.second.data)))
            {
                if (pkmn->isMale())
                    possibleEvolutions.push_back(e.second.pkmnId);
            }
            break;
        case PkmnDef::LevelFemale:
            if (pkmn->getLevel() == size_t(std::stoi(e.second.data)))
            {
                if (pkmn->isFemale())
                    possibleEvolutions.push_back(e.second.pkmnId);
            }
            break;
        case PkmnDef::HasEmptySlotInParty:
            // TODO do only if evolution is not cancelled ?
            if (pkmn->getLevel() == size_t(std::stoi(e.second.data)))
            {
                size_t i = 0;
                for (auto const& pkmn : Game::instance()->data.player.pkmns)
                {
                    if (!pkmn)
                        break;
                    ++i;
                }
                if (i < Game::instance()->data.player.pkmns.size())
                {
                    PkmnDef::PkmnDefPtr newPkmnDef = Game::instance()->data.pkmnDefFor(e.second.pkmnId);
                    if (newPkmnDef)
                    {
                        Pkmn::PkmnPtr newPkmn = std::make_shared<Pkmn>(newPkmnDef, size_t(std::stoi(e.second.data)));
                        Game::instance()->data.player.pkmns.at(i).swap(newPkmn);
                    }
                }
            }
        default:
            break;
        }
    }

    if (possibleEvolutions.size())
    {
        size_t index = Utils::randuint(0, possibleEvolutions.size() - 1);
        return possibleEvolutions[index];
    }

    return {};
}

void SingleBattleScene::changeWeather(Map::Weather weather)
{
    weatherPhase->setWeather(weather);
}

void SingleBattleScene::setOpponentPkmn(Pkmn::PkmnPtr const& newOpponentPkmn)
{
    data->opponentPkmn = newOpponentPkmn;

    data->expGained.clear();
    data->participatingPlayerPkmns.clear();
    if (data->playerPkmn)
        data->participatingPlayerPkmns.insert(data->playerPkmn);

    singleBattleUi->setFoePkmn(newOpponentPkmn);
}

void SingleBattleScene::setPlayerPkmn(Pkmn::PkmnPtr const& newPlayerPkmn)
{
    data->playerPkmn = newPlayerPkmn;
    singleBattleUi->setPlayerPkmn(newPlayerPkmn);
    if (newPlayerPkmn)
        data->participatingPlayerPkmns.insert(newPlayerPkmn);
}

bool SingleBattleScene::pushScene() const
{
    return data->shouldBreakToEvolution || data->state == BAG || data->state == PKMNS;
}

void SingleBattleScene::popReset()
{
    if (data->shouldBreakToEvolution)
    {
        data->shouldBreakToEvolution = false;
        if (data->expPkmn == data->playerPkmn)
            singleBattleUi->setPlayerPkmn(data->playerPkmn);
        return;
    }

    switch (data->state)
    {
    case BAG: {
        data->state = PLAYER_ITEMS;
        break;
    }
    case PKMNS: {
        if (!data->newSelectedPkmn || data->newSelectedPkmn == data->playerPkmn)
        {
            if (Game::instance()->isDebug())
                std::cout << __PRETTY_FUNCTION__ << " no new selected pkmn" << std::endl;

            data->newSelectedPkmn.reset();
            data->state = ACTIONS;
        }
        else
        {
            if (Game::instance()->isDebug())
                std::cout << __PRETTY_FUNCTION__ << " new selected pkmn: " << data->newSelectedPkmn->getDisplayName()
                          << std::endl;

            boost::format pkmnEnterText =
                boost::format(lc::translate("%1% enters the battle !")) % data->newSelectedPkmn->getDisplayName();
            playerPkmnsPhase->showEnterSpeech(pkmnEnterText.str());

            data->state = PLAYER_PKMNS;
        }
        break;
    }
    default:
        break;
    }
}

std::unique_ptr<Scene> SingleBattleScene::nextScene()
{
    if (data->shouldBreakToEvolution)
        return std::make_unique<EvolutionScene>(renderer, data->expPkmn, data->evolutionDef);

    switch (data->state)
    {
    case BAG:
        return std::make_unique<BagScene>(renderer, data->selectedItem, data->itemTargetPkmn);
    case PKMNS:
        data->newSelectedPkmn = data->playerPkmn;
        return std::make_unique<PkmnsScene>(renderer, data->newSelectedPkmn, data->playerPkmn == nullptr);
    default:
        return nullptr;
    }
}
