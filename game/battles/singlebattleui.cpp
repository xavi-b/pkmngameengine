#include "singlebattleui.h"

#include "pkmnutils.h"
#include "textspeech.h"
#include "utils.h"

SingleBattleUi::SingleBattleUi(SDL_Renderer* renderer) : renderer(renderer)
{
    playerUiSurface  = IMG_Load("resources/Graphics/UI/Battle/databox_normal.png");
    playerUiTexture  = SDL_CreateTextureFromSurface(renderer, playerUiSurface);
    foeUiSurface     = IMG_Load("resources/Graphics/UI/Battle/databox_normal_foe.png");
    foeUiTexture     = SDL_CreateTextureFromSurface(renderer, foeUiSurface);
    overlayXpSurface = IMG_Load("resources/Graphics/UI/Battle/overlay_exp.png");
    overlayXpTexture = SDL_CreateTextureFromSurface(renderer, overlayXpSurface);
    overlayHpSurface = IMG_Load("resources/Graphics/UI/Battle/overlay_hp.png");
    overlayHpTexture = SDL_CreateTextureFromSurface(renderer, overlayHpSurface);
}

SingleBattleUi::~SingleBattleUi()
{
    SDL_DestroyTexture(playerUiTexture);
    SDL_FreeSurface(playerUiSurface);
    SDL_DestroyTexture(foeUiTexture);
    SDL_FreeSurface(foeUiSurface);
    SDL_DestroyTexture(overlayXpTexture);
    SDL_FreeSurface(overlayXpSurface);
    SDL_DestroyTexture(overlayHpTexture);
    SDL_FreeSurface(overlayHpSurface);
}

void SingleBattleUi::setFoePkmn(Pkmn::PkmnPtr const& newFoePkmn)
{
    foePkmn = newFoePkmn;
}

void SingleBattleUi::setPlayerPkmn(Pkmn::PkmnPtr const& newPlayerPkmn)
{
    playerPkmn = newPlayerPkmn;
}

