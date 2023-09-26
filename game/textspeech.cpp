#include "textspeech.h"

#include "renderutils.h"
#include "settings.h"

#include <SDL_ttf.h>
#include <iostream>

TextSpeech::TextSpeech(SDL_Renderer* renderer) : renderer(renderer)
{
    bgSurface = IMG_Load("resources/Graphics/Windowskins/choice 1.png");
    bgTexture = SDL_CreateTextureFromSurface(renderer, bgSurface);
}

TextSpeech::~TextSpeech()
{
    SDL_DestroyTexture(bgTexture);
    SDL_FreeSurface(bgSurface);
}

void TextSpeech::init()
{
    if (currentAnimation < animations.size())
        animations[currentAnimation]->start();
}

void TextSpeech::update(Inputs const* inputs)
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

void TextSpeech::draw(Fps const* /*fps*/, RenderSizes rs)
{
    int borderSize     = TextSpeech::TextBoxBorderSize;
    int dstBorderSizeX = borderSize * rs.ww / rs.aw;

    int height    = 2 * TextSpeech::TextBoxHeight;
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
                                                  {3, 86, 252, 255},
                                                  {79, 199, 255, 255},
                                                  fontSize,
                                                  rect.x + dstPaddingX + dstBorderSizeX,
                                                  rect.y + dstPaddingY + textAdjustY,
                                                  rs.aw - (paddingX + borderSize) * 2);
}

bool TextSpeech::shouldClose() const
{
    return currentAnimation >= animations.size();
}

bool TextSpeech::mayClose() const
{
    return currentAnimation == (animations.size() - 1)
        && (animations.size() == 0 || animations[currentAnimation]->isFinished());
}

void TextSpeech::reset()
{
    currentAnimation = 0;
}

void TextSpeech::setTexts(std::vector<std::string> const& texts)
{
    animations.clear();
    for (auto const& text : texts)
        animations.emplace_back(std::make_unique<TextAnimation>(renderer, text));
}
