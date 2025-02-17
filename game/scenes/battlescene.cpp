#include "battlescene.h"

#include "animations/weather/blizzardanimation.h"
#include "animations/weather/harshsunlightanimation.h"
#include "animations/weather/rainanimation.h"
#include "animations/weather/sandstormanimation.h"
#include "game.h"
#include "utils.h"

std::string BattleScene::StateToString(State e)
{
    switch (e)
    {
    case START:
        return "START";
    case WEATHER:
        return "WEATHER";
    case ACTIONS:
        return "ACTIONS";
    case MOVES:
        return "MOVES";
    case BAG:
        return "BAG";
    case PKMNS:
        return "PKMNS";
    case PLAYER_MOVES:
        return "P_MOVES";
    case PLAYER_ITEMS:
        return "P_ITEMS";
    case PLAYER_PKMNS:
        return "P_PKMNS";
    case PLAYER_RUN:
        return "P_RUN";
    case OPPONENT_PKMNS:
        return "O_PKMNS";
    case OPPONENT_ITEMS:
        return "O_ITEMS";
    case OPPONENT_MOVES:
        return "O_MOVES";
    case OPPONENT_RUN:
        return "O_RUN";
    case END:
        return "END";
    }
    return "???";
}

BattleScene::BattleScene(SDL_Renderer* renderer) : Scene(renderer)
{
    battleBackground = std::make_unique<BattleBackground>(renderer);
    battleSpeech     = std::make_unique<BattleSpeech>(renderer);
    battleActions    = std::make_unique<BattleActions>(renderer);
    battleActions->init();
    singleBattleUi = std::make_unique<SingleBattleUi>(renderer);
    moveSelection  = std::make_unique<MoveSelection>(renderer);
    moveSelection->init();
    fadeOutAnimation = std::make_unique<FadeAnimation>(renderer, false);
    eyeAnimation     = std::make_unique<EyeAnimation>(renderer);
    eyeAnimation->start();
}

BattleScene::~BattleScene()
{
}

void BattleScene::init()
{
    state = START;
}

void BattleScene::update(Inputs const* inputs)
{
    if (Game::instance()->isDebug() && previousState != state)
        std::cout << name() << ": " << StateToString(state) << std::endl;

    previousState = state;

    switch (state)
    {
    case START: {
        update_START(inputs);
        break;
    }
    case WEATHER: {
        update_WEATHER(inputs);
        break;
    }
    case ACTIONS: {
        update_ACTIONS(inputs);
        break;
    }
    case MOVES: {
        update_MOVES(inputs);
        break;
    }
    case PLAYER_MOVES: {
        update_P_MOVES(inputs);
        break;
    }
    case PLAYER_ITEMS: {
        update_P_ITEMS(inputs);
        break;
    }
    case PLAYER_PKMNS: {
        update_P_PKMNS(inputs);
        break;
    }
    case PLAYER_RUN: {
        update_P_RUN(inputs);
        break;
    }
    case OPPONENT_PKMNS: {
        update_O_PKMNS(inputs);
        break;
    }
    case OPPONENT_ITEMS: {
        update_O_ITEMS(inputs);
        break;
    }
    case OPPONENT_MOVES: {
        update_O_MOVES(inputs);
        break;
    }
    case OPPONENT_RUN: {
        update_O_RUN(inputs);
        break;
    }
    case END: {
        update_END(inputs);
        break;
    }
    case BAG:
    case PKMNS:
        break;
    }
}

