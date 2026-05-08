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
        return "PLAYER_MOVES";
    case PLAYER_ITEMS:
        return "PLAYER_ITEMS";
    case PLAYER_PKMNS:
        return "PLAYER_PKMNS";
    case PLAYER_RUN:
        return "PLAYER_RUN";
    case OPPONENT_PKMNS:
        return "OPPONENT_PKMNS";
    case OPPONENT_ITEMS:
        return "OPPONENT_ITEMS";
    case OPPONENT_MOVES:
        return "OPPONENT_MOVES";
    case OPPONENT_RUN:
        return "OPPONENT_RUN";
    case EXPERIENCE:
        return "EXPERIENCE";
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
        update_PLAYER_MOVES(inputs);
        break;
    }
    case PLAYER_ITEMS: {
        update_PLAYER_ITEMS(inputs);
        break;
    }
    case PLAYER_PKMNS: {
        update_PLAYER_PKMNS(inputs);
        break;
    }
    case PLAYER_RUN: {
        update_PLAYER_RUN(inputs);
        break;
    }
    case OPPONENT_PKMNS: {
        update_OPPONENT_PKMNS(inputs);
        break;
    }
    case OPPONENT_ITEMS: {
        update_OPPONENT_ITEMS(inputs);
        break;
    }
    case OPPONENT_MOVES: {
        update_OPPONENT_MOVES(inputs);
        break;
    }
    case OPPONENT_RUN: {
        update_OPPONENT_RUN(inputs);
        break;
    }
    case EXPERIENCE: {
        update_EXPERIENCE(inputs);
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
        draw_PLAYER_MOVES(fps, rs);
        break;
    case PLAYER_ITEMS:
        draw_PLAYER_ITEMS(fps, rs);
        break;
    case PLAYER_PKMNS:
        draw_PLAYER_PKMNS(fps, rs);
        break;
    case PLAYER_RUN:
        draw_PLAYER_RUN(fps, rs);
        break;
    case OPPONENT_PKMNS:
        draw_OPPONENT_PKMNS(fps, rs);
        break;
    case OPPONENT_ITEMS:
        draw_OPPONENT_ITEMS(fps, rs);
        break;
    case OPPONENT_MOVES:
        draw_OPPONENT_MOVES(fps, rs);
        break;
    case OPPONENT_RUN:
        draw_OPPONENT_RUN(fps, rs);
        break;
    case EXPERIENCE:
        draw_EXPERIENCE(fps, rs);
        break;
    case END:
        draw_END(fps, rs);
        break;
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

size_t BattleScene::computeDamage(Pkmn::PkmnPtr const& attacker,
                                  Pkmn::PkmnPtr const& defender,
                                  Move::MovePtr const& move) const
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
