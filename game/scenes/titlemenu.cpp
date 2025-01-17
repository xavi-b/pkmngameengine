#include "titlemenu.h"

#include "game.h"
#include "introscene.h"
#include "mapscene.h"
#include "pkmnutils.h"
#include "settings.h"
#include "titlescene.h"
#include "utils.h"

TitleMenu::TitleMenu(SDL_Renderer* renderer) : Scene(renderer)
{
    bgSurface    = IMG_Load("resources/Graphics/UI/Load/bg.png");
    bgTexture    = SDL_CreateTextureFromSurface(renderer, bgSurface);
    panelSurface = IMG_Load("resources/Graphics/UI/Load/panels.png");
    panelTexture = SDL_CreateTextureFromSurface(renderer, panelSurface);

    currentIndex = NewGame;

    if (Settings::instance()->savedGame())
    {
        currentIndex = Continue;

        Game::instance()->loadData();

        if (Game::instance()->data.player.getGender() == Player::Gender::BOY)
            playerSurface = IMG_Load("resources/Graphics/Characters/trainer_POKEMONTRAINER_Red.png");
        else
            playerSurface = IMG_Load("resources/Graphics/Characters/trainer_POKEMONTRAINER_Leaf.png");
        playerTexture = SDL_CreateTextureFromSurface(renderer, playerSurface);

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
}

TitleMenu::~TitleMenu()
{
    SDL_DestroyTexture(bgTexture);
    SDL_FreeSurface(bgSurface);
    SDL_DestroyTexture(panelTexture);
    SDL_FreeSurface(panelSurface);
    if (playerTexture)
        SDL_DestroyTexture(playerTexture);
    if (playerSurface)
        SDL_FreeSurface(playerSurface);

    for (auto it = pkmnsRendering.begin(); it != pkmnsRendering.end(); ++it)
    {
        SDL_DestroyTexture(it->second.second);
        SDL_FreeSurface(it->second.first);
    }
}

void TitleMenu::update(Inputs const* inputs)
{
    ++ticks;

    if (inputs->down)
    {
        if (currentIndex < static_cast<char>(Options))
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
            goToGame = true;
            return;
        case NewGame:
            goToIntroScene = true;
            return;
        case Options:
            return;
        default:
            return;
        }
    }
}

