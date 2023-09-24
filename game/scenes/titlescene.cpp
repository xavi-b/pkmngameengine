#include "titlescene.h"

#include "game.h"
#include "settings.h"
#include "titlemenu.h"

TitleScene::TitleScene(SDL_Renderer* renderer) : Scene(renderer)
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
    if (Settings::instance()->musicOn())
        Mix_PlayMusic(music, -1);
}

void TitleScene::update(Inputs const* inputs)
{
    if (inputs->A || inputs->start)
        goToNextScene = true;
    if (inputs->B)
        Game::instance()->quit();
}

void TitleScene::draw(Fps const* fps, RenderSizes rs)
{
    SDL_RenderCopy(renderer, bgTexture, NULL, NULL);

    static constexpr int ticksDuration = 4;
    if (fps->tick)
        ticks = (ticks + 1) % (ticksDuration * 2);
    if (ticks < ticksDuration)
    {
        SDL_Rect rect;
        rect.x = 0;
        rect.y = 0.8 * rs.wh;
        rect.w = startSurface->w * rs.ww / rs.aw;
        rect.h = startSurface->h * rs.wh / rs.ah;
        SDL_RenderCopy(renderer, startTexture, NULL, &rect);
    }
}

std::unique_ptr<Scene> TitleScene::nextScene()
{
    if (goToNextScene)
        return std::make_unique<TitleMenu>(renderer);
    return nullptr;
}

std::string TitleScene::name()
{
    return "TitleScene";
}
