#include "pkmnsscene.h"

#include "game.h"
#include "pkmnutils.h"
#include "utils.h"

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

    overlayHpSurface          = IMG_Load("resources/Graphics/UI/Party/overlay_hp.png");
    overlayHpTexture          = SDL_CreateTextureFromSurface(renderer, overlayHpSurface);
    overlayHpBackSurface      = IMG_Load("resources/Graphics/UI/Party/overlay_hp_back.png");
    overlayHpBackTexture      = SDL_CreateTextureFromSurface(renderer, overlayHpBackSurface);
    overlayHpBackFaintSurface = IMG_Load("resources/Graphics/UI/Party/overlay_hp_back_faint.png");
    overlayHpBackFaintTexture = SDL_CreateTextureFromSurface(renderer, overlayHpBackFaintSurface);
    overlayHpBackSwapSurface  = IMG_Load("resources/Graphics/UI/Party/overlay_hp_back_swap.png");
    overlayHpBackSwapTexture  = SDL_CreateTextureFromSurface(renderer, overlayHpBackSwapSurface);

    statusSurface = IMG_Load("resources/Graphics/UI/Battle/icon_statuses.png");
    statusTexture = SDL_CreateTextureFromSurface(renderer, statusSurface);

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

    SDL_DestroyTexture(overlayHpTexture);
    SDL_FreeSurface(overlayHpSurface);
    SDL_DestroyTexture(overlayHpBackTexture);
    SDL_FreeSurface(overlayHpBackSurface);
    SDL_DestroyTexture(overlayHpBackFaintTexture);
    SDL_FreeSurface(overlayHpBackFaintSurface);
    SDL_DestroyTexture(overlayHpBackSwapTexture);
    SDL_FreeSurface(overlayHpBackSwapSurface);

    SDL_DestroyTexture(statusTexture);
    SDL_FreeSurface(statusSurface);

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
    // https://gamefaqs.gamespot.com/gameboy/367023-pokemon-red-version/faqs/64175/hp-bar-colour
    auto overlayYBasedOnPercentageHP = [](float percentageHP) {
        if (percentageHP > 0.5625)
            return 0;
        else if (percentageHP > 0.207)
            return 1;
        else
            return 2;
    };

    auto statusYBasedOnStatus = [](Pkmn::StatusCondition status) {
        switch (status)
        {
        case Pkmn::BURN:
            return 2;
        case Pkmn::FREEZE:
            return 4;
        case Pkmn::PARALYSIS:
            return 3;
        case Pkmn::POISON:
            return 1;
        case Pkmn::BADLY_POISON:
            return 5;
        case Pkmn::SLEEP:
            return 0;
        default:
            return -1;
        }
    };

    SDL_Color genderBgColor = {180, 180, 180, 255};

    int margin    = 10;
    int dstMargin = margin * rs.ww / rs.aw;

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

            SDL_Texture* selectedPanelTexture = NULL;
            SDL_Texture* selectedHpTexture    = NULL;

            if (int(i) == swapIndex)
            {
                if (i == currentIndex)
                    selectedPanelTexture = panelSwapSel2Texture;
                else
                    selectedPanelTexture = panelSwapSelTexture;
                selectedHpTexture = overlayHpBackSwapTexture;
            }
            else
            {
                if (swapIndex >= 0 && i == currentIndex)
                {
                    selectedPanelTexture = panelSwapTexture;
                    selectedHpTexture    = overlayHpBackSwapTexture;
                }
                else
                {
                    if (pkmn->isKO())
                    {
                        if (i == currentIndex)
                            selectedPanelTexture = panelFaintSelTexture;
                        else
                            selectedPanelTexture = panelFaintTexture;
                        selectedHpTexture = overlayHpBackFaintTexture;
                    }
                    else
                    {
                        if (i == currentIndex)
                            selectedPanelTexture = panelSelTexture;
                        else
                            selectedPanelTexture = panelTexture;
                        selectedHpTexture = overlayHpBackTexture;
                    }
                }
            }

            SDL_RenderCopy(renderer, selectedPanelTexture, NULL, &dstPanelRect);

            SDL_Rect dstHpRect = dstPanelRect;
            dstHpRect.w        = overlayHpBackSurface->w * rs.ww / rs.aw;
            dstHpRect.h        = overlayHpBackSurface->h * rs.wh / rs.ah;
            dstHpRect.x += dstPanelRect.w / 3;
            dstHpRect.y += (panelBlankSurface->h - overlayHpBackSurface->h + 10) * rs.wh / rs.ah / 2;
            SDL_RenderCopy(renderer, selectedHpTexture, NULL, &dstHpRect);

            float    percentageHP     = pkmn->getPercentageHP();
            SDL_Rect dstOverlayHpRect = dstHpRect;
            dstOverlayHpRect.x += 32 * rs.ww / rs.aw;
            dstOverlayHpRect.y += 2 * rs.wh / rs.ah;
            dstOverlayHpRect.w = percentageHP * overlayHpSurface->w * rs.ww / rs.aw;
            dstOverlayHpRect.h = overlayHpSurface->h / 3 * rs.wh / rs.ah;
            SDL_Rect srcOverlayHpRect;
            srcOverlayHpRect.w = overlayHpSurface->w;
            srcOverlayHpRect.h = overlayHpSurface->h / 3;
            srcOverlayHpRect.x = 0;
            srcOverlayHpRect.y = overlayYBasedOnPercentageHP(percentageHP) * overlayHpSurface->h / 3;
            SDL_RenderCopy(renderer, overlayHpTexture, &srcOverlayHpRect, &dstOverlayHpRect);

            SDL_Rect dstStatusRect = dstHpRect;
            dstStatusRect.y += dstHpRect.h + dstMargin;
            dstStatusRect.w = statusSurface->w * rs.ww / rs.aw;
            dstStatusRect.h = statusSurface->h * rs.wh / rs.ah / 6;
            SDL_Rect srcStatusRect;
            srcStatusRect.w = statusSurface->w;
            srcStatusRect.h = statusSurface->h / 6;
            srcStatusRect.x = 0;
            srcStatusRect.y = statusYBasedOnStatus(pkmn->getStatusCondition()) * statusSurface->h / 6;
            SDL_RenderCopy(renderer, statusTexture, &srcStatusRect, &dstStatusRect);

            SDL_Rect dstNameRect = dstPanelRect;
            dstNameRect.x        = dstHpRect.x;
            dstNameRect.y += dstMargin;
            RenderUtils::drawWhiteText(renderer,
                                       rs,
                                       pkmn->getDisplayName(),
                                       RenderUtils::TextSize,
                                       dstNameRect.x,
                                       dstNameRect.y);

            SDL_Rect dstLvlRect = dstPanelRect;
            dstLvlRect.x += dstMargin * 2;
            dstLvlRect.y += (panelBlankSurface->h - RenderUtils::TextSize - margin * 1.5) * rs.wh / rs.ah;
            boost::format lvl = boost::format("Lv.%1%") % pkmn->getLevel();
            RenderUtils::drawWhiteText(renderer, rs, lvl.str(), RenderUtils::TextSize, dstLvlRect.x, dstLvlRect.y);

            SDL_Rect dstPlayerHpTextRect = dstHpRect;
            dstPlayerHpTextRect.x += dstHpRect.w;
            dstPlayerHpTextRect.y += dstHpRect.h;
            boost::format playerHp = boost::format("%1% / %2%") % pkmn->getHP() % pkmn->getTotalHP();
            RenderUtils::drawWhiteTextRightAligned(renderer,
                                                   rs,
                                                   playerHp.str(),
                                                   RenderUtils::TextSize,
                                                   dstPlayerHpTextRect.x,
                                                   dstPlayerHpTextRect.y);

            SDL_Rect dstIconRect = dstPanelRect;
            dstIconRect.x += dstMargin;
            dstIconRect.y += (panelBlankSurface->h - pkmnsRendering[pkmn].first->h - 30) * rs.wh / rs.ah / 2;
            dstIconRect.w = pkmnsRendering[pkmn].first->w * rs.ww / rs.aw / 2;
            dstIconRect.h = pkmnsRendering[pkmn].first->h * rs.wh / rs.ah;
            SDL_Rect srcIconRect;
            srcIconRect.w = pkmnsRendering[pkmn].first->w / 2;
            srcIconRect.h = pkmnsRendering[pkmn].first->h;
            srcIconRect.x = ticks % 3 ? 0 : pkmnsRendering[pkmn].first->w / 2;
            srcIconRect.y = 0;
            SDL_RenderCopy(renderer, pkmnsRendering[pkmn].second, &srcIconRect, &dstIconRect);

            SDL_Rect dstGenderRect = dstPanelRect;
            dstGenderRect.x += dstPanelRect.w - dstMargin * 3;
            dstGenderRect.y += dstMargin;
            RenderUtils::drawUnicodeSymbol(renderer,
                                           rs,
                                           PkmnUtils::genderSymbol(pkmn->getGender()),
                                           PkmnUtils::genderColor(pkmn->getGender()),
                                           genderBgColor,
                                           RenderUtils::TextSize,
                                           dstGenderRect.x,
                                           dstGenderRect.y);
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
