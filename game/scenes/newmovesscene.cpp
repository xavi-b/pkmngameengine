#include "newmovesscene.h"

#include "game.h"
#include "renderutils.h"
#include "utils.h"

#include <SDL_image.h>
#include <algorithm>
#include <boost/format.hpp>

namespace
{

constexpr int TypeIconW     = 64;
constexpr int TypeIconH     = 28;
constexpr int TypeIconCount = 19;

int typeIconIndex(std::string const& typeId)
{
    if (typeId.empty())
        return -1;

    auto const& types = Game::instance()->data.types;
    for (size_t i = 0; i < types.size(); ++i)
    {
        if (types[i] && types[i]->getId() == typeId)
            return static_cast<int>(i);
    }
    return -1;
}

} // namespace

bool MoveLearn::knowsMove(Pkmn::PkmnPtr const& pkmn, std::string const& moveId)
{
    if (!pkmn || moveId.empty())
        return false;

    for (auto const& move : pkmn->getMoves())
    {
        if (move && move->getDefinition() && move->getDefinition()->getId() == moveId)
            return true;
    }
    return false;
}

int MoveLearn::firstEmptySlot(Pkmn::PkmnPtr const& pkmn)
{
    if (!pkmn)
        return -1;

    auto const& moves = pkmn->getMoves();
    for (int i = 0; i < static_cast<int>(moves.size()); ++i)
    {
        if (!moves.at(i))
            return i;
    }
    return -1;
}

void MoveLearn::teachMove(Pkmn::PkmnPtr const& pkmn, MoveDef::MoveDefPtr const& move, int slot)
{
    if (!pkmn || !move || slot < 0)
        return;

    auto& moves = pkmn->getMoves();
    if (slot >= static_cast<int>(moves.size()))
        return;

    auto learned = std::make_shared<Move>(move);
    learned->setCurrentPP(move->getTotalPP());
    moves.at(slot) = learned;
}

void MoveLearn::queueMoveIds(Pkmn::PkmnPtr const&         pkmn,
                             std::vector<std::string>&    pending,
                             std::set<std::string> const& skip)
{
    if (!pkmn || !pkmn->getDefinition())
        return;

    for (auto const& entry : pkmn->getDefinition()->getMovesToLearn())
    {
        if (entry.level != pkmn->getLevel())
            continue;
        if (entry.id.empty() || skip.count(entry.id) > 0)
            continue;
        if (knowsMove(pkmn, entry.id))
            continue;
        if (std::find(pending.begin(), pending.end(), entry.id) != pending.end())
            continue;
        pending.push_back(entry.id);
    }
}

NewMovesScene::NewMovesScene(SDL_Renderer* renderer, Pkmn::PkmnPtr const& pkmn, MoveDef::MoveDefPtr const& move)
    : Scene(renderer), pkmn(pkmn), newMove(move)
{
    typesSurface = IMG_Load("resources/Graphics/UI/types.png");
    typesTexture = SDL_CreateTextureFromSurface(renderer, typesSurface);
    if (typesTexture)
        SDL_SetTextureBlendMode(typesTexture, SDL_BLENDMODE_BLEND);

    speech   = std::make_unique<TextSpeech>(renderer);
    question = std::make_unique<TextQuestion>(renderer);
    question->setTexts({lc::translate("Yes"), lc::translate("No")});
    question->init();

    if (!pkmn || !newMove)
    {
        leave = true;
        return;
    }

    if (MoveLearn::knowsMove(pkmn, newMove->getId()))
    {
        boost::format text =
            boost::format(lc::translate("%1% already knows %2%.")) % pkmn->getDisplayName() % newMove->getName();
        showResult({text.str()});
        return;
    }

    int slot = MoveLearn::firstEmptySlot(pkmn);
    if (slot >= 0)
    {
        MoveLearn::teachMove(pkmn, newMove, slot);
        boost::format text =
            boost::format(lc::translate("%1% learned %2% !")) % pkmn->getDisplayName() % newMove->getName();
        showResult({text.str()});
        return;
    }

    beginAskReplace();
}

