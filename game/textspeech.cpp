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

void TextSpeech::update(const Inputs* inputs)
{
    if (currentAnimation < animations.size())
    {
        if (inputs->A || inputs->B)
        {
            if (!animations[currentAnimation]->isRunning())
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

void TextSpeech::draw(const Fps* fps, RenderSizes rs)
{
    int borderSize    = 14;
    int dstBorderSize = borderSize * rs.wh / rs.ah;

    int height    = 2 * bgSurface->h;
    int dstHeight = height * rs.wh / rs.ah;

    SDL_Rect rect;
    rect.x = 0;
    rect.w = rs.ww;
    rect.h = dstHeight;
    rect.y = rs.wh - rect.h;
    RenderUtils::drawBorderImage(renderer, rs, bgSurface, bgTexture, rect, borderSize, borderSize);

    int fontSize         = 24;
    int textBoxHeight    = 2 /* lines */ * 24;
    int dstTextBoxHeight = textBoxHeight * rs.wh / rs.ah;
    int padding          = (height - textBoxHeight) / 2;
    int dstPadding       = (dstHeight - dstTextBoxHeight) / 2;

    if (currentAnimation < animations.size())
        RenderUtils::drawGreyTextWithIntroWrapped(renderer,
                                                  rs,
                                                  animations[currentAnimation]->currentText(),
                                                  {3, 86, 252, 255},
                                                  {79, 199, 255, 255},
                                                  fontSize,
                                                  rect.x + dstPadding,
                                                  rect.y + dstPadding * 2 - dstBorderSize * 2,
                                                  rs.aw - padding * 2);
}

bool TextSpeech::isFinished() const
{
    return currentAnimation >= animations.size();
}

void TextSpeech::reset()
{
    currentAnimation = 0;
}

void TextSpeech::setTexts(const std::vector<std::string>& texts)
{
    animations.clear();
    for (const auto& text : texts)
        animations.emplace_back(std::make_unique<TextAnimation>(renderer, text));
}
