#include "bagscene.h"

#include "game.h"
#include "itemutils.h"
#include "pkmnsscene.h"
#include "utils.h"

#include <boost/format.hpp>

BagScene::BagScene(SDL_Renderer* renderer, Item::ItemPtr& selectedItem, Pkmn::PkmnPtr& selectedPkmn, Mode mode)
    : Scene(renderer), selectedItem(selectedItem), selectedPkmn(selectedPkmn), mode(mode)
{
    selectedItem.reset();
    selectedPkmn.reset();
    auto const& player = Game::instance()->data.player;

    for (size_t i = 0; i < ItemDef::NumberOfPockets; ++i)
    {
        SDL_Surface* surface = IMG_Load(("resources/Graphics/UI/Bag/bg_" + std::to_string(i + 1) + ".png").c_str());
        SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, surface);
        backgrounds.push_back({surface, texture});
    }

    for (size_t i = 0; i < ItemDef::NumberOfPockets; ++i)
    {
        SDL_Surface* surface = IMG_Load(("resources/Graphics/UI/Bag/bag_" + std::to_string(i + 1) + ".png").c_str());
        SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, surface);
        bags.push_back({surface, texture});
    }

    iconPocketSurface = IMG_Load("resources/Graphics/UI/Bag/icon_pocket.png");
    iconPocketTexture = SDL_CreateTextureFromSurface(renderer, iconPocketSurface);

    cursorSurface = IMG_Load("resources/Graphics/UI/Bag/cursor.png");
    cursorTexture = SDL_CreateTextureFromSurface(renderer, cursorSurface);

    auto const& items = player.items[currentPocketIndex];

    if (items.size() > 0)
    {
        auto const& item = items[indices[currentPocketIndex]];

        itemSurface = IMG_Load(("resources/Graphics/Items/" + item->getDefinition()->getId() + ".png").c_str());
        itemTexture = SDL_CreateTextureFromSurface(renderer, itemSurface);
    }

    if (mode == Mode::Give)
        cantHoldSpeech = std::make_unique<TextSpeech>(renderer);
}

BagScene::~BagScene()
{
    for (auto& e : backgrounds)
    {
        SDL_DestroyTexture(e.second);
        SDL_FreeSurface(e.first);
    }

    for (auto& e : bags)
    {
        SDL_DestroyTexture(e.second);
        SDL_FreeSurface(e.first);
    }

    SDL_DestroyTexture(iconPocketTexture);
    SDL_FreeSurface(iconPocketSurface);

    SDL_DestroyTexture(cursorTexture);
    SDL_FreeSurface(cursorSurface);

    if (itemTexture)
        SDL_DestroyTexture(itemTexture);
    if (itemSurface)
        SDL_FreeSurface(itemSurface);
}