void TitleMenu::draw(Fps const* /*fps*/, RenderSizes rs)
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
        auto const& player = Game::instance()->data.player;

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
                                   dstRect.x + 30 * rs.ww / rs.aw,
                                   dstRect.y + (46 * rs.wh / rs.ah - (fontSize + 10) * rs.wh / rs.ah) / 2);

        RenderUtils::drawWhiteTextRightAligned(renderer,
                                               rs,
                                               Game::instance()->data.location,
                                               fontSize,
                                               dstRect.x + dstRect.w - 25 * rs.ww / rs.aw,
                                               dstRect.y + (46 * rs.wh / rs.ah - (fontSize + 10) * rs.wh / rs.ah) / 2);

        SDL_Rect srcPlayerRect;
        srcPlayerRect.x = 0;
        srcPlayerRect.y = 0;
        srcPlayerRect.w = playerSurface->w / 4;
        srcPlayerRect.h = playerSurface->h / 4;
        SDL_Rect dstPlayerRect;
        dstPlayerRect.x = dstRect.x + 45 * rs.ww / rs.aw;
        dstPlayerRect.y = dstRect.y + 2 * (fontSize + 6) * rs.wh / rs.ah;
        dstPlayerRect.w = srcPlayerRect.w * rs.ww / rs.aw;
        dstPlayerRect.h = srcPlayerRect.h * rs.wh / rs.ah;
        SDL_RenderCopy(renderer, playerTexture, &srcPlayerRect, &dstPlayerRect);

        SDL_Color color   = PkmnUtils::genderColor(player.getGender());
        SDL_Color bgColor = PkmnUtils::genderBackgroundColor(player.getGender());
        RenderUtils::drawText(renderer,
                              rs,
                              player.name,
                              color,
                              bgColor,
                              fontSize,
                              dstRect.x + 100 * rs.ww / rs.aw,
                              dstRect.y + 2.2 * (fontSize + 6) * rs.wh / rs.ah);

        RenderUtils::drawWhiteText(renderer,
                                   rs,
                                   lc::translate("Badges:"),
                                   fontSize,
                                   dstRect.x + 30 * rs.ww / rs.aw,
                                   dstRect.y + 4 * (fontSize + 6) * rs.wh / rs.ah);

        RenderUtils::drawWhiteTextRightAligned(renderer,
                                               rs,
                                               "X",
                                               fontSize,
                                               dstRect.x + dstRect.w / 2,
                                               dstRect.y + 4 * (fontSize + 6) * rs.wh / rs.ah);

        RenderUtils::drawWhiteText(renderer,
                                   rs,
                                   lc::translate("Pkdx:"),
                                   fontSize,
                                   dstRect.x + 30 * rs.ww / rs.aw,
                                   dstRect.y + 5 * (fontSize + 6) * rs.wh / rs.ah);

        RenderUtils::drawWhiteTextRightAligned(renderer,
                                               rs,
                                               "X",
                                               fontSize,
                                               dstRect.x + dstRect.w / 2,
                                               dstRect.y + 5 * (fontSize + 6) * rs.wh / rs.ah);

        RenderUtils::drawWhiteText(renderer,
                                   rs,
                                   lc::translate("Time:"),
                                   fontSize,
                                   dstRect.x + 30 * rs.ww / rs.aw,
                                   dstRect.y + 6 * (fontSize + 6) * rs.wh / rs.ah);

        RenderUtils::drawWhiteTextRightAligned(renderer,
                                               rs,
                                               Game::instance()->getFormattedTotalRealTime(),
                                               fontSize,
                                               dstRect.x + dstRect.w / 2,
                                               dstRect.y + 6 * (fontSize + 6) * rs.wh / rs.ah);

        for (size_t i = 0; i < 6; ++i)
        {
            if (i < Game::instance()->data.player.getPkmnCount())
            {
                auto pkmn = Game::instance()->data.player.pkmns[i];

                SDL_Rect srcIconRect;
                srcIconRect.w = pkmnsRendering[pkmn].first->w / 2;
                srcIconRect.h = pkmnsRendering[pkmn].first->h;
                srcIconRect.x = ticks % 3 ? 0 : pkmnsRendering[pkmn].first->w / 2;
                srcIconRect.y = 0;
                SDL_Rect dstIconRect;
                dstIconRect.w     = pkmnsRendering[pkmn].first->w * rs.ww / rs.aw / 2;
                dstIconRect.h     = pkmnsRendering[pkmn].first->h * rs.wh / rs.ah;
                int pkmnZoneWidth = dstRect.w / 4;
                dstIconRect.x =
                    dstRect.x + dstRect.w / 2 + (i % 2 == 0 ? pkmnZoneWidth - dstIconRect.w : pkmnZoneWidth);
                dstIconRect.y = dstRect.y + 1.2 * (fontSize + 6) * rs.wh / rs.ah + i / 2 * dstIconRect.h * 3 / 4;
                SDL_RenderCopy(renderer, pkmnsRendering[pkmn].second, &srcIconRect, &dstIconRect);
            }
        }
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
                               dstRect.x + 30 * rs.ww / rs.aw,
                               dstRect.y + (dstRect.h - (fontSize + 10) * rs.wh / rs.ah) / 2);

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
                               dstRect.x + 30 * rs.ww / rs.aw,
                               dstRect.y + (dstRect.h - (fontSize + 10) * rs.wh / rs.ah) / 2);

    y = y + dstRect.h + spacing;
}

std::unique_ptr<Scene> TitleMenu::nextScene()
{
    if (goToIntroScene)
    {
        goToIntroScene = false;
        return std::make_unique<IntroScene>(renderer);
    }
    if (goToTitleScene)
    {
        goToTitleScene = false;
        return std::make_unique<TitleScene>(renderer);
    }
    if (goToGame)
    {
        goToGame = false;
        return Game::instance()->loadScene();
    }
    return nullptr;
}

std::string TitleMenu::name()
{
    return "TitleMenu";
}
