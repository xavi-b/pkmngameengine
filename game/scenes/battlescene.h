#ifndef BATTLESCENE_H
#define BATTLESCENE_H

#include "battles/battleactions.h"
#include "battles/battlebackground.h"
#include "battles/battlespeech.h"
#include "battles/moveselection.h"
#include "scene.h"

#include <SDL_image.h>

class BattleScene : public Scene
{
public:
    BattleScene(SDL_Renderer* renderer);
    virtual ~BattleScene();

    virtual void init() override;
    virtual void update(Inputs const* inputs) override;
    virtual void draw(Fps const* fps, RenderSizes rs) override;

    virtual bool popScene() const override;

    virtual std::unique_ptr<Scene> nextScene() override;

    virtual std::string name() override;

protected:
    enum State
    {
        MENU,
        MOVES,
        ITEMS,
        PKMNS,
        RUN
    };

    std::unique_ptr<BattleBackground> battleBackground;
    std::unique_ptr<BattleSpeech>     battleSpeech;
    std::unique_ptr<BattleActions>    battleActions;
    std::unique_ptr<MoveSelection>    moveSelection;

    State state = MENU;
};

#endif // BATTLESCENE_H
