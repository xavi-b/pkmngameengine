#ifndef BAGSCENE_H
#define BAGSCENE_H

#include "item.h"
#include "itemdef.h"
#include "pkmn.h"
#include "scene.h"
#include "textspeech.h"

#include <SDL_image.h>
#include <array>
#include <vector>

class BagScene : public Scene
{
public:
    enum class Mode
    {
        Field,
        Battle,
        Give
    };

    BagScene(SDL_Renderer* renderer, Item::ItemPtr& selectedItem, Pkmn::PkmnPtr& selectedPkmn, Mode mode = Mode::Field);
    virtual ~BagScene();

    virtual void update(Inputs const* inputs) override;
    virtual void draw(Fps const* fps, RenderSizes rs) override;

    virtual bool                   popScene() const override;
    virtual bool                   pushScene() const override;
    virtual void                   popReset() override;
    virtual std::unique_ptr<Scene> nextScene() override;

    virtual std::string name() override;

private:
    std::vector<std::pair<SDL_Surface*, SDL_Texture*>> backgrounds;
    std::vector<std::pair<SDL_Surface*, SDL_Texture*>> bags;

    SDL_Surface* iconPocketSurface;
    SDL_Texture* iconPocketTexture;
    SDL_Surface* cursorSurface;
    SDL_Texture* cursorTexture;
    SDL_Surface* itemSurface = NULL;
    SDL_Texture* itemTexture = NULL;

    bool           leave = false;
    Item::ItemPtr& selectedItem;
    Pkmn::PkmnPtr& selectedPkmn;
    Item::ItemPtr  pendingItem        = nullptr;
    bool           openPkmnScene      = false;
    bool           allowFaintedTarget = false;
    Mode           mode               = Mode::Field;

    std::unique_ptr<TextSpeech> cantHoldSpeech;

    size_t currentPocketIndex = 0;

    std::array<size_t, ItemDef::NumberOfPockets> indices = {0};
};

#endif // BAGSCENE_H
