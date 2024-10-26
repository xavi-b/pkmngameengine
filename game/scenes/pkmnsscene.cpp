#include "pkmnsscene.h"

#include "game.h"

PkmnsScene::PkmnsScene(SDL_Renderer* renderer, Pkmn::PkmnPtr& currentPkmn)
    : Scene(renderer), selectingPkmn(currentPkmn != nullptr), selectedPkmn(currentPkmn)
{
    bgSurface = IMG_Load("resources/Graphics/UI/Party/bg.png");
    bgTexture = SDL_CreateTextureFromSurface(renderer, bgSurface);

    panelBlankSurface = IMG_Load("resources/Graphics/UI/Party/panel_blank.png");
    panelBlankTexture = SDL_CreateTextureFromSurface(renderer, panelBlankSurface);
    panelSurface      = IMG_Load("resources/Graphics/UI/Party/panel_rect.png");
    panelTexture      = SDL_CreateTextureFromSurface(renderer, panelSurface);
    panelFaintSurface = IMG_Load("resources/Graphics/UI/Party/panel_rect_faint.png");
    panelFaintTexture = SDL_CreateTextureFromSurface(renderer, panelFaintSurface);

    panelSelSurface      = IMG_Load("resources/Graphics/UI/Party/panel_rect_sel.png");
    panelSelTexture      = SDL_CreateTextureFromSurface(renderer, panelSelSurface);
    panelFaintSelSurface = IMG_Load("resources/Graphics/UI/Party/panel_rect_faint_sel.png");
    panelFaintSelTexture = SDL_CreateTextureFromSurface(renderer, panelFaintSelSurface);

    panelSwapSurface     = IMG_Load("resources/Graphics/UI/Party/panel_rect_swap.png");
    panelSwapTexture     = SDL_CreateTextureFromSurface(renderer, panelSwapSurface);
    panelSwapSelSurface  = IMG_Load("resources/Graphics/UI/Party/panel_rect_swap_sel.png");
    panelSwapSelTexture  = SDL_CreateTextureFromSurface(renderer, panelSwapSelSurface);
    panelSwapSel2Surface = IMG_Load("resources/Graphics/UI/Party/panel_rect_swap_sel2.png");
    panelSwapSel2Texture = SDL_CreateTextureFromSurface(renderer, panelSwapSel2Surface);

    for (size_t i = 0; i < Game::instance()->data.player.getPkmnCount(); ++i)
    {
        SDL_Surface* pkmnSurface;
        SDL_Texture* pkmnTexture;

        auto pkmn = Game::instance()->data.player.pkmns[i];

        std::string file = "resources/Graphics/Pokemon/Icons/";
        file += pkmn->getDefinition()->getId();
        if (pkmn->isFemale())
        {
            std::string femaleFile = file + ".png";
            pkmnSurface            = IMG_Load(femaleFile.c_str());
            if (pkmnSurface)
            {
                pkmnTexture          = SDL_CreateTextureFromSurface(renderer, pkmnSurface);
                pkmnsRendering[pkmn] = {pkmnSurface, pkmnTexture};

                continue;
            }
        }
        file += ".png";

        pkmnSurface          = IMG_Load(file.c_str());
        pkmnTexture          = SDL_CreateTextureFromSurface(renderer, pkmnSurface);
        pkmnsRendering[pkmn] = {pkmnSurface, pkmnTexture};
    }
}

PkmnsScene::~PkmnsScene()
{
    SDL_DestroyTexture(bgTexture);
    SDL_FreeSurface(bgSurface);

    SDL_DestroyTexture(panelBlankTexture);
    SDL_FreeSurface(panelBlankSurface);
    SDL_DestroyTexture(panelTexture);
    SDL_FreeSurface(panelSurface);
    SDL_DestroyTexture(panelFaintTexture);
    SDL_FreeSurface(panelFaintSurface);

    SDL_DestroyTexture(panelSelTexture);
    SDL_FreeSurface(panelSelSurface);
    SDL_DestroyTexture(panelFaintSelTexture);
    SDL_FreeSurface(panelFaintSelSurface);

    SDL_DestroyTexture(panelSwapTexture);
    SDL_FreeSurface(panelSwapSurface);
    SDL_DestroyTexture(panelSwapSelTexture);
    SDL_FreeSurface(panelSwapSelSurface);
    SDL_DestroyTexture(panelSwapSel2Texture);
    SDL_FreeSurface(panelSwapSel2Surface);

    for (auto it = pkmnsRendering.begin(); it != pkmnsRendering.end(); ++it)
    {
        SDL_DestroyTexture(it->second.second);
        SDL_FreeSurface(it->second.first);
    }
}

