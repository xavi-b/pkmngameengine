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
    statusSurface    = IMG_Load("resources/Graphics/UI/Battle/icon_statuses.png");
    statusTexture    = SDL_CreateTextureFromSurface(renderer, statusSurface);
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
    SDL_DestroyTexture(statusTexture);
    SDL_FreeSurface(statusSurface);

    if (foeFrontTexture)
        SDL_DestroyTexture(foeFrontTexture);
    if (foeFrontSurface)
        SDL_FreeSurface(foeFrontSurface);

    if (playerBackTexture)
        SDL_DestroyTexture(playerBackTexture);
    if (playerBackSurface)
        SDL_FreeSurface(playerBackSurface);
}

void SingleBattleUi::setFoePkmn(Pkmn::PkmnPtr const& newFoePkmn)
{
    if (foeFrontTexture)
        SDL_DestroyTexture(foeFrontTexture);
    if (foeFrontSurface)
        SDL_FreeSurface(foeFrontSurface);

    foePkmn          = newFoePkmn;
    std::string file = "resources/Graphics/Pokemon/Front";
    if (newFoePkmn->isShiny())
        file += " shiny";
    file += "/";
    file += newFoePkmn->getDefinition()->getId();
    if (newFoePkmn->isFemale())
    {
        std::string femaleFile = file + ".png";
        foeFrontSurface        = IMG_Load(femaleFile.c_str());
        if (foeFrontSurface)
        {
            foeFrontTexture = SDL_CreateTextureFromSurface(renderer, foeFrontSurface);
            return;
        }
    }
    file += ".png";
    foeFrontSurface = IMG_Load(file.c_str());
    foeFrontTexture = SDL_CreateTextureFromSurface(renderer, foeFrontSurface);
}

void SingleBattleUi::setPlayerPkmn(Pkmn::PkmnPtr const& newPlayerPkmn)
{
    if (playerBackTexture)
        SDL_DestroyTexture(playerBackTexture);
    if (playerBackSurface)
        SDL_FreeSurface(playerBackSurface);

    playerPkmn       = newPlayerPkmn;
    std::string file = "resources/Graphics/Pokemon/Back";
    if (newPlayerPkmn->isShiny())
        file += " shiny";
    file += "/";
    file += newPlayerPkmn->getDefinition()->getId();
    if (newPlayerPkmn->isFemale())
    {
        std::string femaleFile = file + ".png";
        playerBackSurface      = IMG_Load(femaleFile.c_str());
        if (playerBackSurface)
        {
            playerBackTexture = SDL_CreateTextureFromSurface(renderer, playerBackSurface);
            return;
        }
    }
    file += ".png";
    playerBackSurface = IMG_Load(file.c_str());
    playerBackTexture = SDL_CreateTextureFromSurface(renderer, playerBackSurface);
}

void SingleBattleUi::draw(Fps const* /*fps*/, RenderSizes rs)
{
    int fontSize = RenderUtils::TextSize;

    int bottomMargin    = 2 * TextSpeech::TextBoxHeight;
    int dstBottomMargin = bottomMargin * rs.wh / rs.ah;

    int databoxMargin    = 10;
    int dstDataboxMargin = databoxMargin * rs.wh / rs.ah;

    int textWidth = 0;

    SDL_Color genderColor;
    SDL_Color genderBgColor = {180, 180, 180, 255};

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

    SDL_Rect dstFoeStatusRect = dstRect;
    dstFoeStatusRect.x += dstDataboxMargin;
    dstFoeStatusRect.y += 35 * rs.wh / rs.ah;
    dstFoeStatusRect.w = statusSurface->w * rs.ww / rs.aw;
    dstFoeStatusRect.h = statusSurface->h * rs.wh / rs.ah / 6;
    SDL_Rect srcFoeStatusRect;
    srcFoeStatusRect.w = statusSurface->w;
    srcFoeStatusRect.h = statusSurface->h / 6;
    srcFoeStatusRect.x = 0;
    srcFoeStatusRect.y = statusYBasedOnStatus(foePkmn->getStatusCondition()) * statusSurface->h / 6;
    SDL_RenderCopy(renderer, statusTexture, &srcFoeStatusRect, &dstFoeStatusRect);

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

    SDL_Rect dstPlayerStatusRect = dstRect;
    dstPlayerStatusRect.x += dstDataboxMargin * 4;
    dstPlayerStatusRect.y += 35 * rs.wh / rs.ah;
    dstPlayerStatusRect.w = statusSurface->w * rs.ww / rs.aw;
    dstPlayerStatusRect.h = statusSurface->h * rs.wh / rs.ah / 6;
    SDL_Rect srcPlayerStatusRect;
    srcPlayerStatusRect.w = statusSurface->w;
    srcPlayerStatusRect.h = statusSurface->h / 6;
    srcPlayerStatusRect.x = 0;
    srcPlayerStatusRect.y = statusYBasedOnStatus(playerPkmn->getStatusCondition()) * statusSurface->h / 6;
    SDL_RenderCopy(renderer, statusTexture, &srcPlayerStatusRect, &dstPlayerStatusRect);

    float    playerPercentageExp = playerPkmn->getPercentageExp();
    SDL_Rect dstPlayerXpRect     = dstRect;
    dstPlayerXpRect.x += 40 * rs.ww / rs.aw;
    dstPlayerXpRect.y += 74 * rs.wh / rs.ah;
    dstPlayerXpRect.w = playerPercentageExp * overlayXpSurface->w * rs.ww / rs.aw;
    dstPlayerXpRect.h = overlayXpSurface->h * rs.wh / rs.ah;
    SDL_RenderCopy(renderer, overlayXpTexture, NULL, &dstPlayerXpRect);

    SDL_Rect dstPlayerHpTextRect = dstRect;
    dstPlayerHpTextRect.x += 232 * rs.ww / rs.aw;
    dstPlayerHpTextRect.y += (56 - fontSize / 3) * rs.wh / rs.ah;
    boost::format playerHp = boost::format("%1% / %2%") % playerPkmn->getHP() % playerPkmn->getTotalHP();
    RenderUtils::drawGreyTextRightAligned(renderer,
                                          rs,
                                          playerHp.str(),
                                          fontSize * 2 / 3,
                                          dstPlayerHpTextRect.x,
                                          dstPlayerHpTextRect.y);

    /* PKMNS */

    dstRect.w = foeFrontSurface->w * rs.ww / rs.aw;
    dstRect.h = foeFrontSurface->h * rs.wh / rs.ah;
    dstRect.x = rs.ww - dstRect.w * 4 / 3;
    dstRect.y = dstDataboxMargin * 3;
    SDL_RenderCopy(renderer, foeFrontTexture, NULL, &dstRect);

    dstRect.w = playerBackSurface->w * rs.ww / rs.aw;
    dstRect.h = playerBackSurface->h * rs.wh / rs.ah;
    dstRect.x = dstRect.w / 3;
    dstRect.y = rs.wh - dstRect.h - dstBottomMargin;
    SDL_RenderCopy(renderer, playerBackTexture, NULL, &dstRect);
}
