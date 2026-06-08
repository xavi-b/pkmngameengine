#ifndef TOWN1SCENE_H
#define TOWN1SCENE_H

#include "entities/erasableentity.h"
#include "scenes/mapscene.h"
#include "textspeech.h"

#include <memory>
#include <string>

class Town1Scene : public MapScene
{
public:
    Town1Scene(SDL_Renderer* renderer);

    virtual void init() override;
    virtual void draw(Fps const* fps, RenderSizes rs) override;

    virtual std::string name() override;

    virtual std::unique_ptr<Scene> nextScene() override;

    virtual bool shouldShowNightTextures() const override;

    virtual std::string currentLocation() const override;

protected:
    virtual bool updateAmbient(Inputs const* inputs) override;
    virtual bool updateBeforeMovement(Inputs const* inputs) override;
    virtual void updateAfterMovement(Inputs const* inputs) override;
    virtual bool handleActionButton(Inputs const* inputs) override;

private:
    std::unique_ptr<TextSpeech> hiddenItemFoundSpeech;
    std::unique_ptr<TextSpeech> itemFoundSpeech;

    ErasableEntity* itemEntity = nullptr;

    Entity* npc1Entity = nullptr;
    Sprite* npc1Sprite = nullptr;
};

#endif // TOWN1SCENE_H
