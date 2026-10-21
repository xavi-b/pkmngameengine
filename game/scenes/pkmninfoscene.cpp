#include "pkmninfoscene.h"

#include "game.h"
#include "pkmnutils.h"
#include "renderutils.h"
#include "utils.h"

#include <SDL_image.h>
#include <boost/format.hpp>
#include <filesystem>
#include <utility>
#include <vector>

namespace
{

constexpr int TypeIconW = 64;
constexpr int TypeIconH = 28;
constexpr int Font      = 24;

// Move rows on bg_4. Each row is 62px: a name band, then a PP band.
constexpr int MoveTop[4] = {96, 160, 224, 288};

constexpr int InfoValueRight   = 490;
constexpr int SkillsValueRight = 474;

int sx(RenderSizes rs, int logical)
{
    return logical * rs.ww / rs.aw;
}

int sy(RenderSizes rs, int logical)
{
    return logical * rs.wh / rs.ah;
}

void blitContained(SDL_Renderer* renderer, SDL_Texture* texture, RenderSizes rs, int x, int y, int boxW, int boxH)
{
    int w = 0;
    int h = 0;
    if (!texture || SDL_QueryTexture(texture, nullptr, nullptr, &w, &h) != 0 || w <= 0 || h <= 0)
        return;

    int dw = boxW;
    int dh = boxH;
    if (w * boxH >= h * boxW)
        dh = boxW * h / w;
    else
        dw = boxH * w / h;

    SDL_Rect dst{sx(rs, x + (boxW - dw) / 2), sy(rs, y + (boxH - dh) / 2), sx(rs, dw), sy(rs, dh)};
    SDL_RenderCopy(renderer, texture, nullptr, &dst);
}

int dexNumber(PkmnDef::PkmnDefPtr const& def)
{
    if (!def)
        return 0;
    auto const& list = Game::instance()->data.pkmns;
    for (size_t i = 0; i < list.size(); ++i)
    {
        if (list[i] && list[i]->getId() == def->getId())
            return static_cast<int>(i) + 1;
    }
    return 0;
}

int typeIconIndex(std::string const& typeId)
{
    auto const& types = Game::instance()->data.types;
    for (size_t i = 0; i < types.size(); ++i)
    {
        if (types[i] && types[i]->getId() == typeId)
            return static_cast<int>(i);
    }
    return -1;
}

std::string natureName(Pkmn::Nature nature)
{
    switch (nature)
    {
    case Pkmn::Nature::HARDY:
        return lc::translate("Hardy");
    case Pkmn::Nature::LONELY:
        return lc::translate("Lonely");
    case Pkmn::Nature::BRAVE:
        return lc::translate("Brave");
    case Pkmn::Nature::ADAMANT:
        return lc::translate("Adamant");
    case Pkmn::Nature::NAUGHTY:
        return lc::translate("Naughty");
    case Pkmn::Nature::BOLD:
        return lc::translate("Bold");
    case Pkmn::Nature::DOCILE:
        return lc::translate("Docile");
    case Pkmn::Nature::RELAXED:
        return lc::translate("Relaxed");
    case Pkmn::Nature::IMPISH:
        return lc::translate("Impish");
    case Pkmn::Nature::LAX:
        return lc::translate("Lax");
    case Pkmn::Nature::TIMID:
        return lc::translate("Timid");
    case Pkmn::Nature::HASTY:
        return lc::translate("Hasty");
    case Pkmn::Nature::SERIOUS:
        return lc::translate("Serious");
    case Pkmn::Nature::JOLLY:
        return lc::translate("Jolly");
    case Pkmn::Nature::NAIVE:
        return lc::translate("Naive");
    case Pkmn::Nature::MODEST:
        return lc::translate("Modest");
    case Pkmn::Nature::MILD:
        return lc::translate("Mild");
    case Pkmn::Nature::QUIET:
        return lc::translate("Quiet");
    case Pkmn::Nature::BASHFUL:
        return lc::translate("Bashful");
    case Pkmn::Nature::RASH:
        return lc::translate("Rash");
    case Pkmn::Nature::CALM:
        return lc::translate("Calm");
    case Pkmn::Nature::GENTLE:
        return lc::translate("Gentle");
    case Pkmn::Nature::SASSY:
        return lc::translate("Sassy");
    case Pkmn::Nature::CAREFUL:
        return lc::translate("Careful");
    case Pkmn::Nature::QUIRKY:
        return lc::translate("Quirky");
    case Pkmn::Nature::__SIZE_NATURE:
        break;
    }
    return "";
}

std::string natureFlavor(Pkmn::Nature nature)
{
    switch (nature)
    {
    case Pkmn::Nature::LONELY:
    case Pkmn::Nature::BRAVE:
    case Pkmn::Nature::ADAMANT:
    case Pkmn::Nature::NAUGHTY:
        return lc::translate("It likes to fight.");
    case Pkmn::Nature::BOLD:
    case Pkmn::Nature::RELAXED:
    case Pkmn::Nature::IMPISH:
    case Pkmn::Nature::LAX:
        return lc::translate("It is sturdy.");
    case Pkmn::Nature::MODEST:
    case Pkmn::Nature::MILD:
    case Pkmn::Nature::QUIET:
    case Pkmn::Nature::RASH:
        return lc::translate("It is proud of its power.");
    case Pkmn::Nature::CALM:
    case Pkmn::Nature::GENTLE:
    case Pkmn::Nature::SASSY:
    case Pkmn::Nature::CAREFUL:
        return lc::translate("It is strongly defensive.");
    case Pkmn::Nature::TIMID:
    case Pkmn::Nature::HASTY:
    case Pkmn::Nature::JOLLY:
    case Pkmn::Nature::NAIVE:
        return lc::translate("It is quick to flee.");
    default:
        return lc::translate("It is quite average.");
    }
}

} // namespace

