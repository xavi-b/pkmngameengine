#include "battlespeech.h"

#include "battleactions.h"
#include "renderutils.h"
#include "settings.h"

#include <SDL_ttf.h>
#include <iostream>

BattleSpeech::BattleSpeech(SDL_Renderer* renderer) : renderer(renderer)
{
    bgSurface = IMG_Load("resources/Graphics/Windowskins/choice 1.png");
    bgTexture = SDL_CreateTextureFromSurface(renderer, bgSurface);
}

BattleSpeech::~BattleSpeech()
{
    SDL_DestroyTexture(bgTexture);
    SDL_FreeSurface(bgSurface);
}

void BattleSpeech::init()
{
    if (currentAnimation < animations.size())
        animations[currentAnimation]->start();
}

void BattleSpeech::update(Inputs const* inputs)
{
    if (currentAnimation < animations.size())
    {
        if (inputs->A || inputs->B)
        {
            if (animations[currentAnimation]->isFinished())
            {
                currentAnimation++;
                if (currentAnimation < animations.size())
                    animations[currentAnimation]->start();
            }
        }
        else if (inputs->start || Settings::instance()->autoText())
        {
            animations[currentAnimation]->forceEnd();
        }
        else
        {
            animations[currentAnimation]->incrementTicks();
        }
    }
}

void BattleSpeech::draw(Fps const* /*fps*/, RenderSizes rs)
{
    int borderSize     = TextSpeech::TextBoxBorderSize;
    int dstBorderSizeX = borderSize * rs.ww / rs.aw;

    int height    = 2 * BattleSpeech::TextBoxSize;
    int dstHeight = height * rs.wh / rs.ah;

    SDL_Rect rect;
    rect.x = 0;
    rect.w = rs.ww;
    rect.h = dstHeight;
    rect.y = rs.wh - rect.h;
    RenderUtils::drawBorderImage(renderer, rs, bgSurface, bgTexture, rect, borderSize, borderSize);

    int fontSize         = RenderUtils::TextSize;
    int textBoxHeight    = 2 /* lines */ * RenderUtils::TextSize;
    int dstTextBoxHeight = textBoxHeight * rs.wh / rs.ah;
    int paddingX         = RenderUtils::TextPadding;
    int dstPaddingX      = paddingX * rs.ww / rs.aw;
    int dstPaddingY      = (dstHeight - dstTextBoxHeight) / 2;

    int textAdjustY = -4 * rs.wh / rs.ah;

    if (currentAnimation < animations.size())
        RenderUtils::drawGreyTextWithIntroWrapped(renderer,
                                                  rs,
                                                  animations[currentAnimation]->currentText(),
                                                  {},
                                                  {},
                                                  fontSize,
                                                  rect.x + dstPaddingX + dstBorderSizeX,
                                                  rect.y + dstPaddingY + textAdjustY,
                                                  rs.aw - BattleActions::TextBoxWidth - (paddingX + borderSize) * 2);
}

bool BattleSpeech::shouldClose() const
{
    return currentAnimation >= animations.size();
}

bool BattleSpeech::mayClose() const
{
    return currentAnimation == (animations.size() - 1)
        && (animations.size() == 0 || animations[currentAnimation]->isFinished());
}

void BattleSpeech::reset()
{
    currentAnimation = 0;
}

void BattleSpeech::setTexts(std::vector<std::string> const& texts)
{
    animations.clear();
    for (auto const& text : texts)
        animations.emplace_back(std::make_unique<TextAnimation>(renderer, text));
}
