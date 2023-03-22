#ifndef TEXTANIMATION_H
#define TEXTANIMATION_H

#include "animation.h"

class TextAnimation : public Animation
{
public:
    TextAnimation(SDL_Renderer* renderer, const std::string& text);

    virtual void incrementTicks() override;
    virtual void draw(const Fps* fps, RenderSizes rs) override;
    virtual void forceEnd() override;

    std::string currentText() const;

private:
    static constexpr int NbCharPerTick = 4;

    std::string text;
};

#endif // TEXTANIMATION_H