PkmnInfoScene::PkmnInfoScene(SDL_Renderer* renderer, size_t partyIndex) : Scene(renderer), partyIndex(partyIndex)
{
    char const* pages[] = {
        "resources/Graphics/UI/Summary/bg_1.png",
        "resources/Graphics/UI/Summary/bg_2.png",
        "resources/Graphics/UI/Summary/bg_3.png",
        "resources/Graphics/UI/Summary/bg_4.png",
        "resources/Graphics/UI/Summary/bg_5.png",
    };
    for (size_t i = 0; i < pageTextures.size(); ++i)
    {
        SDL_Surface* surface = IMG_Load(pages[i]);
        pageTextures[i]      = SDL_CreateTextureFromSurface(renderer, surface);
        SDL_FreeSurface(surface);
    }

    SDL_Surface* moveDetailSurface = IMG_Load("resources/Graphics/UI/Summary/bg_movedetail.png");
    moveDetailTexture              = SDL_CreateTextureFromSurface(renderer, moveDetailSurface);
    SDL_FreeSurface(moveDetailSurface);

    SDL_Surface* moveCursorSurface = IMG_Load("resources/Graphics/UI/Summary/cursor_move.png");
    moveCursorTexture              = SDL_CreateTextureFromSurface(renderer, moveCursorSurface);
    SDL_FreeSurface(moveCursorSurface);
    if (moveCursorTexture)
        SDL_SetTextureBlendMode(moveCursorTexture, SDL_BLENDMODE_BLEND);

    SDL_Surface* categorySurface = IMG_Load("resources/Graphics/UI/category.png");
    categoryTexture              = SDL_CreateTextureFromSurface(renderer, categorySurface);
    SDL_FreeSurface(categorySurface);
    if (categoryTexture)
        SDL_SetTextureBlendMode(categoryTexture, SDL_BLENDMODE_BLEND);

    SDL_Surface* typesSurface = IMG_Load("resources/Graphics/UI/types.png");
    typesTexture              = SDL_CreateTextureFromSurface(renderer, typesSurface);
    SDL_FreeSurface(typesSurface);
    if (typesTexture)
        SDL_SetTextureBlendMode(typesTexture, SDL_BLENDMODE_BLEND);

    SDL_Surface* hpSurface = IMG_Load("resources/Graphics/UI/Summary/overlay_hp.png");
    hpTexture              = SDL_CreateTextureFromSurface(renderer, hpSurface);
    SDL_FreeSurface(hpSurface);

    SDL_Surface* expSurface = IMG_Load("resources/Graphics/UI/Summary/overlay_exp.png");
    expTexture              = SDL_CreateTextureFromSurface(renderer, expSurface);
    SDL_FreeSurface(expSurface);

    SDL_Surface* shinySurface = IMG_Load("resources/Graphics/UI/shiny.png");
    shinyTexture              = SDL_CreateTextureFromSurface(renderer, shinySurface);
    SDL_FreeSurface(shinySurface);
    if (shinyTexture)
        SDL_SetTextureBlendMode(shinyTexture, SDL_BLENDMODE_BLEND);

    pageSound = loadChunk("resources/Audio/SE/GUI summary change page.ogg.wav");

    if (!currentPkmn() || !currentPkmn()->getDefinition())
    {
        leave = true;
        return;
    }
    reloadSprite();
    reloadIcon();
    reloadItem();
    reloadBall();
    playCry(currentPkmn()->getDefinition()->getId());
}