NewMovesScene::~NewMovesScene()
{
    SDL_DestroyTexture(typesTexture);
    SDL_FreeSurface(typesSurface);
}

void NewMovesScene::showPrompt(std::vector<std::string> const& pages)
{
    speech->setTexts(pages);
    speech->reset();
    speech->start();
    question->setTexts({lc::translate("Yes"), lc::translate("No")});
    question->reset();
}

void NewMovesScene::showResult(std::vector<std::string> const& pages)
{
    step = Step::Result;
    speech->setTexts(pages);
    speech->reset();
    speech->start();
}

void NewMovesScene::beginAskReplace()
{
    step = Step::AskReplace;
    boost::format wants =
        boost::format(lc::translate("%1% wants to learn %2%.")) % pkmn->getDisplayName() % newMove->getName();
    boost::format full = boost::format(lc::translate("But %1% already knows 4 moves.")) % pkmn->getDisplayName();
    boost::format ask =
        boost::format(lc::translate("Should a move be forgotten to make room for %1%?")) % newMove->getName();
    showPrompt({wants.str(), full.str(), ask.str()});
}

void NewMovesScene::beginAskStop()
{
    step              = Step::AskStop;
    boost::format ask = boost::format(lc::translate("Stop learning %1%?")) % newMove->getName();
    showPrompt({ask.str()});
}

void NewMovesScene::beginConfirmReplace()
{
    step = Step::ConfirmReplace;
    boost::format ask =
        boost::format(lc::translate("Forget %1% and learn %2%?")) % moveNameAt(cursor) % newMove->getName();
    showPrompt({ask.str()});
}

void NewMovesScene::beginLearned(std::string const& forgottenName)
{
    boost::format forgot = boost::format(lc::translate("%1% forgot %2%.")) % pkmn->getDisplayName() % forgottenName;
    boost::format learned =
        boost::format(lc::translate("And... %1% learned %2% !")) % pkmn->getDisplayName() % newMove->getName();
    showResult({lc::translate("1, 2, and... Poof!"), forgot.str(), learned.str()});
}

void NewMovesScene::beginDeclined()
{
    boost::format text =
        boost::format(lc::translate("%1% did not learn %2%.")) % pkmn->getDisplayName() % newMove->getName();
    showResult({text.str()});
}

bool NewMovesScene::prompting() const
{
    return step == Step::AskReplace || step == Step::AskStop || step == Step::ConfirmReplace;
}

MoveDef::MoveDefPtr NewMovesScene::highlightedMove() const
{
    if (!pkmn || !newMove)
        return nullptr;
    if (cursor < pkmn->getMoves().size())
    {
        auto move = pkmn->getMoves().at(cursor);
        if (move)
            return move->getDefinition();
        return nullptr;
    }
    return newMove;
}

std::string NewMovesScene::moveNameAt(size_t index) const
{
    if (!pkmn)
        return "";
    if (index < pkmn->getMoves().size())
    {
        auto move = pkmn->getMoves().at(index);
        if (move && move->getDefinition())
            return move->getDefinition()->getName();
        return "---";
    }
    if (newMove)
        return newMove->getName();
    return "";
}

void NewMovesScene::update(Inputs const* inputs)
{
    if (leave || !pkmn || !newMove)
    {
        leave = true;
        return;
    }

    if (step == Step::Choose)
    {
        inputs->setHandler(inputId());
        size_t const optionCount = pkmn->getMoves().size() + 1;
        if (inputs->A)
        {
            if (cursor < pkmn->getMoves().size())
                beginConfirmReplace();
            else
                beginAskStop();
        }
        else if (inputs->B)
        {
            beginAskStop();
        }
        else if (inputs->up)
        {
            if (cursor > 0)
                --cursor;
        }
        else if (inputs->down)
        {
            if (cursor + 1 < optionCount)
                ++cursor;
        }
        return;
    }

    if (step == Step::Result)
    {
        inputs->setHandler(speech->inputId());
        speech->update(inputs);
        if (speech->shouldClose())
            leave = true;
        return;
    }

    if (!prompting())
        return;

    if (speech->mayClose())
    {
        inputs->setHandler(question->inputId());
        question->update(inputs);
    }
    else
    {
        inputs->setHandler(speech->inputId());
        speech->update(inputs);
    }

    if (!question->isFinished())
        return;

    bool yes = question->selectedIndex() == 0;
    question->reset();

    switch (step)
    {
    case Step::AskReplace:
        if (yes)
            step = Step::Choose;
        else
            beginAskStop();
        break;
    case Step::AskStop:
        if (yes)
            beginDeclined();
        else
            beginAskReplace();
        break;
    case Step::ConfirmReplace:
        if (yes && cursor < pkmn->getMoves().size())
        {
            std::string forgotten = moveNameAt(cursor);
            MoveLearn::teachMove(pkmn, newMove, static_cast<int>(cursor));
            beginLearned(forgotten);
        }
        else
        {
            step = Step::Choose;
        }
        break;
    case Step::Choose:
    case Step::Result:
        break;
    }
}

