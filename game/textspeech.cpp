#include "textspeech.h"

#include "renderutils.h"
#include <SDL_ttf.h>

TextSpeech::TextSpeech(SDL_Renderer* renderer, const std::vector<std::string>& texts) : renderer(renderer)
{
    for (const auto& text : texts)
        animations.emplace_back(std::make_unique<TextAnimation>(renderer, text));

    bgSurface = IMG_Load("resources/Graphics/Windowskins/choice 1.png");
    bgTexture = SDL_CreateTextureFromSurface(renderer, bgSurface);
}

TextSpeech::~TextSpeech()
{
    SDL_DestroyTexture(bgTexture);
    SDL_FreeSurface(bgSurface);
}

void TextSpeech::update(const Inputs* inputs)
{
    // TODO start animation (auto for first animation)
    if (inputs->A || inputs->B)
    {
        if (currentAnimation < animations.size())
        {
            if (!animations[currentAnimation]->isRunning())
                currentAnimation++;
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

    int textBoxHeight    = 2 /* lines */ * RenderUtils::FontSize;
    int dstTextBoxHeight = textBoxHeight * rs.wh / rs.ah;
    int padding          = (height - textBoxHeight) / 2;
    int dstPadding       = (dstHeight - dstTextBoxHeight) / 2;

    if (currentAnimation < animations.size())
        RenderUtils::drawTextWithIntroWrapped(renderer,
                                              rs,
                                              animations[currentAnimation]->text,
                                              {80, 80, 80, 255},
                                              {180, 180, 180, 255},
                                              {3, 86, 252, 255},
                                              {79, 199, 255, 255},
                                              rect.x + dstPadding,
                                              rect.y + dstPadding * 2 - dstBorderSize * 2,
                                              rs.aw - padding * 2);
}

bool TextSpeech::isFinished() const
{
    return currentAnimation >= animations.size();
}
