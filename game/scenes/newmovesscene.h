#ifndef NEWMOVESSCENE_H
#define NEWMOVESSCENE_H

#include "movedef.h"
#include "pkmn.h"
#include "scene.h"
#include "textquestion.h"
#include "textspeech.h"

#include <set>
#include <string>
#include <vector>

// Shared by battle experience and field level-up (Rare Candy).
namespace MoveLearn
{

bool knowsMove(Pkmn::PkmnPtr const& pkmn, std::string const& moveId);
int  firstEmptySlot(Pkmn::PkmnPtr const& pkmn);
void teachMove(Pkmn::PkmnPtr const& pkmn, MoveDef::MoveDefPtr const& move, int slot);
void queueMoveIds(Pkmn::PkmnPtr const& pkmn, std::vector<std::string>& pending, std::set<std::string> const& skip);

} // namespace MoveLearn

class NewMovesScene : public Scene
{
public:
    NewMovesScene(SDL_Renderer* renderer, Pkmn::PkmnPtr const& pkmn, MoveDef::MoveDefPtr const& move);
    virtual ~NewMovesScene();

    virtual void update(Inputs const* inputs) override;
    virtual void draw(Fps const* fps, RenderSizes rs) override;

    virtual bool popScene() const override;

    virtual std::string name() override;

private:
    enum class Step
    {
        AskReplace,
        AskStop,
        Choose,
        ConfirmReplace,
        Result
    };

    void showPrompt(std::vector<std::string> const& pages);
    void showResult(std::vector<std::string> const& pages);
    void beginAskReplace();
    void beginAskStop();
    void beginConfirmReplace();
    void beginLearned(std::string const& forgottenName);
    void beginDeclined();

    bool                prompting() const;
    MoveDef::MoveDefPtr highlightedMove() const;
    std::string         moveNameAt(size_t index) const;
    void                drawMoveSelection(RenderSizes rs);

    Pkmn::PkmnPtr                 pkmn;
    MoveDef::MoveDefPtr           newMove;
    std::unique_ptr<TextSpeech>   speech;
    std::unique_ptr<TextQuestion> question;
    SDL_Surface*                  typesSurface = nullptr;
    SDL_Texture*                  typesTexture = nullptr;
    Step                          step         = Step::AskReplace;
    size_t                        cursor       = 0;
    bool                          leave        = false;
};

#endif // NEWMOVESSCENE_H