PkmnInfoScene::~PkmnInfoScene()
{
    for (SDL_Texture* texture : pageTextures)
        SDL_DestroyTexture(texture);
    SDL_DestroyTexture(moveDetailTexture);
    SDL_DestroyTexture(moveCursorTexture);
    SDL_DestroyTexture(categoryTexture);
    SDL_DestroyTexture(typesTexture);
    SDL_DestroyTexture(hpTexture);
    SDL_DestroyTexture(expTexture);
    SDL_DestroyTexture(ballTexture);
    SDL_DestroyTexture(shinyTexture);
    SDL_DestroyTexture(itemTexture);
    SDL_DestroyTexture(iconTexture);
    SDL_DestroyTexture(sprite);
    if (pageSound)
    {
        Mix_HaltChannel(SoundChannel);
        Mix_FreeChunk(pageSound);
    }
}

Pkmn::PkmnPtr PkmnInfoScene::currentPkmn() const
{
    auto& party = Game::instance()->data.player.pkmns;
    if (partyIndex >= party.size())
        return nullptr;
    return party[partyIndex];
}

void PkmnInfoScene::reloadSprite()
{
    if (sprite)
    {
        SDL_DestroyTexture(sprite);
        sprite = nullptr;
    }
    auto pkmn = currentPkmn();
    if (!pkmn || !pkmn->getDefinition())
        return;

    std::string folder = "resources/Graphics/Pokemon/Front";
    if (pkmn->isShiny())
        folder += " shiny";
    folder += "/";

    std::vector<std::string> candidates;
    if (pkmn->isFemale())
        candidates.push_back(folder + pkmn->getDefinition()->getId() + "_female.png");
    candidates.push_back(folder + pkmn->getDefinition()->getId() + ".png");

    SDL_Surface* surface = nullptr;
    for (auto const& path : candidates)
    {
        if (!std::filesystem::exists(path))
            continue;
        surface = IMG_Load(path.c_str());
        if (surface)
            break;
    }
    if (!surface)
        return;

    sprite = SDL_CreateTextureFromSurface(renderer, surface);
    SDL_FreeSurface(surface);
    if (sprite)
        SDL_SetTextureBlendMode(sprite, SDL_BLENDMODE_BLEND);
}

void PkmnInfoScene::reloadIcon()
{
    if (iconTexture)
    {
        SDL_DestroyTexture(iconTexture);
        iconTexture = nullptr;
    }

    auto pkmn = currentPkmn();
    if (!pkmn || !pkmn->getDefinition())
        return;

    std::string base = "resources/Graphics/Pokemon/Icons/" + pkmn->getDefinition()->getId();
    std::string path = base + ".png";
    if (pkmn->isFemale())
    {
        std::string femalePath = base + "_female.png";
        if (std::filesystem::exists(femalePath))
            path = femalePath;
    }
    if (!std::filesystem::exists(path))
        return;

    SDL_Surface* surface = IMG_Load(path.c_str());
    if (!surface)
        return;
    iconTexture = SDL_CreateTextureFromSurface(renderer, surface);
    SDL_FreeSurface(surface);
    if (iconTexture)
        SDL_SetTextureBlendMode(iconTexture, SDL_BLENDMODE_BLEND);
}