void PkmnsScene::update(Inputs const* inputs)
{
    ++ticks;

    if (inputs->A)
    {
        if (selectingPkmn)
        {
            selectedPkmn = Game::instance()->data.player.pkmns[currentIndex];
            if (!selectedPkmn->isKO())
                leave = true;
        }
        else
        {
            if (swapIndex >= 0)
            {
                Game::instance()->data.player.pkmns[currentIndex].swap(Game::instance()->data.player.pkmns[swapIndex]);
                swapIndex = -1;
            }
            else
            {
                swapIndex = currentIndex;
            }
        }
    }
    else if (inputs->B)
    {
        if (swapIndex >= 0)
        {
            swapIndex = -1;
        }
        else
        {
            selectedPkmn = nullptr;
            leave        = true;
        }
    }
    else if (inputs->left)
    {
        if (currentIndex % 2 == 1)
            currentIndex = currentIndex - 1;
    }
    else if (inputs->right)
    {
        if (currentIndex % 2 == 0 && currentIndex < Game::instance()->data.player.getPkmnCount() - 1)
            currentIndex = currentIndex + 1;
    }
    else if (inputs->up)
    {
        if (currentIndex > 1)
            currentIndex = currentIndex - 2;
    }
    else if (inputs->down)
    {
        if (currentIndex < Game::instance()->data.player.getPkmnCount() - 2)
            currentIndex = currentIndex + 2;
    }
}

void PkmnsScene::draw(Fps const* /*fps*/, RenderSizes rs)
{
    SDL_Rect dstRect;

    dstRect.w = bgSurface->w * rs.ww / rs.aw;
    dstRect.h = bgSurface->h * rs.wh / rs.ah;
    dstRect.x = 0;
    dstRect.y = 0;
    SDL_RenderCopy(renderer, bgTexture, NULL, &dstRect);

    for (size_t i = 0; i < 6; ++i)
    {
        SDL_Rect dstPanelRect;
        dstPanelRect.w = panelBlankSurface->w * rs.ww / rs.aw;
        dstPanelRect.h = panelBlankSurface->h * rs.wh / rs.ah;
        dstPanelRect.x = i % 2 == 0 ? 0 : (dstRect.w - dstPanelRect.w);
        dstPanelRect.y = i / 2 * dstPanelRect.h + (i % 2 == 0 ? 0 : 16 * rs.ww / rs.aw) - i * rs.ww / rs.aw;

        if (i >= Game::instance()->data.player.getPkmnCount())
        {
            SDL_RenderCopy(renderer, panelBlankTexture, NULL, &dstPanelRect);
        }
        else
        {
            auto pkmn = Game::instance()->data.player.pkmns[i];

            if (int(i) == swapIndex)
            {
                if (i == currentIndex)
                    SDL_RenderCopy(renderer, panelSwapSel2Texture, NULL, &dstPanelRect);
                else
                    SDL_RenderCopy(renderer, panelSwapSelTexture, NULL, &dstPanelRect);
            }
            else
            {
                if (swapIndex >= 0 && i == currentIndex)
                {
                    SDL_RenderCopy(renderer, panelSwapTexture, NULL, &dstPanelRect);
                }
                else
                {
                    if (pkmn->isKO())
                    {
                        if (i == currentIndex)
                            SDL_RenderCopy(renderer, panelFaintSelTexture, NULL, &dstPanelRect);
                        else
                            SDL_RenderCopy(renderer, panelFaintTexture, NULL, &dstPanelRect);
                    }
                    else
                    {
                        if (i == currentIndex)
                            SDL_RenderCopy(renderer, panelSelTexture, NULL, &dstPanelRect);
                        else
                            SDL_RenderCopy(renderer, panelTexture, NULL, &dstPanelRect);
                    }
                }
            }

            SDL_Rect dstIconRect = dstPanelRect;
            dstIconRect.x += 10 * rs.ww / rs.aw;
            dstIconRect.y += (panelBlankSurface->h - pkmnsRendering[pkmn].first->h) * rs.wh / rs.ah / 2;
            dstIconRect.w = pkmnsRendering[pkmn].first->w * rs.ww / rs.aw / 2;
            dstIconRect.h = pkmnsRendering[pkmn].first->h * rs.wh / rs.ah;
            SDL_Rect srcIconRect;
            srcIconRect.w = pkmnsRendering[pkmn].first->w / 2;
            srcIconRect.h = pkmnsRendering[pkmn].first->h;
            srcIconRect.x = ticks % 3 ? 0 : pkmnsRendering[pkmn].first->w / 2;
            srcIconRect.y = 0;
            SDL_RenderCopy(renderer, pkmnsRendering[pkmn].second, &srcIconRect, &dstIconRect);
        }
    }
}

bool PkmnsScene::popScene() const
{
    return leave;
}

std::string PkmnsScene::name()
{
    return "PkmnsScene";
}

void PkmnsScene::selectPkmn(Pkmn::PkmnPtr newSelectedPkmn)
{
    selectedPkmn = newSelectedPkmn;
    leave        = true;
}