void BagScene::update(Inputs const* inputs)
{
    if (cantHoldSpeech && cantHoldSpeech->isStarted() && !cantHoldSpeech->shouldClose())
    {
        inputs->setHandler(cantHoldSpeech->inputId());
        cantHoldSpeech->update(inputs);
        return;
    }
    if (cantHoldSpeech && cantHoldSpeech->shouldClose())
    {
        cantHoldSpeech->reset();
        return;
    }

    inputs->setHandler(inputId());

    auto const& player = Game::instance()->data.player;

    if (inputs->A)
    {
        auto const& items = player.items[currentPocketIndex];
        if (!items.empty() && indices[currentPocketIndex] < items.size())
        {
            auto const& item = items[indices[currentPocketIndex]];
            auto        def  = item ? item->getDefinition() : nullptr;

            if (!def)
                return;

            switch (mode)
            {
            case Mode::Give:
                if (def->hasFlag(ItemDef::Flag::KeyItem))
                {
                    cantHoldSpeech->setTexts(
                        {(boost::format(lc::translate("The %1% can't be held.")) % def->getName()).str()});
                    cantHoldSpeech->reset();
                    cantHoldSpeech->start();
                    return;
                }
                break;
            case Mode::Battle:
                if (def->getBattleUse() == ItemDef::BattleUse::OnMove
                    || def->getBattleUse() == ItemDef::BattleUse::OnPkmn)
                {
                    pendingItem = item;
                    selectedPkmn.reset();
                    allowFaintedTarget = ItemUtils::allowFaintedTarget(item);
                    openPkmnScene      = def->getBattleUse() == ItemDef::BattleUse::OnPkmn;
                    return;
                }
                break;
            case Mode::Field:
                if (def->getFieldUse() == ItemDef::FieldUse::OnPkmn
                    || def->getFieldUse() == ItemDef::FieldUse::TR
                    || def->getFieldUse() == ItemDef::FieldUse::HM)
                {
                    pendingItem = item;
                    selectedPkmn.reset();
                    allowFaintedTarget = ItemUtils::allowFaintedTarget(item)
                                         || def->getFieldUse() == ItemDef::FieldUse::TR
                                         || def->getFieldUse() == ItemDef::FieldUse::HM;
                    openPkmnScene = true;
                    return;
                }
                break;
            }

            selectedItem = item;
            leave        = true;
        }
        return;
    }

    if (inputs->B)
    {
        selectedItem.reset();
        leave = true;
        return;
    }

    bool changed = false;
    if (inputs->left)
    {
        currentPocketIndex = (currentPocketIndex + ItemDef::NumberOfPockets - 1) % ItemDef::NumberOfPockets;
        changed            = true;
    }
    else if (inputs->right)
    {
        currentPocketIndex = (currentPocketIndex + ItemDef::NumberOfPockets + 1) % ItemDef::NumberOfPockets;
        changed            = true;
    }
    else if (inputs->up)
    {
        if (indices[currentPocketIndex] > 0)
            indices[currentPocketIndex]--;
        changed = true;
    }
    else if (inputs->down)
    {
        if (indices[currentPocketIndex] < player.items[currentPocketIndex].size() - 1)
            indices[currentPocketIndex]++;
        changed = true;
    }

    if (changed)
    {
        auto const& items = player.items[currentPocketIndex];

        if (items.size() > 0)
        {
            auto const& item = items[indices[currentPocketIndex]];

            if (itemTexture)
                SDL_DestroyTexture(itemTexture);
            if (itemSurface)
                SDL_FreeSurface(itemSurface);

            itemSurface = IMG_Load(("resources/Graphics/Items/" + item->getDefinition()->getId() + ".png").c_str());
            itemTexture = SDL_CreateTextureFromSurface(renderer, itemSurface);
        }
    }
}