void PkmnInfoScene::reloadBall()
{
    if (ballTexture)
    {
        SDL_DestroyTexture(ballTexture);
        ballTexture = nullptr;
    }

    auto        pkmn   = currentPkmn();
    std::string ballId = pkmn ? pkmn->getCaptureBall() : "POKEBALL";
    if (ballId.empty())
        ballId = "POKEBALL";

    std::string path = "resources/Graphics/UI/Summary/icon_ball_" + ballId + ".png";
    if (!std::filesystem::exists(path))
        path = "resources/Graphics/UI/Summary/icon_ball_POKEBALL.png";
    if (!std::filesystem::exists(path))
        return;

    SDL_Surface* surface = IMG_Load(path.c_str());
    if (!surface)
        return;
    ballTexture = SDL_CreateTextureFromSurface(renderer, surface);
    SDL_FreeSurface(surface);
    if (ballTexture)
        SDL_SetTextureBlendMode(ballTexture, SDL_BLENDMODE_BLEND);
}

void PkmnInfoScene::reloadItem()
{
    if (itemTexture)
    {
        SDL_DestroyTexture(itemTexture);
        itemTexture = nullptr;
    }

    auto pkmn = currentPkmn();
    if (!pkmn || !pkmn->getHeldItem() || !pkmn->getHeldItem()->getDefinition())
        return;

    std::string path = "resources/Graphics/Items/" + pkmn->getHeldItem()->getDefinition()->getId() + ".png";
    if (!std::filesystem::exists(path))
        return;

    SDL_Surface* surface = IMG_Load(path.c_str());
    if (!surface)
        return;
    itemTexture = SDL_CreateTextureFromSurface(renderer, surface);
    SDL_FreeSurface(surface);
    if (itemTexture)
        SDL_SetTextureBlendMode(itemTexture, SDL_BLENDMODE_BLEND);
}

void PkmnInfoScene::shiftParty(int delta)
{
    auto& party = Game::instance()->data.player.pkmns;
    if (Game::instance()->data.player.getPkmnCount() == 0)
        return;

    size_t next = partyIndex;
    for (size_t step = 0; step < party.size(); ++step)
    {
        next = static_cast<size_t>((static_cast<int>(next) + delta + static_cast<int>(party.size())) % party.size());
        if (party[next])
            break;
    }
    if (next == partyIndex || !party[next])
        return;

    partyIndex    = next;
    moveIndex     = 0;
    choosingMove  = false;
    switchingMove = false;
    reloadSprite();
    reloadIcon();
    reloadItem();
    reloadBall();
    if (currentPkmn() && currentPkmn()->getDefinition())
        playCry(currentPkmn()->getDefinition()->getId());
}

void PkmnInfoScene::update(Inputs const* inputs)
{
    if (leave)
        return;

    inputs->setHandler(inputId());

    if (inputs->B)
    {
        if (switchingMove)
        {
            switchingMove = false;
            moveIndex     = switchIndex;
            return;
        }
        if (choosingMove)
        {
            choosingMove = false;
            return;
        }
        leave = true;
        return;
    }

    if (inputs->left || inputs->right)
    {
        if (choosingMove)
            return;
        int dir   = inputs->right ? 1 : -1;
        int index = static_cast<int>(page) + dir;
        int count = static_cast<int>(Page::Count);
        page      = static_cast<Page>((index + count) % count);
        playSound(pageSound);
        return;
    }

    if (page == Page::Moves && choosingMove && (inputs->up || inputs->down))
    {
        int dir   = inputs->down ? 1 : -1;
        moveIndex = static_cast<size_t>((static_cast<int>(moveIndex) + dir + 4) % 4);
        return;
    }

    if (page == Page::Moves && choosingMove && inputs->A)
    {
        if (!switchingMove)
        {
            switchingMove = true;
            switchIndex   = moveIndex;
        }
        else
        {
            if (switchIndex != moveIndex && currentPkmn())
                std::swap(currentPkmn()->getMoves()[switchIndex], currentPkmn()->getMoves()[moveIndex]);
            switchingMove = false;
        }
        return;
    }

    if (page == Page::Moves && inputs->A)
    {
        choosingMove = true;
        return;
    }

    if (inputs->up)
        shiftParty(-1);
    else if (inputs->down)
        shiftParty(1);
}

