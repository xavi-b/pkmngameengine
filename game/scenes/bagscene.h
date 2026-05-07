#ifndef BAGSCENE_H
#define BAGSCENE_H

#include "item.h"
#include "itemdef.h"
#include "scene.h"

#include <SDL_image.h>
#include <array>
#include <vector>

class BagScene : public Scene
{
public:
    BagScene(SDL_Renderer* renderer, Item::ItemPtr& selectedItem);
    virtual ~BagScene();

    virtual void update(Inputs const* inputs) override;
    virtual void draw(Fps const* fps, RenderSizes rs) override;

    virtual bool popScene() const override;

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

    size_t currentPocketIndex = 0;

    std::array<size_t, ItemDef::NumberOfPockets> indices = {0};
};

#endif // BAGSCENE_H