void BagScene::draw(Fps const* fps, RenderSizes rs)
{
    auto const& player = Game::instance()->data.player;

    int fontSize = 24;

    SDL_Surface* surface;
    SDL_Texture* texture;

    SDL_Rect srcRect;
    SDL_Rect dstRect;

    surface   = backgrounds[currentPocketIndex].first;
    texture   = backgrounds[currentPocketIndex].second;
    dstRect.x = 0;
    dstRect.y = 0;
    dstRect.w = surface->w * rs.ww / rs.aw;
    dstRect.h = surface->h * rs.wh / rs.ah;
    SDL_RenderCopy(renderer, texture, NULL, &dstRect);

    surface   = bags[currentPocketIndex].first;
    texture   = bags[currentPocketIndex].second;
    dstRect.x = (184 - surface->w) * rs.ww / rs.aw / 2;
    dstRect.y = (184 - surface->h) * rs.wh / rs.ah / 2;
    dstRect.w = surface->w * rs.ww / rs.aw;
    dstRect.h = surface->h * rs.wh / rs.ah;
    SDL_RenderCopy(renderer, texture, NULL, &dstRect);

    srcRect.x = 28 * currentPocketIndex;
    srcRect.y = 0;
    srcRect.w = 28;
    srcRect.h = 28;
    dstRect.x = (2 + 22 * currentPocketIndex) * rs.ww / rs.aw;
    dstRect.y = 226 * rs.wh / rs.ah;
    dstRect.w = 28 * rs.ww / rs.aw;
    dstRect.h = 28 * rs.wh / rs.ah;
    SDL_RenderCopy(renderer, iconPocketTexture, &srcRect, &dstRect);

    auto const& items = player.items[currentPocketIndex];

    for (size_t i = 0; i < items.size(); ++i)
    {
        size_t dstPadding = 30 * rs.ww / rs.aw;

        SDL_Rect dstItemRect;
        dstItemRect.x = 184 * rs.ww / rs.aw;
        dstItemRect.y = 8 * rs.wh / rs.ah;
        dstItemRect.w = cursorSurface->w * rs.ww / rs.aw;
        dstItemRect.h = cursorSurface->h * rs.wh / rs.ah / 2;
        dstItemRect.y += i * dstItemRect.h;

        size_t itemIndex = i;

        if (items.size() > 6)
        {
            if (indices[currentPocketIndex] >= 3 && indices[currentPocketIndex] < items.size() - 3 - 1)
                itemIndex += indices[currentPocketIndex] - 3;
            else if (indices[currentPocketIndex] >= items.size() - 3 - 1)
                itemIndex += items.size() - 3 - 3 - 1;
        }

        if (dstItemRect.y < (8 * rs.wh / rs.ah + 7 * dstItemRect.h))
        {
            auto const& item = items[itemIndex];

            RenderUtils::drawBlackText(renderer,
                                       rs,
                                       item->getDefinition()->getName(),
                                       fontSize,
                                       dstItemRect.x + dstPadding,
                                       dstItemRect.y);

            RenderUtils::drawBlackTextRightAligned(renderer,
                                                   rs,
                                                   "x " + std::to_string(item->getCount()),
                                                   fontSize,
                                                   dstItemRect.x + dstItemRect.w - dstPadding,
                                                   dstItemRect.y);
        }

        if (itemIndex == indices[currentPocketIndex])
        {
            dstItemRect.y -= dstItemRect.h / 2;
            dstItemRect.h *= 2;
            SDL_RenderCopy(renderer, cursorTexture, NULL, &dstItemRect);
        }
    }

    if (items.size() > 0)
    {
        auto const& item = items[indices[currentPocketIndex]];

        dstRect.x = 96 * rs.ww / rs.aw;
        dstRect.w = rs.ww - dstRect.x - 216 * rs.ww / rs.aw;
        dstRect.h = 96 * rs.wh / rs.ah;
        dstRect.y = rs.wh - dstRect.h;

        RenderUtils::drawWhiteTextWrapped(renderer,
                                          rs,
                                          item->getDefinition()->getDescription(),
                                          fontSize,
                                          dstRect.x,
                                          dstRect.y,
                                          dstRect.w);

        if (itemTexture)
        {
            dstRect.x = 24 * rs.ww / rs.aw;
            dstRect.w = 48 * rs.ww / rs.aw;
            dstRect.h = 48 * rs.wh / rs.ah;
            dstRect.y = rs.wh - dstRect.h - dstRect.x;
            SDL_RenderCopy(renderer, itemTexture, NULL, &dstRect);
        }
    }

    if (cantHoldSpeech && cantHoldSpeech->isStarted())
        cantHoldSpeech->draw(fps, rs);
}

bool BagScene::popScene() const
{
    return leave;
}

bool BagScene::pushScene() const
{
    return openPkmnScene;
}

void BagScene::popReset()
{
    openPkmnScene = false;
    if (selectedPkmn)
    {
        selectedItem = pendingItem;
        leave        = true;
    }
    pendingItem.reset();
}

std::unique_ptr<Scene> BagScene::nextScene()
{
    if (!openPkmnScene)
        return nullptr;

    if (!selectedPkmn)
    {
        for (auto const& pkmn : Game::instance()->data.player.pkmns)
        {
            if (pkmn)
            {
                selectedPkmn = pkmn;
                break;
            }
        }
    }
    return std::make_unique<PkmnsScene>(renderer, selectedPkmn, false, allowFaintedTarget);
}

std::string BagScene::name()
{
    return "BagScene";
}
