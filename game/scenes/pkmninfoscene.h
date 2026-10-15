#ifndef PKMNINFOSCENE_H
#define PKMNINFOSCENE_H

#include "pkmn.h"
#include "scene.h"

#include <SDL_mixer.h>
#include <array>
#include <string>

// FireRed Pokémon summary (pokemon_summary_screen.c): Info, then the memo,
// skills, known moves, and ribbons. Left/right changes page. Up/down changes
// the party member, or the move while the moves page is open.
class PkmnInfoScene : public Scene
{
public:
    explicit PkmnInfoScene(SDL_Renderer* renderer, size_t partyIndex);
    virtual ~PkmnInfoScene();

    virtual void update(Inputs const* inputs) override;
    virtual void draw(Fps const* fps, RenderSizes rs) override;

    virtual bool popScene() const override;

    virtual std::string name() override;

private:
    enum class Page
    {
        Info,
        Memo,
        Skills,
        Moves,
        Ribbons,
        Count
    };

    Pkmn::PkmnPtr currentPkmn() const;
    void          reloadSprite();
    void          reloadIcon();
    void          reloadItem();
    void          shiftParty(int delta);
    void          drawPage(RenderSizes rs);

    size_t                                                     partyIndex    = 0;
    Page                                                       page          = Page::Info;
    size_t                                                     moveIndex     = 0;
    size_t                                                     switchIndex   = 0;
    bool                                                       choosingMove  = false;
    bool                                                       switchingMove = false;
    bool                                                       leave         = false;
    std::array<SDL_Texture*, static_cast<size_t>(Page::Count)> pageTextures{};
    SDL_Texture*                                               moveDetailTexture = nullptr;
    SDL_Texture*                                               moveCursorTexture = nullptr;
    SDL_Texture*                                               categoryTexture   = nullptr;
    SDL_Texture*                                               typesTexture      = nullptr;
    SDL_Texture*                                               hpTexture         = nullptr;
    SDL_Texture*                                               expTexture        = nullptr;
    SDL_Texture*                                               ballTexture       = nullptr;
    SDL_Texture*                                               shinyTexture      = nullptr;
    SDL_Texture*                                               itemTexture       = nullptr;
    SDL_Texture*                                               iconTexture       = nullptr;
    SDL_Texture*                                               sprite            = nullptr;
    Mix_Chunk*                                                 pageSound         = nullptr;
};

#endif // PKMNINFOSCENE_H