void PkmnInfoScene::drawPage(RenderSizes rs)
{
    auto pkmn = currentPkmn();
    if (!pkmn || !pkmn->getDefinition())
        return;

    auto labelMove = [&](int bandTop, int valueRight, std::string const& label, std::string const& value) {
        int y = sy(rs, bandTop);
        RenderUtils::drawGreyText(renderer, rs, label, Font, sx(rs, 256), y);
        RenderUtils::drawGreyTextRightAligned(renderer, rs, value, Font, sx(rs, valueRight), y);
    };

    auto labelInfo = [&](int bandTop, int valueRight, std::string const& label, std::string const& value) {
        int y = sy(rs, bandTop);
        RenderUtils::drawGreyText(renderer, rs, label, Font, sx(rs, 236), y);
        RenderUtils::drawGreyTextRightAligned(renderer, rs, value, Font, sx(rs, valueRight), y);
    };

    auto labelName = [&](int bandTop, int valueRight, std::string const& label, Player const& player) {
        SDL_Color color   = PkmnUtils::genderColor(player.getGender());
        SDL_Color bgColor = PkmnUtils::genderBackgroundColor(player.getGender());
        int       y       = sy(rs, bandTop);
        RenderUtils::drawGreyText(renderer, rs, label, Font, sx(rs, 236), y);
        RenderUtils::drawTextRightAligned(renderer, rs, player.name, color, bgColor, Font, sx(rs, valueRight), y);
    };

    switch (page)
    {
    case Page::Info: {
        int dex = dexNumber(pkmn->getDefinition());
        labelInfo(80, InfoValueRight, lc::translate("DEX NO."), (boost::format("%03d") % dex).str());
        labelInfo(112, InfoValueRight, lc::translate("NAME"), pkmn->getDefinition()->getName());
        RenderUtils::drawGreyText(renderer, rs, lc::translate("TYPE"), Font, sx(rs, 236), sy(rs, 144));
        if (typesTexture)
        {
            int              iconH = 22;
            int              iconW = iconH * TypeIconW / TypeIconH;
            auto const&      types = pkmn->getDefinition()->getTypes();
            std::vector<int> icons;
            for (size_t i = 0; i < types.size() && icons.size() < 2; ++i)
            {
                if (i == 1 && types[0] == types[1])
                    break;
                int index = typeIconIndex(types[i]);
                if (index >= 0)
                    icons.push_back(index);
            }
            int gap   = 4;
            int iconX = InfoValueRight - static_cast<int>(icons.size()) * iconW;
            if (icons.size() > 1)
                iconX -= gap * static_cast<int>(icons.size() - 1);
            for (int index : icons)
            {
                SDL_Rect src{0, index * TypeIconH, TypeIconW, TypeIconH};
                SDL_Rect dst{sx(rs, iconX), sy(rs, 144 + (30 - iconH) / 2), sx(rs, iconW), sy(rs, iconH)};
                SDL_RenderCopy(renderer, typesTexture, &src, &dst);
                iconX += iconW + gap;
            }
        }

        uint64_t id = pkmn->getFirstTrainerId();
        if (id == 0)
            id = Game::instance()->data.player.id;
        labelName(176, InfoValueRight, lc::translate("OT"), Game::instance()->data.player);
        labelInfo(208, InfoValueRight, lc::translate("ID NO."), (boost::format("%05d") % (id % 100000)).str());

        auto labelAbove = [&](int valueY, std::string const& label, std::string const& value) {
            RenderUtils::drawGreyText(renderer, rs, label, Font, sx(rs, 236), sy(rs, valueY - 32));
            RenderUtils::drawGreyTextRightAligned(renderer, rs, value, Font, sx(rs, InfoValueRight), sy(rs, valueY));
        };

        std::string accumulatedExp = std::to_string(
            pkmn->accumulatedExpForLevel(pkmn->getLevel(), pkmn->getDefinition()->getGrowthRate()) + pkmn->getExp());
        labelAbove(272, lc::translate("Exp. Points"), accumulatedExp);

        std::string toNext = "---";
        if (pkmn->getLevel() < 100)
        {
            size_t span = pkmn->expToNextLevel();
            size_t have = pkmn->getExp();
            toNext      = std::to_string(have < span ? span - have : 0);
        }
        labelAbove(336, lc::translate("To Next Lv."), toNext);

        if (expTexture)
        {
            float pct = pkmn->getLevel() >= 100 ? 1.f : pkmn->getPercentageExp();
            if (pct < 0.f)
                pct = 0.f;
            if (pct > 1.f)
                pct = 1.f;
            int width = static_cast<int>(128.f * pct + 0.5f);
            if (width > 128)
                width = 128;
            if (width > 0)
            {
                // Trough on bg_1 is x=362..489, y=372..377, matching overlay_exp (128x6).
                SDL_Rect src{0, 0, width, 6};
                SDL_Rect dst{sx(rs, 362), sy(rs, 372), sx(rs, width), sy(rs, 6)};
                SDL_RenderCopy(renderer, expTexture, &src, &dst);
            }
        }
        break;
    }
    case Page::Memo: {
        int  y    = 78;
        auto line = [&](std::string const& text) {
            SDL_Color color{80, 80, 80, 255};
            SDL_Color shadow{180, 180, 180, 255};
            RenderUtils::drawTextWrapped(renderer, rs, text, color, shadow, Font, sx(rs, 236), sy(rs, y), 256);
            y += 32;
        };
        line((boost::format(lc::translate("%1% nature.")) % natureName(pkmn->getNature())).str());
        line(natureFlavor(pkmn->getNature()));
        if (pkmn->getHappiness() >= Pkmn::HighFriendship)
            line(lc::translate("It is very friendly."));
        else if (pkmn->getHappiness() >= 100)
            line(lc::translate("It looks happy."));
        else
            line(lc::translate("It seems unsure."));
        break;
    }
    case Page::Skills: {
        auto stats = pkmn->getStats();

        struct StatRow
        {
            int           y;
            char const*   label;
            PkmnDef::Stat stat;
            bool          hp;
        };

        StatRow rows[] = {
            {76,  "HP",      PkmnDef::HP,              true },
            {120, "ATTACK",  PkmnDef::ATTACK,          false},
            {152, "DEFENSE", PkmnDef::DEFENSE,         false},
            {184, "SP. ATK", PkmnDef::SPECIAL_ATTACK,  false},
            {216, "SP. DEF", PkmnDef::SPECIAL_DEFENSE, false},
            {248, "SPEED",   PkmnDef::SPEED,           false},
        };
        if (hpTexture)
        {
            float pct   = pkmn->getPercentageHP();
            int   frame = pct > 0.5625f ? 0 : (pct > 0.207f ? 1 : 2);
            // "255/255" is 57px at this font size and is right-aligned at SkillsValueRight, so the bar stops before it.
            int width = static_cast<int>(48 * pct + 0.5f);
            if (width < 1 && pkmn->getHP() > 0)
                width = 1;
            SDL_Rect src{0, frame * 6, 96, 6};
            SDL_Rect dst{sx(rs, 360), sy(rs, 110), sx(rs, width), sy(rs, 6)};
            SDL_RenderCopy(renderer, hpTexture, &src, &dst);
        }

        for (auto const& row : rows)
        {
            std::string value = std::to_string(stats[row.stat]);
            if (row.hp)
                value = (boost::format("%1%/%2%") % pkmn->getHP() % pkmn->getTotalHP()).str();
            labelMove(row.y, SkillsValueRight, lc::translate(row.label), value);
        }

        std::string ability = pkmn->effectiveAbilityId();
        if (ability.empty())
            ability = "---";
        else
        {
            for (auto const& known : Game::instance()->data.abilities)
            {
                if (known && known->getId() == ability)
                {
                    ability = known->getName();
                    break;
                }
            }
        }
        labelMove(284, 496, lc::translate("ABILITY"), ability);
        break;
    }
    case Page::Moves: {
        for (int i = 0; i < 4; ++i)
        {
            int         nameY = MoveTop[i];
            int         ppY   = MoveTop[i] + 32;
            auto        move  = pkmn->getMoves().at(i);
            std::string name  = "-";
            std::string pp    = "--";
            if (move && move->getDefinition())
            {
                name = move->getDefinition()->getName();
                pp   = (boost::format("%1%/%2%") % move->getCurrentPP() % move->maxPP()).str();
                if (typesTexture)
                {
                    int index = typeIconIndex(move->getDefinition()->getType());
                    if (index >= 0)
                    {
                        int      iconH = 22;
                        int      iconW = iconH * TypeIconW / TypeIconH;
                        SDL_Rect src{0, index * TypeIconH, TypeIconW, TypeIconH};
                        SDL_Rect dst{sx(rs, 252), sy(rs, MoveTop[i] + (36 - iconH) / 2), sx(rs, iconW), sy(rs, iconH)};
                        SDL_RenderCopy(renderer, typesTexture, &src, &dst);
                    }
                }
            }
            RenderUtils::drawGreyText(renderer, rs, name, Font, sx(rs, 310), sy(rs, nameY));
            auto ppLine = boost::format(lc::translate("PP %1%")) % pp;
            RenderUtils::drawGreyText(renderer, rs, ppLine.str(), Font, sx(rs, 316), sy(rs, ppY));
        }

        if (choosingMove && moveCursorTexture && moveIndex < 4)
        {
            auto blitCursor = [&](int frame, size_t index) {
                SDL_Rect src{0, frame * 74, 252, 74};
                SDL_Rect dst{sx(rs, 242), sy(rs, MoveTop[index] - 5), sx(rs, 252), sy(rs, 74)};
                SDL_RenderCopy(renderer, moveCursorTexture, &src, &dst);
            };
            if (!switchingMove)
                blitCursor(0, moveIndex);
            else
            {
                blitCursor(1, switchIndex);
                if (switchIndex != moveIndex)
                    blitCursor(0, moveIndex);
            }
        }

        if (choosingMove)
        {
            if (iconTexture)
            {
                int iconSheetW = 0;
                int iconSheetH = 0;
                SDL_QueryTexture(iconTexture, nullptr, nullptr, &iconSheetW, &iconSheetH);
                SDL_Rect src{0, 0, iconSheetW / 2, iconSheetH};
                SDL_Rect dst{sx(rs, 8), sy(rs, 62), sx(rs, 56), sy(rs, 56)};
                SDL_RenderCopy(renderer, iconTexture, &src, &dst);
            }
            if (typesTexture)
            {
                int         iconH = 22;
                int         iconW = iconH * TypeIconW / TypeIconH;
                int         iconX = 70;
                int         iconY = 62 + (56 - iconH) / 2;
                auto const& types = pkmn->getDefinition()->getTypes();
                for (size_t i = 0; i < types.size() && i < 2; ++i)
                {
                    if (i == 1 && types[0] == types[1])
                        break;
                    int index = typeIconIndex(types[i]);
                    if (index < 0)
                        continue;
                    SDL_Rect src{0, index * TypeIconH, TypeIconW, TypeIconH};
                    SDL_Rect dst{sx(rs, iconX), sy(rs, iconY), sx(rs, iconW), sy(rs, iconH)};
                    SDL_RenderCopy(renderer, typesTexture, &src, &dst);
                    iconX += iconW + 4;
                }
            }

            MoveDef::MoveDefPtr shown;
            if (moveIndex < pkmn->getMoves().size() && pkmn->getMoves().at(moveIndex))
                shown = pkmn->getMoves().at(moveIndex)->getDefinition();
            std::string power    = "---";
            std::string accuracy = "---";
            std::string description;
            if (shown)
            {
                if (shown->getPower() > 1)
                    power = std::to_string(shown->getPower());
                if (shown->getAccuracy() > 0)
                    accuracy = std::to_string(shown->getAccuracy());
                description = shown->getDescription();
            }
            auto detailRow = [&](int y, std::string const& label, std::string const& value) {
                RenderUtils::drawGreyText(renderer, rs, label, Font, sx(rs, 12), sy(rs, y));
                RenderUtils::drawGreyTextRightAligned(renderer, rs, value, Font, sx(rs, 228), sy(rs, y));
            };
            RenderUtils::drawGreyText(renderer, rs, lc::translate("CATEGORY"), Font, sx(rs, 12), sy(rs, 122));
            if (categoryTexture && shown && shown->getCategory() < MoveDef::__SIZE)
            {
                int      frame = static_cast<int>(shown->getCategory());
                SDL_Rect src{0, frame * 28, 64, 28};
                SDL_Rect dst{sx(rs, 167), sy(rs, 124), sx(rs, 64), sy(rs, 28)};
                SDL_RenderCopy(renderer, categoryTexture, &src, &dst);
            }
            detailRow(154, lc::translate("POWER"), power);
            detailRow(186, lc::translate("ACCURACY"), accuracy);
            if (!description.empty())
            {
                SDL_Rect clip{sx(rs, 8), sy(rs, 220), sx(rs, 216), sy(rs, 152)};
                SDL_RenderSetClipRect(renderer, &clip);
                SDL_Color color{80, 80, 80, 255};
                SDL_Color shadow{180, 180, 180, 255};
                RenderUtils::drawTextWrapped(renderer,
                                             rs,
                                             description,
                                             color,
                                             shadow,
                                             Font,
                                             sx(rs, 16),
                                             sy(rs, 224),
                                             200);
                SDL_RenderSetClipRect(renderer, nullptr);
            }
        }
        break;
    }
    case Page::Ribbons:
        // TODO
        break;
    case Page::Count:
        break;
    }
}

