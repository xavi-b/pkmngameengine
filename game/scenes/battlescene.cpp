#include "battlescene.h"

#include "bagscene.h"
#include "game.h"
#include "pkmnsscene.h"
#include "utils.h"

std::string BattleScene::StateToString(State e)
{
    switch (e)
    {
    case START:
        return "START";
    case ACTIONS:
        return "ACTIONS";
    case BAG:
        return "BAG";
    case PKMNS:
        return "PKMNS";
    case P_MOVES:
        return "P_MOVES";
    case P_ITEMS:
        return "P_ITEMS";
    case P_PKMNS:
        return "P_PKMNS";
    case P_RUN:
        return "P_RUN";
    case O_PKMNS:
        return "O_PKMNS";
    case O_ITEMS:
        return "O_ITEMS";
    case O_MOVES:
        return "O_MOVES";
    case END:
        return "END";
    }
    return "???";
}

BattleScene::BattleScene(SDL_Renderer* renderer) : Scene(renderer)
{
    battleBackground = std::make_unique<BattleBackground>(renderer);
    battleSpeech     = std::make_unique<BattleSpeech>(renderer);
    std::vector<std::string> texts;
    texts.push_back(lc::translate("What should AAAAAAAAAAAA do ?"));
    battleSpeech->setTexts(texts);
    battleSpeech->init();
    battleActions = std::make_unique<BattleActions>(renderer);
    battleActions->init();
    moveSelection = std::make_unique<MoveSelection>(renderer);
    moveSelection->init();
}

BattleScene::~BattleScene()
{
}

void BattleScene::init()
{
    state = ACTIONS;
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
    case ACTIONS: {
        update_ACTIONS(inputs);
        break;
    }
    case P_MOVES: {
        update_P_MOVES(inputs);
        break;
    }
    case P_ITEMS: {
        update_P_ITEMS(inputs);
        break;
    }
    case P_PKMNS: {
        update_P_PKMNS(inputs);
        break;
    }
    case P_RUN: {
        update_P_RUN(inputs);
        break;
    }
    case O_PKMNS: {
        update_O_PKMNS(inputs);
        break;
    }
    case O_ITEMS: {
        update_O_ITEMS(inputs);
        break;
    }
    case O_MOVES: {
        update_O_MOVES(inputs);
        break;
    }
    case END:
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
    case ACTIONS:
        draw_ACTIONS(fps, rs);
        break;
    case P_MOVES:
        draw_P_MOVES(fps, rs);
        break;
    case P_ITEMS:
        draw_P_ITEMS(fps, rs);
        break;
    case P_PKMNS:
        draw_P_PKMNS(fps, rs);
        break;
    case P_RUN:
        draw_P_RUN(fps, rs);
        break;
    case O_PKMNS:
        draw_O_PKMNS(fps, rs);
        break;
    case O_ITEMS:
        draw_O_ITEMS(fps, rs);
        break;
    case O_MOVES:
        draw_O_MOVES(fps, rs);
        break;
    case END:
    case BAG:
    case PKMNS:
        break;
    }
}

bool BattleScene::popScene() const
{
    return state == END;
}

bool BattleScene::pushScene() const
{
    return state == BAG || state == PKMNS;
}

void BattleScene::popReset()
{
    switch (state)
    {
    case BAG: {
        // If selected
        state = P_ITEMS;
        // If not selected
        state = ACTIONS;
        break;
    }
    case PKMNS: {
        // If selected
        state = P_PKMNS;
        // If not selected
        state = ACTIONS;
        break;
    }
    default:
        break;
    }
}

std::unique_ptr<Scene> BattleScene::nextScene()
{
    switch (state)
    {
    case BAG:
        return std::make_unique<BagScene>(renderer);
    case PKMNS:
        return std::make_unique<PkmnsScene>(renderer);
    default:
        return nullptr;
    }
}

std::string BattleScene::name()
{
    return "BattleScene";
}

size_t BattleScene::computeDamage(Pkmn::PkmnPtr const& pkmn, Move::MovePtr const& move) const
{
    if (!move || move->getCurrentPP() == 0)
        return 0;

    auto category = move->getDefinition()->getCategory();

    // https://bulbapedia.bulbagarden.net/wiki/Damage#Generation_III
    float  level = pkmn->getLevel();
    size_t power = move->getDefinition()->getPower();
    size_t ad =
        category == MoveDef::PHYSICAL ? pkmn->getStats()[PkmnDef::ATTACK] : pkmn->getStats()[PkmnDef::SPECIAL_ATTACK];
    float burn = pkmn->getStatusCondition() == Pkmn::BURN ? 0.5 : 1;
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
    float random            = Utils::randint(85, 100) / 100.0;
    damage *= stockpile * critical * doubledmg * charge * hh * stab * typeEffectiveness * random;
    return damage;
}
