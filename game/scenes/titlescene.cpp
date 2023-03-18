#include "titlescene.h"

#include "titlemenu.h"

TitleScene::TitleScene(SDL_Renderer* renderer)
    : Scene(renderer)
{
    music        = Mix_LoadMUS("resources/Audio/BGM/Title.ogg");
    bgSurface    = IMG_Load("resources/Graphics/Titles/title.png");
    bgTexture    = SDL_CreateTextureFromSurface(renderer, bgSurface);
    startSurface = IMG_Load("resources/Graphics/Titles/start.png");
    startTexture = SDL_CreateTextureFromSurface(renderer, startSurface);
}

TitleScene::~TitleScene()
{
    Mix_FreeMusic(music);
    SDL_DestroyTexture(bgTexture);
    SDL_FreeSurface(bgSurface);
    SDL_DestroyTexture(startTexture);
    SDL_FreeSurface(startSurface);
}

void TitleScene::init()
{
    Mix_PlayMusic(music, -1);
}

void TitleScene::sendInputs(const Inputs* inputs)
{
    if (inputs->A || inputs->start)
        goToNextScene = true;
}

void TitleScene::draw(const Fps* fps)
{
    SDL_RenderCopy(renderer, bgTexture, NULL, NULL);
    if (fps->tick)
        alternate = !alternate;
    if (alternate)
    {
        int w, h;
        SDL_GetRendererOutputSize(renderer, &w, &h);

        SDL_Rect rect;
        rect.x = 0;
        rect.y = 0.8 * h;
        rect.w = startSurface->w * w / bgSurface->w;
        rect.h = startSurface->h * h / bgSurface->h;
        SDL_RenderCopy(renderer, startTexture, NULL, &rect);
    }
}

std::unique_ptr<Scene> TitleScene::nextScene() const
{
    if (goToNextScene)
        return std::make_unique<TitleMenu>(renderer);
    return nullptr;
}
