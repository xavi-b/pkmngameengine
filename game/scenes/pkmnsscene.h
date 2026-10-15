#ifndef PKMNSSCENE_H
#define PKMNSSCENE_H

#include "pkmn.h"
#include "scene.h"
#include "textquestion.h"
#include "textspeech.h"

#include <SDL_image.h>
#include <memory>

class PkmnsScene : public Scene
{
public:
    PkmnsScene(SDL_Renderer*  renderer,
               Pkmn::PkmnPtr& currentPkmn,
               bool           forceSelectedPkmn = false,
               bool           allowFaintedPkmn  = false);
    virtual ~PkmnsScene();

    virtual void update(Inputs const* inputs) override;
    virtual void draw(Fps const* fps, RenderSizes rs) override;

    virtual bool popScene() const override;
    virtual bool pushScene() const override;
    virtual void popReset() override;

    virtual std::unique_ptr<Scene> nextScene() override;

    virtual std::string name() override;

private:
    SDL_Surface* bgSurface;
    SDL_Texture* bgTexture;

    SDL_Surface* panelBlankSurface;
    SDL_Texture* panelBlankTexture;
    SDL_Surface* panelSurface;
    SDL_Texture* panelTexture;
    SDL_Surface* panelFaintSurface;
    SDL_Texture* panelFaintTexture;

    SDL_Surface* panelSelSurface;
    SDL_Texture* panelSelTexture;
    SDL_Surface* panelFaintSelSurface;
    SDL_Texture* panelFaintSelTexture;

    SDL_Surface* panelSwapSurface;
    SDL_Texture* panelSwapTexture;
    SDL_Surface* panelSwapSelSurface;
    SDL_Texture* panelSwapSelTexture;
    SDL_Surface* panelSwapSel2Surface;
    SDL_Texture* panelSwapSel2Texture;

    SDL_Surface* overlayHpSurface;
    SDL_Texture* overlayHpTexture;
    SDL_Surface* overlayHpBackSurface;
    SDL_Texture* overlayHpBackTexture;
    SDL_Surface* overlayHpBackFaintSurface;
    SDL_Texture* overlayHpBackFaintTexture;
    SDL_Surface* overlayHpBackSwapSurface;
    SDL_Texture* overlayHpBackSwapTexture;

    SDL_Surface* statusSurface;
    SDL_Texture* statusTexture;

    SDL_Surface* holdItemSurface = nullptr;
    SDL_Texture* holdItemTexture = nullptr;
    SDL_Surface* holdMailSurface = nullptr;
    SDL_Texture* holdMailTexture = nullptr;

    std::map<Pkmn::PkmnPtr, std::pair<SDL_Surface*, SDL_Texture*>> pkmnsRendering;

    size_t ticks = 0;

    bool           leave             = false;
    bool           selectingPkmn     = false;
    bool           forceSelectedPkmn = false;
    bool           allowFaintedPkmn  = false;
    bool           showCommand       = false;
    bool           showItemMenu      = false;
    bool           showSwitch        = false;
    bool           openInfo          = false;
    bool           openBag           = false;
    Pkmn::PkmnPtr& selectedPkmn;
    Item::ItemPtr  giveItem;
    Pkmn::PkmnPtr  bagTarget;
    size_t         currentIndex = 0;
    int            swapIndex    = -1;

    std::unique_ptr<TextQuestion> command;
    std::unique_ptr<TextQuestion> itemCommand;
    std::unique_ptr<TextQuestion> switchQuestion;
    std::unique_ptr<TextSpeech>   speech;

    Pkmn::PkmnPtr currentPartyPkmn() const;
    void          startSpeech(std::vector<std::string> const& texts);
    void          giveSelectedItem();
    void          takeHeldItem();
    void          switchHeldItem();
};

#endif // PKMNSSCENE_H
