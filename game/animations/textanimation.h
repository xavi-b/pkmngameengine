#ifndef TEXTANIMATION_H
#define TEXTANIMATION_H

#include "animation.h"

class TextAnimation : public Animation
{
public:
    TextAnimation(SDL_Renderer* renderer, std::string const& text);

    virtual void start() override;
    virtual void stop() override;
    virtual void incrementTicks() override;
    virtual void draw(Fps const* fps, RenderSizes rs) override;
    virtual void forceEnd() override;

    std::string currentText(Fps const* fps) const;

private:
    static constexpr int NbCharPerTick = 4;

    std::string text;
};

#endif // TEXTANIMATION_H