void NewMovesScene::drawMoveSelection(RenderSizes rs)
{
    auto toRect = [&](int x, int y, int w, int h) {
        SDL_Rect rect;
        rect.x = x * rs.ww / rs.aw;
        rect.y = y * rs.wh / rs.ah;
        rect.w = w * rs.ww / rs.aw;
        rect.h = h * rs.wh / rs.ah;
        return rect;
    };
    auto fill = [&](SDL_Rect const& rect, SDL_Color color) {
        SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, color.a);
        SDL_RenderFillRect(renderer, &rect);
    };

    // GBA summary moves page, scaled onto the 512x384 logical screen:
    // header, level and nickname, detail pane on the left, five move rows on the right.
    constexpr int HeaderH = 42;
    constexpr int ListX   = 250;
    constexpr int RowH    = 66;
    constexpr int Rows    = 5;

    fill(toRect(ListX - 6, HeaderH, 512 - ListX + 6, 384 - HeaderH), {186, 220, 244, 255});
    fill(toRect(0, 0, 512, HeaderH), {200, 48, 32, 255});
    fill(toRect(0, HeaderH - 4, 512, 4), {248, 208, 88, 255});

    SDL_Rect headerText = toRect(16, 8, 0, 0);
    RenderUtils::drawWhiteText(renderer, rs, lc::translate("KNOWN MOVES"), 22, headerText.x, headerText.y);
    SDL_Rect controls = toRect(496, 12, 0, 0);
    RenderUtils::drawWhiteTextRightAligned(renderer, rs, lc::translate("A:PICK   B:BACK"), 16, controls.x, controls.y);

    if (pkmn)
    {
        boost::format levelName =
            boost::format(lc::translate("Lv.%1%  %2%")) % pkmn->getLevel() % pkmn->getDisplayName();
        SDL_Rect nick = toRect(16, HeaderH + 8, 0, 0);
        RenderUtils::drawWhiteText(renderer, rs, levelName.str(), 20, nick.x, nick.y);
    }

    int const infoX = 12;
    int const infoY = HeaderH + 40;
    int const infoW = ListX - 28;
    int const infoH = 384 - infoY - 12;
    fill(toRect(infoX - 3, infoY - 3, infoW + 6, infoH + 6), {32, 64, 128, 255});
    fill(toRect(infoX, infoY, infoW, infoH), {232, 242, 252, 255});

    auto        def      = highlightedMove();
    SDL_Rect    label    = toRect(infoX + 14, infoY + 12, 0, 0);
    int const   line     = 36 * rs.wh / rs.ah;
    std::string power    = "---";
    std::string accuracy = "---";
    if (def)
    {
        if (def->getPower() > 1)
            power = std::to_string(def->getPower());
        if (def->getAccuracy() > 0)
            accuracy = std::to_string(def->getAccuracy());
    }

    RenderUtils::drawGreyText(renderer, rs, lc::translate("POWER"), 20, label.x, label.y);
    RenderUtils::drawGreyText(renderer, rs, lc::translate("ACCURACY"), 20, label.x, label.y + line);
    int const numRight = toRect(infoX + infoW - 14, 0, 0, 0).x;
    RenderUtils::drawGreyTextRightAligned(renderer, rs, power, 20, numRight, label.y);
    RenderUtils::drawGreyTextRightAligned(renderer, rs, accuracy, 20, numRight, label.y + line);

    if (def && !def->getDescription().empty())
    {
        SDL_Rect clip = toRect(infoX + 8, infoY + 92, infoW - 16, infoH - 104);
        SDL_RenderSetClipRect(renderer, &clip);
        SDL_Color color   = {80, 80, 80, 255};
        SDL_Color bgColor = {180, 180, 180, 255};
        SDL_Rect  desc    = toRect(infoX + 14, infoY + 96, 0, 0);
        RenderUtils::drawTextWrapped(renderer,
                                     rs,
                                     def->getDescription(),
                                     color,
                                     bgColor,
                                     18,
                                     desc.x,
                                     desc.y,
                                     infoW - 28);
        SDL_RenderSetClipRect(renderer, nullptr);
    }

    int const iconH = 36;
    int const iconW = iconH * TypeIconW / TypeIconH;
    for (int i = 0; i < Rows; ++i)
    {
        int const  rowY     = HeaderH + 6 + i * RowH;
        bool const selected = static_cast<size_t>(i) == cursor;
        if (selected)
        {
            fill(toRect(ListX, rowY, 512 - ListX - 8, RowH - 8), {232, 48, 40, 255});
            fill(toRect(ListX + 3, rowY + 3, 512 - ListX - 14, RowH - 14), {214, 232, 248, 255});
        }

        MoveDef::MoveDefPtr rowDef;
        std::string         ppText = "--";
        SDL_Color           ppColor{80, 80, 80, 255};
        if (pkmn && static_cast<size_t>(i) < pkmn->getMoves().size())
        {
            auto move = pkmn->getMoves().at(i);
            if (move && move->getDefinition())
            {
                rowDef       = move->getDefinition();
                size_t curPP = move->getCurrentPP();
                size_t maxPP = rowDef->getTotalPP();
                ppText       = (boost::format("%1%/%2%") % curPP % maxPP).str();
                if (maxPP > 0 && curPP == 0)
                    ppColor = {192, 32, 32, 255};
                else if (maxPP > 0 && curPP * 2 <= maxPP)
                    ppColor = {192, 112, 24, 255};
            }
        }
        else if (newMove)
        {
            rowDef = newMove;
            ppText = (boost::format("%1%/%1%") % newMove->getTotalPP()).str();
        }

        if (typesTexture && rowDef)
        {
            int index = typeIconIndex(rowDef->getType());
            if (index >= 0 && index < TypeIconCount)
            {
                SDL_Rect src{0, index * TypeIconH, TypeIconW, TypeIconH};
                int      iconY = rowY + (RowH - 8 - iconH) / 2;
                SDL_Rect dst   = toRect(ListX + 8, iconY, iconW, iconH);
                SDL_RenderCopy(renderer, typesTexture, &src, &dst);
            }
        }

        int const   textX = toRect(ListX + 16 + iconW, 0, 0, 0).x;
        int const   nameY = toRect(0, rowY + 4, 0, 0).y;
        int const   ppY   = toRect(0, rowY + 30, 0, 0).y;
        std::string name  = rowDef ? rowDef->getName() : "-";
        RenderUtils::drawGreyText(renderer, rs, name, 20, textX, nameY);
        auto ppLine = boost::format(lc::translate("PP %1%")) % ppText;
        RenderUtils::drawText(renderer, rs, ppLine.str(), ppColor, {180, 180, 180, 255}, 16, textX, ppY);
    }
}

void NewMovesScene::draw(Fps const* fps, RenderSizes rs)
{
    SDL_SetRenderDrawColor(renderer, 48, 112, 184, 255);
    SDL_RenderClear(renderer);

    if (step == Step::Choose && pkmn && newMove)
        drawMoveSelection(rs);

    if (speech && (prompting() || step == Step::Result))
        speech->draw(fps, rs);
    if (question && prompting() && speech && speech->mayClose())
        question->draw(fps, rs, TextQuestion::RIGHT);
}

bool NewMovesScene::popScene() const
{
    return leave;
}

std::string NewMovesScene::name()
{
    return "NewMovesScene";
}
