#include "movepickscene.h"

#include "renderutils.h"

#include <boost/format.hpp>
#include <boost/locale.hpp>

namespace lc = boost::locale;

MovePickScene::MovePickScene(SDL_Renderer* renderer, Pkmn::PkmnPtr const& pkmn, int& slot)
    : Scene(renderer), pkmn(pkmn), slot(slot)
{
    if (!pkmn || pkmn->getMoves().empty())
    {
        slot  = -2;
        leave = true;
    }
}

void MovePickScene::update(Inputs const* inputs)
{
    if (leave || !pkmn)
    {
        leave = true;
        return;
    }

    auto const& moves = pkmn->getMoves();
    if (moves.empty())
    {
        slot  = -2;
        leave = true;
        return;
    }

    inputs->setHandler(inputId());
    if (inputs->up && cursor > 0)
        --cursor;
    else if (inputs->down && cursor + 1 < moves.size())
        ++cursor;
    else if (inputs->B)
    {
        slot  = -2;
        leave = true;
    }
    else if (inputs->A)
    {
        auto move = moves.at(cursor);
        if (move && move->getDefinition())
        {
            slot  = static_cast<int>(cursor);
            leave = true;
        }
    }
}

void MovePickScene::draw(Fps const* /*fps*/, RenderSizes rs)
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

    SDL_SetRenderDrawColor(renderer, 48, 112, 184, 255);
    SDL_RenderClear(renderer);
    fill(toRect(16, 16, 480, 352), {214, 232, 248, 255});

    if (!pkmn)
        return;

    auto const& moves = pkmn->getMoves();
    for (size_t i = 0; i < moves.size(); ++i)
    {
        int const rowY = 32 + static_cast<int>(i) * 80;
        if (i == cursor)
        {
            fill(toRect(28, rowY, 456, 68), {232, 48, 40, 255});
            fill(toRect(32, rowY + 4, 448, 60), {255, 255, 255, 255});
        }

        std::string name = "-";
        std::string pp   = "--";
        auto        move = moves.at(i);
        if (move && move->getDefinition())
        {
            name = move->getDefinition()->getName();
            pp   = (boost::format("%1%/%2%") % move->getCurrentPP() % move->maxPP()).str();
        }
        int const textX = toRect(48, 0, 0, 0).x;
        int const nameY = toRect(0, rowY + 8, 0, 0).y;
        int const ppY   = toRect(0, rowY + 36, 0, 0).y;
        RenderUtils::drawGreyText(renderer, rs, name, 22, textX, nameY);
        RenderUtils::drawGreyText(renderer, rs, (boost::format(lc::translate("PP %1%")) % pp).str(), 18, textX, ppY);
    }
}

bool MovePickScene::popScene() const
{
    return leave;
}

std::string MovePickScene::name()
{
    return "MovePickScene";
}