void BattleScene::draw(Fps const* fps, RenderSizes rs)
{
    switch (state)
    {
    case START:
        draw_START(fps, rs);
        break;
    case WEATHER:
        draw_WEATHER(fps, rs);
        break;
    case ACTIONS:
        draw_ACTIONS(fps, rs);
        break;
    case MOVES:
        draw_MOVES(fps, rs);
        break;
    case PLAYER_MOVES:
        draw_P_MOVES(fps, rs);
        break;
    case PLAYER_ITEMS:
        draw_P_ITEMS(fps, rs);
        break;
    case PLAYER_PKMNS:
        draw_P_PKMNS(fps, rs);
        break;
    case PLAYER_RUN:
        draw_P_RUN(fps, rs);
        break;
    case OPPONENT_PKMNS:
        draw_O_PKMNS(fps, rs);
        break;
    case OPPONENT_ITEMS:
        draw_O_ITEMS(fps, rs);
        break;
    case OPPONENT_MOVES:
        draw_O_MOVES(fps, rs);
        break;
    case OPPONENT_RUN:
        draw_O_RUN(fps, rs);
        break;
    case END:
        draw_END(fps, rs);
    case BAG:
    case PKMNS:
        break;
    }
}

bool BattleScene::popScene() const
{
    return state == END && fadeOutAnimation->isFinished();
}

std::string BattleScene::name()
{
    return "BattleScene";
}

size_t BattleScene::computeDamage(Pkmn::PkmnPtr const& pkmn, Move::MovePtr const& move) const
{
    if (!move)
        return 0;

    auto category = move->getDefinition()->getCategory();

    // https://bulbapedia.bulbagarden.net/wiki/Damage#Generation_III
    float  level = pkmn->getLevel();
    size_t power = move->getDefinition()->getPower();
    size_t ad =
        category == MoveDef::PHYSICAL ? pkmn->getStats()[PkmnDef::ATTACK] : pkmn->getStats()[PkmnDef::SPECIAL_ATTACK];
    float burn = pkmn->getStatusCondition() == Pkmn::StatusCondition::BURN ? 0.5 : 1;
    // TODO
    float  screen  = 1.0;
    size_t targets = 1;
    // TODO
    float weather = 1.0;
    // TODO
    float  ff        = 1.0;
    float  damage    = (((2 * level / 5 + 2) * power * ad) / 50) * burn * screen * targets * weather * ff;
    size_t stockpile = 1;
    // TODO
    // https://bulbapedia.bulbagarden.net/wiki/Critical_hit
    size_t critical = 1;
    // TODO
    size_t doubledmg = 1;
    // TODO
    size_t charge = 1;
    // TODO
    size_t      hh        = 1;
    auto const& pkmnTypes = pkmn->getDefinition()->getTypes();
    float       stab =
        std::find(pkmnTypes.begin(), pkmnTypes.end(), move->getDefinition()->getType()) != pkmnTypes.end() ? 1.5 : 1.0;
    // TODO
    // https://bulbapedia.bulbagarden.net/wiki/Type#Type_effectiveness
    float typeEffectiveness = 1.0;
    float random            = Utils::randuint(85, 100) / 100.0;
    damage *= stockpile * critical * doubledmg * charge * hh * stab * typeEffectiveness * random;
    return damage;
}

std::string BattleScene::canEvolve(Pkmn::PkmnPtr const& pkmn)
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

void BattleScene::changeWeather(Map::Weather weather)
{
    this->weather = weather;

    if (weather == Map::Weather::NONE)
    {
        weatherAnimation.release();
        weatherSpeech.release();
        return;
    }

    weatherSpeech = std::make_unique<TextSpeech>(renderer);

    switch (weather)
    {
    case Map::HARSH_SUNLIGHT:
        weatherAnimation = std::make_unique<HarshSunlightAnimation>(renderer, false);
        break;
    case Map::HAIL:
        weatherAnimation = std::make_unique<BlizzardAnimation>(renderer, false);
        break;
    case Map::SANDSTORM:
        weatherAnimation = std::make_unique<SandstormAnimation>(renderer, false);
        break;
    case Map::RAIN:
        weatherAnimation = std::make_unique<RainAnimation>(renderer, false);
        break;
    default:
        break;
    }
    weatherSpeech->setTexts({weatherAnimation->getStartingText()});
    weatherSpeech->start();
}