void SingleBattleUi::draw(Fps const* /*fps*/, RenderSizes rs)
{
    // render hp
    // render state (paralyzed, frozen, etc)
    // render xp
    // render display name
    // render gender

    int fontSize = RenderUtils::TextSize;

    int bottomMargin    = 2 * TextSpeech::TextBoxHeight;
    int dstBottomMargin = bottomMargin * rs.wh / rs.ah;

    int databoxMargin    = 10;
    int dstDataboxMargin = databoxMargin * rs.wh / rs.ah;

    int textWidth = 0;

    SDL_Color genderColor;
    SDL_Color genderBgColor = {180, 180, 180, 255};

    auto overlayYBasedOnPercentageHP = [](float percentageHP) {
        if (percentageHP > 0.5625)
            return 0;
        else if (percentageHP > 0.207)
            return 1;
        else
            return 2;
    };

    SDL_Rect dstRect;

    /* FOE PKMN */
    dstRect.w = foeUiSurface->w * rs.ww / rs.aw;
    dstRect.h = foeUiSurface->h * rs.wh / rs.ah;
    dstRect.x = 0;
    dstRect.y = dstDataboxMargin;
    SDL_RenderCopy(renderer, foeUiTexture, NULL, &dstRect);

    SDL_Rect dstFoeNameRect = dstRect;
    dstFoeNameRect.x += dstDataboxMargin;
    dstFoeNameRect.y += dstDataboxMargin / 2;
    textWidth = RenderUtils::drawGreyText(renderer,
                                          rs,
                                          foePkmn->getDisplayName(),
                                          fontSize,
                                          dstFoeNameRect.x,
                                          dstFoeNameRect.y);
    dstFoeNameRect.x += textWidth * rs.ww / rs.aw;
    genderColor = PkmnUtils::genderColor(foePkmn->getGender());
    RenderUtils::drawUnicodeSymbol(renderer,
                                   rs,
                                   PkmnUtils::genderSymbol(foePkmn->getGender()),
                                   genderColor,
                                   genderBgColor,
                                   fontSize,
                                   dstFoeNameRect.x,
                                   dstFoeNameRect.y);

    SDL_Rect dstFoeLvlRect = dstRect;
    dstFoeLvlRect.x += dstRect.w - dstDataboxMargin * 4;
    dstFoeLvlRect.y += dstDataboxMargin / 2;
    boost::format foeLvl = boost::format("Lv%1%") % foePkmn->getLevel();
    RenderUtils::drawGreyTextRightAligned(renderer, rs, foeLvl.str(), fontSize, dstFoeLvlRect.x, dstFoeLvlRect.y);

    float    foePercentageHP = foePkmn->getPercentageHP();
    SDL_Rect dstFoeHpRect    = dstRect;
    dstFoeHpRect.x += 118 * rs.ww / rs.aw;
    dstFoeHpRect.y += 40 * rs.wh / rs.ah;
    dstFoeHpRect.w = foePercentageHP * overlayHpSurface->w * rs.ww / rs.aw;
    dstFoeHpRect.h = overlayHpSurface->h / 3 * rs.wh / rs.ah;
    SDL_Rect srcFoeHpRect;
    srcFoeHpRect.w = overlayHpSurface->w;
    srcFoeHpRect.h = overlayHpSurface->h / 3;
    srcFoeHpRect.x = 0;
    srcFoeHpRect.y = overlayYBasedOnPercentageHP(foePercentageHP) * overlayHpSurface->h / 3;
    SDL_RenderCopy(renderer, overlayHpTexture, &srcFoeHpRect, &dstFoeHpRect);

    /* PLAYER PKMN */
    dstRect.w = playerUiSurface->w * rs.ww / rs.aw;
    dstRect.h = playerUiSurface->h * rs.wh / rs.ah;
    dstRect.x = rs.ww - dstRect.w;
    dstRect.y = rs.wh - dstBottomMargin - dstRect.h - dstDataboxMargin;
    SDL_RenderCopy(renderer, playerUiTexture, NULL, &dstRect);

    SDL_Rect dstPlayerNameRect = dstRect;
    dstPlayerNameRect.x += dstDataboxMargin * 4;
    dstPlayerNameRect.y += dstDataboxMargin / 2;
    textWidth = RenderUtils::drawGreyText(renderer,
                                          rs,
                                          playerPkmn->getDisplayName(),
                                          fontSize,
                                          dstPlayerNameRect.x,
                                          dstPlayerNameRect.y);
    dstPlayerNameRect.x += textWidth * rs.ww / rs.aw;
    genderColor = PkmnUtils::genderColor(playerPkmn->getGender());
    RenderUtils::drawUnicodeSymbol(renderer,
                                   rs,
                                   PkmnUtils::genderSymbol(playerPkmn->getGender()),
                                   genderColor,
                                   genderBgColor,
                                   fontSize,
                                   dstPlayerNameRect.x,
                                   dstPlayerNameRect.y);

    SDL_Rect dstPlayerLvlRect = dstRect;
    dstPlayerLvlRect.x += dstRect.w - dstDataboxMargin;
    dstPlayerLvlRect.y += dstDataboxMargin / 2;
    boost::format playerLvl = boost::format("Lv%1%") % playerPkmn->getLevel();
    RenderUtils::drawGreyTextRightAligned(renderer,
                                          rs,
                                          playerLvl.str(),
                                          fontSize,
                                          dstPlayerLvlRect.x,
                                          dstPlayerLvlRect.y);

    float    playerPercentageHP = playerPkmn->getPercentageHP();
    SDL_Rect dstPlayerHpRect    = dstRect;
    dstPlayerHpRect.x += 136 * rs.ww / rs.aw;
    dstPlayerHpRect.y += 40 * rs.wh / rs.ah;
    dstPlayerHpRect.w = playerPercentageHP * overlayHpSurface->w * rs.ww / rs.aw;
    dstPlayerHpRect.h = overlayHpSurface->h / 3 * rs.wh / rs.ah;
    SDL_Rect srcPlayerHpRect;
    srcPlayerHpRect.w = overlayHpSurface->w;
    srcPlayerHpRect.h = overlayHpSurface->h / 3;
    srcPlayerHpRect.x = 0;
    srcPlayerHpRect.y = overlayYBasedOnPercentageHP(playerPercentageHP) * overlayHpSurface->h / 3;
    SDL_RenderCopy(renderer, overlayHpTexture, &srcPlayerHpRect, &dstPlayerHpRect);
}
