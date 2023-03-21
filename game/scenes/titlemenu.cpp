#include "titlemenu.h"

#include "introscene.h"
#include "titlescene.h"
#include "game.h"
#include "settings.h"
#include "utils.h"

TitleMenu::TitleMenu(SDL_Renderer* renderer) : Scene(renderer)
{
    bgSurface    = IMG_Load("resources/Graphics/Pictures/loadbg.png");
    bgTexture    = SDL_CreateTextureFromSurface(renderer, bgSurface);
    panelSurface = IMG_Load("resources/Graphics/Pictures/loadPanels.png");
    panelTexture = SDL_CreateTextureFromSurface(renderer, panelSurface);

    currentIndex = static_cast<Index>(!Settings::instance()->savedGame() ? static_cast<char>(NewGame)
                                                                         : static_cast<char>(Continue));
}

TitleMenu::~TitleMenu()
{
    SDL_DestroyTexture(bgTexture);
    SDL_FreeSurface(bgSurface);
    SDL_DestroyTexture(panelTexture);
    SDL_FreeSurface(panelSurface);
}

void TitleMenu::update(const Inputs* inputs)
{
    if (inputs->down)
    {
        if (currentIndex < static_cast<char>(Quit))
            currentIndex = static_cast<Index>(static_cast<char>(currentIndex) + 1);
        return;
    }

    if (inputs->up)
    {
        int lowest = !Settings::instance()->savedGame() ? static_cast<char>(NewGame) : static_cast<char>(Continue);

        if (currentIndex > lowest)
            currentIndex = static_cast<Index>(static_cast<char>(currentIndex) - 1);

        return;
    }

    if (inputs->B)
        goToTitleScene = true;

    if (inputs->A)
    {
        switch (currentIndex)
        {
        case Continue:
            return;
        case NewGame:
            goToIntroScene = true;
            return;
        case Options:
            return;
        case Quit:
            Game::instance()->quit();
            return;
        default:
            return;
        }
    }
}

void TitleMenu::draw(const Fps* /*fps*/, RenderSizes rs)
{
    // Background
    SDL_RenderCopy(renderer, bgTexture, NULL, NULL);

    SDL_Rect srcRect;
    SDL_Rect dstRect;
    dstRect.y = 0;
    dstRect.h = 0;

    int spacing  = 16 * rs.wh / rs.ah;
    int padding  = ((rs.aw - panelSurface->w) / 2) * rs.ww / rs.aw;
    int offset   = 0;
    int y        = spacing;
    int fontSize = 24;

    if (Settings::instance()->savedGame())
    {
        if (currentIndex > static_cast<char>(NewGame))
            offset = spacing + (222 + 46 * 3) * rs.wh / rs.ah + spacing * 4 - rs.wh;

        // Continue
        srcRect.x = 0;
        srcRect.y = 0 + (currentIndex == Continue ? 222 : 0);
        srcRect.w = panelSurface->w;
        srcRect.h = 222;
        dstRect.x = padding;
        dstRect.y = y - offset;
        dstRect.w = srcRect.w * rs.ww / rs.aw;
        dstRect.h = srcRect.h * rs.wh / rs.ah;
        SDL_RenderCopy(renderer, panelTexture, &srcRect, &dstRect);
        RenderUtils::drawWhiteText(renderer,
                                   rs,
                                   lc::translate("Continue"),
                                   fontSize,
                                   dstRect.x + 40 * rs.ww / rs.aw,
                                   dstRect.y + (12 * rs.wh / rs.ah));
    }

    y = y + dstRect.h + spacing;
    // NewGame
    srcRect.x = 0;
    srcRect.y = 222 + 222 + (currentIndex == NewGame ? 46 : 0);
    srcRect.w = panelSurface->w;
    srcRect.h = 46;
    dstRect.x = padding;
    dstRect.y = y - offset;
    dstRect.w = srcRect.w * rs.ww / rs.aw;
    dstRect.h = srcRect.h * rs.wh / rs.ah;
    SDL_RenderCopy(renderer, panelTexture, &srcRect, &dstRect);
    RenderUtils::drawWhiteText(renderer,
                               rs,
                               lc::translate("New Game"),
                               fontSize,
                               dstRect.x + 40 * rs.ww / rs.aw,
                               dstRect.y + (dstRect.h - fontSize * rs.wh / rs.ah) / 2);

    y = y + dstRect.h + spacing;
    // Options
    srcRect.x = 0;
    srcRect.y = 222 + 222 + (currentIndex == Options ? 46 : 0);
    srcRect.w = panelSurface->w;
    srcRect.h = 46;
    dstRect.x = padding;
    dstRect.y = y - offset;
    dstRect.w = srcRect.w * rs.ww / rs.aw;
    dstRect.h = srcRect.h * rs.wh / rs.ah;
    SDL_RenderCopy(renderer, panelTexture, &srcRect, &dstRect);
    RenderUtils::drawWhiteText(renderer,
                               rs,
                               lc::translate("Options"),
                               fontSize,
                               dstRect.x + 40 * rs.ww / rs.aw,
                               dstRect.y + (dstRect.h - fontSize * rs.wh / rs.ah) / 2);

    y = y + dstRect.h + spacing;
    // Quit
    srcRect.x = 0;
    srcRect.y = 222 + 222 + (currentIndex == Quit ? 46 : 0);
    srcRect.w = panelSurface->w;
    srcRect.h = 46;
    dstRect.x = padding;
    dstRect.y = y - offset;
    dstRect.w = srcRect.w * rs.ww / rs.aw;
    dstRect.h = srcRect.h * rs.wh / rs.ah;
    SDL_RenderCopy(renderer, panelTexture, &srcRect, &dstRect);
    RenderUtils::drawWhiteText(renderer,
                               rs,
                               lc::translate("Quit"),
                               fontSize,
                               dstRect.x + 40 * rs.ww / rs.aw,
                               dstRect.y + (dstRect.h - fontSize * rs.wh / rs.ah) / 2);
}

std::unique_ptr<Scene> TitleMenu::nextScene()
{
    if (goToIntroScene)
    {
        goToIntroScene = false;
        return std::make_unique<IntroScene>(renderer);
    }
    if (goToTitleScene)
        return std::make_unique<TitleScene>(renderer);
    return nullptr;
}

std::string TitleMenu::name()
{
    return "TitleMenu";
}
