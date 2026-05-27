#ifndef ROAD1SCENE_H
#define ROAD1SCENE_H

#include "animations/weather/foganimation.h"
#include "entities/trainer.h"
#include "scenes/mapscene.h"
#include "textspeech.h"

#include <memory>
#include <string>

class Road1Scene : public MapScene
{
public:
    Road1Scene(SDL_Renderer* renderer);

    virtual void init() override;
    virtual void draw(Fps const* fps, RenderSizes rs) override;

    virtual void drawAmbientOverlay(Fps const* fps, RenderSizes rs, size_t offsetX, size_t offsetY) override;

    virtual bool manageEvents() override;

    virtual std::string name() override;

    virtual bool manageTrainers() override;

    virtual std::unique_ptr<Scene> nextScene() override;

    virtual bool shouldShowNightTextures() const override;

    virtual std::string currentLocation() const override;

protected:
    virtual bool updateAmbient(Inputs const* inputs) override;
    virtual bool updateBeforeMovement(Inputs const* inputs) override;
    virtual void updateAfterMovement(Inputs const* inputs) override;
    virtual bool handleActionButton(Inputs const* inputs) override;

private:
    std::unique_ptr<FogAnimation> fogAnimation;
    std::unique_ptr<TextSpeech>   text1Speech;
    std::unique_ptr<TextSpeech>   childSpeech;
    std::unique_ptr<TextSpeech>   trainerSpeech;

    Entity*           childNpc;
    Sprite*           childSprite;
    Entity::Direction childStoppedDirection;

    Entity* ladyNpc;

    Trainer::TrainerPtr trainerNpc;
    Sprite*             trainerSprite;
};

#endif // ROAD1SCENE_H
