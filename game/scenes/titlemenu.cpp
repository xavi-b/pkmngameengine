#include "titlemenu.h"

#include "titlescene.h"
#include "game.h"

TitleMenu::TitleMenu(SDL_Renderer* renderer)
    : Scene(renderer)
{
    bgSurface    = IMG_Load("resources/Graphics/Pictures/loadbg.png");
    bgTexture    = SDL_CreateTextureFromSurface(renderer, bgSurface);
    panelSurface = IMG_Load("resources/Graphics/Pictures/loadPanels.png");
    panelTexture = SDL_CreateTextureFromSurface(renderer, panelSurface);

    // TODO
    bool savedGame = true;

    currentIndex = static_cast<Index>(!savedGame ? static_cast<char>(NewGame) : static_cast<char>(Continue));
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
        // TODO
        bool savedGame = true;

        int lowest = !savedGame ? static_cast<char>(NewGame) : static_cast<char>(Continue);

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

void TitleMenu::draw(const Fps* fps, int w, int h)
{
    // Background
    SDL_RenderCopy(renderer, bgTexture, NULL, NULL);

    SDL_Rect srcRect;
    SDL_Rect dstRect;
    dstRect.y = 0;
    dstRect.h = 0;

    int spacing = 16 * h / bgSurface->h;
    int padding = ((bgSurface->w - panelSurface->w) / 2) * w / bgSurface->w;
    int offset  = 0;
    int y       = spacing;

    // TODO
    bool savedGame = true;

    if (savedGame)
    {
        if (currentIndex > static_cast<char>(NewGame))
            offset = spacing + (222 + 46 * 3) * h / bgSurface->h + spacing * 4 - h;

        // Continue
        srcRect.x = 0;
        srcRect.y = 0 + (currentIndex == Continue ? 222 : 0);
        srcRect.w = panelSurface->w;
        srcRect.h = 222;
        dstRect.x = padding;
        dstRect.y = y - offset;
        dstRect.w = srcRect.w * w / bgSurface->w;
        dstRect.h = srcRect.h * h / bgSurface->h;
        SDL_RenderCopy(renderer, panelTexture, &srcRect, &dstRect);
    }

    y = y + dstRect.h + spacing;
    // NewGame
    srcRect.x = 0;
    srcRect.y = 222 + 222 + (currentIndex == NewGame ? 46 : 0);
    srcRect.w = panelSurface->w;
    srcRect.h = 46;
    dstRect.x = padding;
    dstRect.y = y - offset;
    dstRect.w = srcRect.w * w / bgSurface->w;
    dstRect.h = srcRect.h * h / bgSurface->h;
    SDL_RenderCopy(renderer, panelTexture, &srcRect, &dstRect);

    y = y + dstRect.h + spacing;
    // Options
    srcRect.x = 0;
    srcRect.y = 222 + 222 + (currentIndex == Options ? 46 : 0);
    srcRect.w = panelSurface->w;
    srcRect.h = 46;
    dstRect.x = padding;
    dstRect.y = y - offset;
    dstRect.w = srcRect.w * w / bgSurface->w;
    dstRect.h = srcRect.h * h / bgSurface->h;
    SDL_RenderCopy(renderer, panelTexture, &srcRect, &dstRect);

    y = y + dstRect.h + spacing;
    // Quit
    srcRect.x = 0;
    srcRect.y = 222 + 222 + (currentIndex == Quit ? 46 : 0);
    srcRect.w = panelSurface->w;
    srcRect.h = 46;
    dstRect.x = padding;
    dstRect.y = y - offset;
    dstRect.w = srcRect.w * w / bgSurface->w;
    dstRect.h = srcRect.h * h / bgSurface->h;
    SDL_RenderCopy(renderer, panelTexture, &srcRect, &dstRect);
}

std::unique_ptr<Scene> TitleMenu::nextScene() const
{
    if (goToTitleScene)
        return std::make_unique<TitleScene>(renderer);
    return nullptr;
}