void PkmnInfoScene::draw(Fps const* /*fps*/, RenderSizes rs)
{
    bool const moveDetail = page == Page::Moves && choosingMove && moveDetailTexture;
    if (moveDetail)
    {
        SDL_Rect dst{0, 0, rs.ww, rs.wh};
        SDL_RenderCopy(renderer, moveDetailTexture, nullptr, &dst);
    }
    else
    {
        size_t pageIndex = static_cast<size_t>(page);
        if (pageIndex < pageTextures.size() && pageTextures[pageIndex])
        {
            SDL_Rect dst{0, 0, rs.ww, rs.wh};
            SDL_RenderCopy(renderer, pageTextures[pageIndex], nullptr, &dst);
        }
        else
        {
            SDL_SetRenderDrawColor(renderer, 48, 112, 184, 255);
            SDL_RenderClear(renderer);
        }
    }

    auto pkmn = currentPkmn();
    if (pkmn && !moveDetail)
    {
        auto level = boost::format("%1%") % pkmn->getLevel();
        RenderUtils::drawGreyText(renderer, rs, level.str(), Font, sx(rs, 50), sy(rs, 91));

        if (shinyTexture && pkmn->isShiny())
        {
            SDL_Rect star{sx(rs, 178), sy(rs, 98), sx(rs, 14), sy(rs, 16)};
            SDL_RenderCopy(renderer, shinyTexture, nullptr, &star);
        }

        int const ballX    = 2;
        int const ballY    = 60;
        int const ballSize = 32;
        if (ballTexture && !choosingMove)
        {
            SDL_Rect ball{sx(rs, ballX), sy(rs, ballY), sx(rs, ballSize), sy(rs, ballSize)};
            SDL_RenderCopy(renderer, ballTexture, nullptr, &ball);
        }

        int nameX = sx(rs, ballX + ballSize + 4);
        int nameY = sy(rs, ballY - 4 + (ballSize - Font) / 2);
        int nameW = RenderUtils::drawGreyText(renderer, rs, pkmn->getDisplayName(), Font, nameX, nameY);
        if (pkmn->getGender() != Pkmn::Gender::GENDERLESS)
        {
            RenderUtils::drawUnicodeSymbol(renderer,
                                           rs,
                                           PkmnUtils::genderSymbol(pkmn->getGender()),
                                           PkmnUtils::genderColor(pkmn->getGender()),
                                           {180, 180, 180, 255},
                                           Font,
                                           nameX + nameW * rs.ww / rs.aw + sx(rs, 4),
                                           nameY);
        }

        if (sprite)
            blitContained(renderer, sprite, rs, 36, 150, 136, 136);

        std::string itemName = lc::translate("None");
        if (pkmn->getHeldItem() && pkmn->getHeldItem()->getDefinition())
            itemName = pkmn->getHeldItem()->getDefinition()->getName();
        RenderUtils::drawGreyText(renderer, rs, itemName, Font, sx(rs, 12), sy(rs, 352));

        if (itemTexture)
        {
            SDL_Rect item{sx(rs, 14), sy(rs, 304), sx(rs, 32), sy(rs, 32)};
            SDL_RenderCopy(renderer, itemTexture, nullptr, &item);
        }
    }

    drawPage(rs);
}

bool PkmnInfoScene::popScene() const
{
    return leave;
}

std::string PkmnInfoScene::name()
{
    return "PkmnInfoScene";
}
