#ifndef MOVEPICKSCENE_H
#define MOVEPICKSCENE_H

#include "pkmn.h"
#include "scene.h"

// Chooses one of a Pokémon's moves. Writes the slot, or -2 when cancelled.
class MovePickScene : public Scene
{
public:
    MovePickScene(SDL_Renderer* renderer, Pkmn::PkmnPtr const& pkmn, int& slot);
    virtual ~MovePickScene() = default;

    virtual void update(Inputs const* inputs) override;
    virtual void draw(Fps const* fps, RenderSizes rs) override;

    virtual bool popScene() const override;

    virtual std::string name() override;

private:
    Pkmn::PkmnPtr pkmn;
    int&          slot;
    size_t        cursor = 0;
    bool          leave  = false;
};

#endif // MOVEPICKSCENE_H
