#include "battleanim.h"

#include <boost/json.hpp>

#include <SDL_image.h>

#include <algorithm>
#include <cmath>
#include <cstdlib>
#include <fstream>
#include <sstream>
#include <unordered_map>
#include <vector>

namespace
{
constexpr int PlayerX    = 72;
constexpr int PlayerY    = 80;
constexpr int PlayerYPic = 88;
constexpr int FoeX       = 176;
constexpr int FoeY       = 40;
constexpr int FoeYPic    = 48;
constexpr int AttackerId = 100;
constexpr int TargetId   = 101;

int sin256(int index, int amplitude)
{
    double turns = (index & 255) * 6.283185307179586 / 256.0;
    return static_cast<int>(std::lround(std::sin(turns) * amplitude));
}

int jsonInt(boost::json::value const& value, int fallback = 0)
{
    if (value.is_int64())
        return static_cast<int>(value.as_int64());
    if (value.is_uint64())
        return static_cast<int>(value.as_uint64());
    if (value.is_double())
        return static_cast<int>(value.as_double());
    return fallback;
}

std::string jsonString(boost::json::value const& value)
{
    if (!value.is_string())
        return {};
    return std::string(value.as_string());
}

struct Step
{
    std::string op;
    int         tile  = 0;
    int         dur   = 0;
    bool        hFlip = false;
    bool        vFlip = false;
    int         x     = 0;
    int         y     = 0;
    int         rot   = 0;
    int         count = 0;
    int         to    = 0;
};

struct Template
{
    std::string                  tileTag;
    int                          width  = 0;
    int                          height = 0;
    bool                         affine = false;
    bool                         blend  = false;
    std::string                  callback;
    std::vector<std::vector<Step>> anims;
    std::vector<std::vector<Step>> affineAnims;
};

struct Command
{
    int              op = 0;
    std::string      text;
    std::string      text2;
    int              a      = 0;
    int              b      = 0;
    int              c      = 0;
    bool             target = false;
    std::vector<int> args;
};

struct Profile
{
    std::string kind;
    std::string anchor;
    std::string life;
};

struct Gfx
{
    SDL_Surface* surface = nullptr;
    SDL_Texture* texture = nullptr;
    int          width   = 0;
    int          height  = 0;
};

int bgId(std::string const& name)
{
    static std::unordered_map<std::string, int> const ids = {
        {"BG_NONE", 0},
        {"BG_DARK", 1},
        {"BG_GHOST", 2},
        {"BG_PSYCHIC", 3},
        {"BG_IMPACT_OPPONENT", 4},
        {"BG_IMPACT_PLAYER", 5},
        {"BG_DRILL", 7},
        {"BG_HIGHSPEED_OPPONENT", 9},
        {"BG_HIGHSPEED_PLAYER", 10},
        {"BG_THUNDER", 11},
        {"BG_GUILLOTINE_OPPONENT", 12},
        {"BG_GUILLOTINE_PLAYER", 13},
        {"BG_ICE", 15},
        {"BG_COSMIC", 16},
        {"BG_IN_AIR", 17},
        {"BG_SKY", 18},
        {"BG_AURORA", 20},
        {"BG_FISSURE", 21},
        {"BG_BUG_OPPONENT", 22},
        {"BG_BUG_PLAYER", 23},
        {"BG_SOLAR_BEAM_OPPONENT", 24},
        {"BG_SOLAR_BEAM_PLAYER", 25},
    };
    auto found = ids.find(name);
    return found == ids.end() ? -1 : found->second;
}
} // namespace

struct BattleAnimPlayer::Impl
{
    struct Sprite
    {
        bool        alive    = false;
        bool        visible  = true;
        int         x        = 0;
        int         y        = 0;
        int         x2       = 0;
        int         y2       = 0;
        int         data[16] = {};
        int         priority = 0;
        std::string tag;
        int         width      = 0;
        int         height     = 0;
        bool        blend      = false;
        bool        affine     = false;
        int         xScale     = 256;
        int         yScale     = 256;
        int         rotation   = 0;
        bool        affineEnded = true;
        int         affineAnim  = 0;
        int         affineCmd   = 0;
        int         affineDelay = 0;
        bool        animEnded   = false;
        int         animNum     = 0;
        int         animCmd     = 0;
        int         animDelay   = 0;
        int         tile        = 0;
        bool        hFlip       = false;
        bool        vFlip       = false;
        int         life        = 0;
        int         goalX       = 0;
        int         goalY       = 0;
        std::vector<std::vector<Step>> anims;
        std::vector<std::vector<Step>> affineAnims;
        void (*callback)(Impl&, Sprite*) = nullptr;
        void (*stored)(Impl&, Sprite*)   = nullptr;
    };

    struct Task
    {
        bool alive      = false;
        int  data[16]   = {};
        int  priority   = 0;
        void (*callback)(Impl&, Task*) = nullptr;
    };

    struct Battler
    {
        int   x2      = 0;
        int   y2      = 0;
        int   scaleX  = 256;
        int   scaleY  = 256;
        int   tintR   = 255;
        int   tintG   = 255;
        int   tintB   = 255;
        int   tintA   = 0;
        int   alpha   = 255;
        bool  hidden  = false;
        bool  blendOn = false;
    };

    explicit Impl(SDL_Renderer* rendererIn)
        : renderer(rendererIn)
    {
        load();
    }

    ~Impl()
    {
        for (auto& entry : sheets)
        {
            if (entry.second.texture)
                SDL_DestroyTexture(entry.second.texture);
            if (entry.second.surface)
                SDL_FreeSurface(entry.second.surface);
        }
        for (auto& entry : frameCache)
        {
            if (entry.second)
                SDL_DestroyTexture(entry.second);
        }
        for (auto& entry : backgrounds)
        {
            if (entry.second)
                SDL_DestroyTexture(entry.second);
        }
    }

    SDL_Renderer*                                    renderer;
    std::unordered_map<std::string, std::string>     moves;
    std::unordered_map<std::string, std::vector<Command>> scripts;
    std::unordered_map<std::string, Template>        templates;
    std::unordered_map<std::string, std::string>     gfxPath;
    std::unordered_map<int, std::string>             backgroundPath;
    std::unordered_map<std::string, Profile>         profiles;
    mutable std::unordered_map<std::string, Gfx>          sheets;
    mutable std::unordered_map<std::string, SDL_Texture*> frameCache;
    mutable std::unordered_map<int, SDL_Texture*>         backgrounds;

    bool                active            = false;
    bool                attackerIsPlayer  = true;
    bool                targetIsPlayer    = false;
    int                 turn              = 0;
    int                 power             = 0;
    int                 damage            = 0;
    int                 args[8]           = {};
    std::string         script;
    int                 index             = 0;
    std::vector<std::pair<std::string, int>> stack;
    int                 waitFrames        = 0;
    bool                waitVisual        = false;
    int                 elapsed           = 0;
    int                 background        = -1;
    int                 backgroundAlpha   = 0;
    bool                backgroundIn      = false;
    Battler             battlers[2];
    std::vector<Sprite> sprites;
    std::vector<Task>   tasks;

    bool sidePlayer(int battler) const
    {
        if (battler == 0 || battler == 2)
            return attackerIsPlayer;
        return targetIsPlayer;
    }

    int sideIndex(int battler) const
    {
        return sidePlayer(battler) ? 1 : 0;
    }

    int logicalX(bool player, bool picture) const
    {
        (void)picture;
        return player ? PlayerX : FoeX;
    }

    int logicalY(bool player, bool picture) const
    {
        if (player)
            return picture ? PlayerYPic : PlayerY;
        return picture ? FoeYPic : FoeY;
    }

    bool alive() const
    {
        for (auto const& sprite : sprites)
        {
            if (sprite.alive)
                return true;
        }
        for (auto const& task : tasks)
        {
            if (task.alive)
                return true;
        }
        return false;
    }

    void clearPoses()
    {
        battlers[0] = {};
        battlers[1] = {};
        battlers[0].tintR = battlers[1].tintR = 255;
        battlers[0].tintG = battlers[1].tintG = 255;
        battlers[0].tintB = battlers[1].tintB = 255;
        battlers[0].alpha = battlers[1].alpha = 255;
        battlers[0].scaleX = battlers[1].scaleX = 256;
        battlers[0].scaleY = battlers[1].scaleY = 256;
    }

    void finish()
    {
        active = false;
        waitFrames = 0;
        waitVisual = false;
        sprites.clear();
        tasks.clear();
        background      = -1;
        backgroundAlpha = 0;
        clearPoses();
    }

    void load()
    {
        auto read = [](char const* path) {
            std::ifstream in(path);
            std::stringstream buffer;
            buffer << in.rdbuf();
            if (!in)
                return boost::json::value();
            return boost::json::parse(buffer.str());
        };

        boost::json::value scriptsValue;
        boost::json::value templatesValue;
        boost::json::value callbacksValue;
        try
        {
            scriptsValue   = read("resources/BattleAnims/scripts.json");
            templatesValue = read("resources/BattleAnims/templates.json");
            callbacksValue = read("resources/BattleAnims/callbacks.json");
        }
        catch (...)
        {
            return;
        }
        if (!scriptsValue.is_object() || !templatesValue.is_object() || !callbacksValue.is_object())
            return;

        for (auto const& entry : scriptsValue.at("moves").as_object())
            moves.emplace(entry.key(), jsonString(entry.value()));
        for (auto const& entry : scriptsValue.at("scripts").as_object())
        {
            std::vector<Command> commands;
            for (auto const& item : entry.value().as_array())
            {
                auto const& object = item.as_object();
                Command     command;
                command.op     = jsonInt(object.at("op"));
                command.text   = object.contains("tag") ? jsonString(object.at("tag"))
                               : object.contains("template") ? jsonString(object.at("template"))
                               : object.contains("task") ? jsonString(object.at("task"))
                               : object.contains("script") ? jsonString(object.at("script"))
                               : object.contains("a") ? jsonString(object.at("a"))
                                                        : std::string();
                command.text2  = object.contains("b") ? jsonString(object.at("b")) : std::string();
                command.a      = object.contains("frames") ? jsonInt(object.at("frames"))
                               : object.contains("battler") ? jsonInt(object.at("battler"))
                               : object.contains("eva") ? jsonInt(object.at("eva"))
                               : object.contains("arg") ? jsonInt(object.at("arg"))
                               : object.contains("turn") ? jsonInt(object.at("turn"))
                               : object.contains("bg") ? jsonInt(object.at("bg"))
                               : object.contains("priority") ? jsonInt(object.at("priority"))
                               : object.contains("opponent") ? jsonInt(object.at("opponent"))
                                                             : 0;
                command.b      = object.contains("evb") ? jsonInt(object.at("evb"))
                               : object.contains("value") ? jsonInt(object.at("value"))
                               : object.contains("player") ? jsonInt(object.at("player"))
                                                           : 0;
                command.c      = object.contains("contest") ? jsonInt(object.at("contest")) : 0;
                command.target = object.contains("target") && object.at("target").is_bool() && object.at("target").as_bool();
                if (object.contains("args"))
                {
                    for (auto const& arg : object.at("args").as_array())
                        command.args.push_back(jsonInt(arg));
                }
                commands.push_back(std::move(command));
            }
            scripts.emplace(entry.key(), std::move(commands));
        }

        auto const& templateObject = templatesValue.at("templates").as_object();
        for (auto const& entry : templateObject)
        {
            auto const& object = entry.value().as_object();
            Template    templ;
            templ.tileTag  = jsonString(object.at("tileTag"));
            templ.width    = jsonInt(object.at("width"));
            templ.height   = jsonInt(object.at("height"));
            templ.affine   = object.at("affine").as_bool();
            templ.blend    = object.at("blend").as_bool();
            templ.callback = jsonString(object.at("callback"));
            auto readSteps = [](boost::json::array const& rows) {
                std::vector<std::vector<Step>> anims;
                for (auto const& rowValue : rows)
                {
                    std::vector<Step> row;
                    for (auto const& stepValue : rowValue.as_array())
                    {
                        auto const& stepObject = stepValue.as_object();
                        Step        step;
                        step.op    = jsonString(stepObject.at("op"));
                        step.tile  = stepObject.contains("tile") ? jsonInt(stepObject.at("tile")) : 0;
                        step.dur   = stepObject.contains("dur") ? jsonInt(stepObject.at("dur")) : 0;
                        step.hFlip = stepObject.contains("hFlip") && stepObject.at("hFlip").as_bool();
                        step.vFlip = stepObject.contains("vFlip") && stepObject.at("vFlip").as_bool();
                        step.x     = stepObject.contains("x") ? jsonInt(stepObject.at("x")) : 0;
                        step.y     = stepObject.contains("y") ? jsonInt(stepObject.at("y")) : 0;
                        step.rot   = stepObject.contains("rot") ? jsonInt(stepObject.at("rot")) : 0;
                        step.count = stepObject.contains("count") ? jsonInt(stepObject.at("count")) : 0;
                        step.to    = stepObject.contains("to") ? jsonInt(stepObject.at("to")) : 0;
                        row.push_back(step);
                    }
                    anims.push_back(std::move(row));
                }
                return anims;
            };
            templ.anims       = readSteps(object.at("anims").as_array());
            templ.affineAnims = readSteps(object.at("affineAnims").as_array());
            templates.emplace(entry.key(), std::move(templ));
        }
        for (auto const& entry : templatesValue.at("gfx").as_object())
            gfxPath.emplace(entry.key(), "resources/BattleAnims/" + jsonString(entry.value().as_object().at("file")));
        for (auto const& entry : templatesValue.at("backgrounds").as_object())
        {
            int id = bgId(std::string(entry.key()));
            if (id >= 0)
                backgroundPath.emplace(id, "resources/BattleAnims/" + jsonString(entry.value()));
        }
        for (auto const& entry : callbacksValue.as_object())
        {
            auto const& object = entry.value().as_object();
            Profile     profile;
            profile.kind   = jsonString(object.at("kind"));
            profile.anchor = object.contains("anchor") ? jsonString(object.at("anchor")) : std::string();
            profile.life   = object.contains("life") ? jsonString(object.at("life")) : std::string();
            profiles.emplace(entry.key(), profile);
        }
    }

    bool scriptReady(std::string const& name, int depth) const
    {
        if (depth > 8)
            return true;
        auto found = scripts.find(name);
        if (found == scripts.end())
            return false;
        for (auto const& command : found->second)
        {
            if (command.op == 0x02 && !templates.contains(command.text))
                return false;
            if ((command.op == 0x0E || command.op == 0x13) && !command.text.empty()
                && !scriptReady(command.text, depth + 1))
                return false;
            if (command.op == 0x11
                && ((!command.text.empty() && !scriptReady(command.text, depth + 1))
                    || (!command.text2.empty() && !scriptReady(command.text2, depth + 1))))
                return false;
            if ((command.op == 0x12 || command.op == 0x21) && !command.text.empty()
                && !scriptReady(command.text, depth + 1))
                return false;
        }
        return true;
    }

    Gfx* sheetFor(std::string const& tag) const
    {
        auto path = gfxPath.find(tag);
        if (path == gfxPath.end())
            return nullptr;
        auto& gfx = sheets[tag];
        if (!gfx.surface)
        {
            gfx.surface = IMG_Load(path->second.c_str());
            if (!gfx.surface)
                return nullptr;
            if (SDL_ISPIXELFORMAT_INDEXED(gfx.surface->format->format))
                SDL_SetColorKey(gfx.surface, SDL_TRUE, 0);
            gfx.texture = SDL_CreateTextureFromSurface(renderer, gfx.surface);
            if (gfx.texture)
                SDL_SetTextureBlendMode(gfx.texture, SDL_BLENDMODE_BLEND);
            gfx.width  = gfx.surface->w;
            gfx.height = gfx.surface->h;
        }
        return gfx.surface ? &gfx : nullptr;
    }

    SDL_Texture* backgroundTexture(int id) const
    {
        auto found = backgrounds.find(id);
        if (found != backgrounds.end())
            return found->second;
        auto path = backgroundPath.find(id);
        if (path == backgroundPath.end())
            return nullptr;
        SDL_Surface* surface = IMG_Load(path->second.c_str());
        if (!surface)
            return nullptr;
        SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, surface);
        SDL_FreeSurface(surface);
        if (texture)
            SDL_SetTextureBlendMode(texture, SDL_BLENDMODE_BLEND);
        backgrounds.emplace(id, texture);
        return texture;
    }

    SDL_Texture* frameFor(Sprite const& sprite) const
    {
        if (sprite.width <= 0 || sprite.height <= 0)
            return nullptr;
        auto* gfx = sheetFor(sprite.tag);
        if (!gfx || !gfx->surface)
            return nullptr;
        std::string key = sprite.tag + ":" + std::to_string(sprite.tile) + ":" + std::to_string(sprite.width) + "x"
                          + std::to_string(sprite.height);
        auto cached = frameCache.find(key);
        if (cached != frameCache.end())
            return cached->second;

        SDL_Surface* frame = SDL_CreateRGBSurfaceWithFormat(0, sprite.width, sprite.height, 32, SDL_PIXELFORMAT_RGBA8888);
        if (!frame)
            return nullptr;
        SDL_FillRect(frame, nullptr, SDL_MapRGBA(frame->format, 0, 0, 0, 0));
        int tilesPerRow = std::max(1, gfx->width / 8);
        int tilesW      = std::max(1, sprite.width / 8);
        int tilesH      = std::max(1, sprite.height / 8);
        for (int tileY = 0; tileY < tilesH; ++tileY)
        {
            for (int tileX = 0; tileX < tilesW; ++tileX)
            {
                int linear = sprite.tile + tileY * tilesW + tileX;
                SDL_Rect src;
                src.x = (linear % tilesPerRow) * 8;
                src.y = (linear / tilesPerRow) * 8;
                src.w = 8;
                src.h = 8;
                if (src.x + 8 > gfx->width || src.y + 8 > gfx->height)
                    continue;
                SDL_Rect dst{tileX * 8, tileY * 8, 8, 8};
                SDL_BlitSurface(gfx->surface, &src, frame, &dst);
            }
        }
        SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, frame);
        SDL_FreeSurface(frame);
        if (texture)
            SDL_SetTextureBlendMode(texture, SDL_BLENDMODE_BLEND);
        frameCache.emplace(key, texture);
        return texture;
    }

    void copyArgs(std::vector<int> const& values)
    {
        for (int& arg : args)
            arg = 0;
        for (size_t i = 0; i < values.size() && i < 8; ++i)
            args[i] = values[i];
    }

    int arg(int index) const
    {
        return index >= 0 && index < 8 ? args[index] : 0;
    }

    void offsetXArg(Sprite& sprite, int amount)
    {
        int attacker = logicalX(attackerIsPlayer, false);
        int target   = logicalX(targetIsPlayer, false);
        if (attacker > target)
            sprite.x -= amount;
        else
            sprite.x += amount;
    }

    void initAttacker(Sprite& sprite, bool picture)
    {
        sprite.x = logicalX(attackerIsPlayer, picture);
        sprite.y = logicalY(attackerIsPlayer, picture);
        offsetXArg(sprite, arg(0));
        sprite.y += arg(1);
    }

    void initTarget(Sprite& sprite, bool picture)
    {
        sprite.x = logicalX(targetIsPlayer, picture);
        sprite.y = logicalY(targetIsPlayer, picture);
        offsetXArg(sprite, arg(0));
        sprite.y += arg(1);
    }

    int battlerSlot(int animBattler) const
    {
        return sideIndex(animBattler);
    }

    void destroySprite(Sprite& sprite)
    {
        sprite.alive    = false;
        sprite.callback = nullptr;
    }

    void destroyTask(Task& task)
    {
        task.alive    = false;
        task.callback = nullptr;
    }

    void startAffine(Sprite& sprite, int anim)
    {
        if (!sprite.affine || sprite.affineAnims.empty())
        {
            sprite.affineEnded = true;
            return;
        }
        sprite.affineAnim  = std::clamp(anim, 0, static_cast<int>(sprite.affineAnims.size()) - 1);
        sprite.affineCmd   = 0;
        sprite.affineDelay = 0;
        sprite.xScale      = 256;
        sprite.yScale      = 256;
        sprite.rotation    = 0;
        sprite.affineEnded = false;
        applyAffine(sprite);
    }

    void applyAffine(Sprite& sprite)
    {
        auto const& anim = sprite.affineAnims[static_cast<size_t>(sprite.affineAnim)];
        if (sprite.affineCmd < 0 || sprite.affineCmd >= static_cast<int>(anim.size()))
        {
            sprite.affineEnded = true;
            return;
        }
        auto const& step = anim[static_cast<size_t>(sprite.affineCmd)];
        if (step.op == "end")
        {
            sprite.affineEnded = true;
            return;
        }
        if (step.op == "jump")
        {
            sprite.affineCmd = step.to;
            applyAffine(sprite);
            return;
        }
        if (step.op == "loop")
        {
            sprite.affineCmd = 0;
            return;
        }
        if (step.dur <= 0)
        {
            sprite.xScale = step.x;
            sprite.yScale = step.y;
            sprite.rotation = step.rot & 255;
            sprite.affineDelay = 0;
            ++sprite.affineCmd;
        }
        else
        {
            sprite.xScale += step.x;
            sprite.yScale += step.y;
            sprite.rotation = (sprite.rotation + step.rot) & 255;
            sprite.affineDelay = step.dur;
        }
    }

    void stepAffine(Sprite& sprite)
    {
        if (!sprite.affine || sprite.affineEnded)
            return;
        if (sprite.affineDelay > 0)
        {
            --sprite.affineDelay;
            if (sprite.affineDelay == 0)
            {
                ++sprite.affineCmd;
                applyAffine(sprite);
            }
            return;
        }
        applyAffine(sprite);
    }

    void stepAnim(Sprite& sprite)
    {
        if (sprite.anims.empty())
            return;
        int which = std::clamp(sprite.animNum, 0, static_cast<int>(sprite.anims.size()) - 1);
        auto const& anim = sprite.anims[static_cast<size_t>(which)];
        if (anim.empty())
        {
            sprite.animEnded = true;
            return;
        }
        if (sprite.animDelay > 0)
        {
            --sprite.animDelay;
            return;
        }
        if (sprite.animCmd < 0 || sprite.animCmd >= static_cast<int>(anim.size()))
        {
            sprite.animEnded = true;
            return;
        }
        auto const& step = anim[static_cast<size_t>(sprite.animCmd)];
        if (step.op == "end")
        {
            sprite.animEnded = true;
            return;
        }
        if (step.op == "jump")
        {
            sprite.animCmd = step.to;
            return;
        }
        if (step.op == "loop")
        {
            sprite.animCmd = 0;
            return;
        }
        sprite.tile  = step.tile;
        sprite.hFlip = step.hFlip;
        sprite.vFlip = step.vFlip;
        sprite.animDelay = std::max(0, step.dur - 1);
        ++sprite.animCmd;
    }

    void placeNew(Sprite& sprite)
    {
        sprite.x = logicalX(targetIsPlayer, true);
        sprite.y = logicalY(targetIsPlayer, true);
    }

    void spawn(Command const& command)
    {
        auto found = templates.find(command.text);
        if (found == templates.end() || sprites.size() >= 96)
            return;
        Sprite sprite;
        sprite.alive    = true;
        sprite.tag      = found->second.tileTag;
        sprite.width    = found->second.width;
        sprite.height   = found->second.height;
        sprite.blend    = found->second.blend;
        sprite.affine   = found->second.affine;
        sprite.priority = command.a;
        sprite.anims    = found->second.anims;
        sprite.affineAnims = found->second.affineAnims;
        sprite.affineEnded = !sprite.affine;
        placeNew(sprite);
        if (sprite.anims.size() == 1 && sprite.anims[0].size() == 1 && sprite.anims[0][0].op == "end")
            sprite.animEnded = false;
        copyArgs(command.args);
        sprites.push_back(sprite);
        Sprite& live = sprites.back();
        if (!dispatchSprite(found->second.callback, live))
            genericSprite(found->second.callback, live);
    }

    bool spawnTemplate(std::string const& name, int priority)
    {
        Command command;
        command.op   = 0x02;
        command.text = name;
        command.a    = priority;
        size_t before = sprites.size();
        spawn(command);
        return sprites.size() > before;
    }

    void spawnTask(Command const& command)
    {
        if (tasks.size() >= 64)
            return;
        Task task;
        task.alive    = true;
        task.priority = command.a;
        copyArgs(command.args);
        tasks.push_back(task);
        Task& live = tasks.back();
        if (!dispatchTask(command.text, live))
            genericTask(command.text, live);
    }

    void gotoScript(std::string const& name)
    {
        script = name;
        index  = 0;
    }

    void run()
    {
        for (int guard = 0; guard < 2000 && active && waitFrames == 0 && !waitVisual; ++guard)
        {
            auto found = scripts.find(script);
            if (found == scripts.end() || index < 0 || index >= static_cast<int>(found->second.size()))
            {
                if (!stack.empty())
                {
                    script = stack.back().first;
                    index  = stack.back().second;
                    stack.pop_back();
                    continue;
                }
                if (alive())
                {
                    waitVisual = true;
                    return;
                }
                finish();
                return;
            }
            Command const& command = found->second[static_cast<size_t>(index)];
            switch (command.op)
            {
            case 0x02:
                spawn(command);
                ++index;
                break;
            case 0x03:
                spawnTask(command);
                ++index;
                break;
            case 0x04:
                ++index;
                if (command.a > 0)
                    waitFrames = command.a;
                break;
            case 0x05:
                if (alive())
                    waitVisual = true;
                else
                    ++index;
                break;
            case 0x08:
                if (alive())
                    waitVisual = true;
                else
                    finish();
                return;
            case 0x0A:
            case 0x22:
                battlers[sideIndex(command.a)].blendOn = true;
                ++index;
                break;
            case 0x0B:
            case 0x23:
            case 0x0D:
                battlers[0].blendOn = false;
                battlers[1].blendOn = false;
                battlers[0].alpha   = 255;
                battlers[1].alpha   = 255;
                ++index;
                break;
            case 0x0C: {
                int sum   = std::max(1, command.a + command.b);
                int alpha = std::clamp(command.a * 255 / sum, 0, 255);
                if (battlers[0].blendOn)
                    battlers[0].alpha = alpha;
                if (battlers[1].blendOn)
                    battlers[1].alpha = alpha;
                ++index;
                break;
            }
            case 0x0E:
                stack.emplace_back(script, index + 1);
                gotoScript(command.text);
                break;
            case 0x0F:
                if (!stack.empty())
                {
                    script = stack.back().first;
                    index  = stack.back().second;
                    stack.pop_back();
                }
                else
                    ++index;
                break;
            case 0x10:
                if (command.a >= 0 && command.a < 8)
                    args[command.a] = command.b;
                ++index;
                break;
            case 0x11:
                gotoScript((turn & 1) ? command.text2 : command.text);
                break;
            case 0x12:
                if (command.a == turn)
                    gotoScript(command.text);
                else
                    ++index;
                break;
            case 0x13:
                gotoScript(command.text);
                break;
            case 0x14:
            case 0x18:
                background      = command.a;
                backgroundIn    = true;
                backgroundAlpha = 0;
                ++index;
                break;
            case 0x15:
                backgroundIn    = false;
                ++index;
                break;
            case 0x16:
            case 0x17:
                ++index;
                if (background >= 0)
                    waitFrames = 8;
                break;
            case 0x21:
                if (command.a >= 0 && command.a < 8 && args[command.a] == command.b)
                    gotoScript(command.text);
                else
                    ++index;
                break;
            case 0x24:
                ++index;
                break;
            case 0x25:
                background      = targetIsPlayer ? command.b : command.a;
                backgroundIn    = true;
                backgroundAlpha = 0;
                ++index;
                break;
            case 0x2B:
                battlers[sideIndex(command.a)].hidden = true;
                ++index;
                break;
            case 0x2C:
                battlers[sideIndex(command.a)].hidden = false;
                ++index;
                break;
            default:
                ++index;
                break;
            }
        }
    }

    void tick()
    {
        if (!active)
            return;
        if (++elapsed > 900)
        {
            finish();
            return;
        }
        for (auto& sprite : sprites)
        {
            if (!sprite.alive)
                continue;
            stepAffine(sprite);
            stepAnim(sprite);
            if (sprite.callback)
                sprite.callback(*this, &sprite);
        }
        for (auto& task : tasks)
        {
            if (task.alive && task.callback)
                task.callback(*this, &task);
        }
        if (background >= 0)
        {
            if (backgroundIn)
                backgroundAlpha = std::min(255, backgroundAlpha + 32);
            else
            {
                backgroundAlpha = std::max(0, backgroundAlpha - 32);
                if (backgroundAlpha == 0)
                    background = -1;
            }
        }
        if (waitFrames > 0)
        {
            --waitFrames;
            return;
        }
        if (waitVisual)
        {
            if (alive())
                return;
            waitVisual = false;
        }
        run();
    }

    void screenPoint(int logicalXIn, int logicalYIn, SDL_Rect const& player, SDL_Rect const& foe, int& outX, int& outY) const
    {
        float playerX = player.x + player.w / 2.f;
        float playerY = player.y + player.h / 2.f;
        float foeXPos = foe.x + foe.w / 2.f;
        float foeYPos = foe.y + foe.h / 2.f;
        float tx      = (logicalXIn - PlayerX) / static_cast<float>(FoeX - PlayerX);
        float ty      = (logicalYIn - PlayerY) / static_cast<float>(FoeY - PlayerY);
        outX          = static_cast<int>(std::lround(playerX + tx * (foeXPos - playerX)));
        outY          = static_cast<int>(std::lround(playerY + ty * (foeYPos - playerY)));
    }

    float pixelScale(SDL_Rect const& player, SDL_Rect const& foe) const
    {
        float dx = std::fabs((foe.x + foe.w / 2.f) - (player.x + player.w / 2.f));
        return std::max(0.5f, dx / static_cast<float>(FoeX - PlayerX));
    }

    bool dispatchSprite(std::string const& name, Sprite& sprite);
    bool dispatchTask(std::string const& name, Task& task);
    void genericSprite(std::string const& name, Sprite& sprite);
    void genericTask(std::string const& name, Task& task);
};

namespace
{
void destroySprite(BattleAnimPlayer::Impl& anim, BattleAnimPlayer::Impl::Sprite* sprite)
{
    anim.destroySprite(*sprite);
}

void runWhenAffineEnds(BattleAnimPlayer::Impl& anim, BattleAnimPlayer::Impl::Sprite* sprite)
{
    if (sprite->affineEnded && sprite->stored)
        sprite->stored(anim, sprite);
    else if (sprite->affineEnded)
        anim.destroySprite(*sprite);
}

void translateById(BattleAnimPlayer::Impl& anim, BattleAnimPlayer::Impl::Sprite* sprite)
{
    if (sprite->data[0] > 0)
    {
        --sprite->data[0];
        int slot = sprite->data[3];
        if (slot == AttackerId || slot == TargetId)
        {
            auto& battler = anim.battlers[slot == AttackerId ? (anim.attackerIsPlayer ? 1 : 0) : (anim.targetIsPlayer ? 1 : 0)];
            battler.x2 += sprite->data[1];
            battler.y2 += sprite->data[2];
        }
    }
    else if (sprite->stored)
        sprite->stored(anim, sprite);
    else
        anim.destroySprite(*sprite);
}

void reverseLunge(BattleAnimPlayer::Impl& anim, BattleAnimPlayer::Impl::Sprite* sprite)
{
    sprite->data[0] = sprite->data[4];
    sprite->data[1] = -sprite->data[1];
    sprite->stored = destroySprite;
    sprite->callback = translateById;
    (void)anim;
}

void doHorizontalLunge(BattleAnimPlayer::Impl& anim, BattleAnimPlayer::Impl::Sprite* sprite)
{
    sprite->visible = false;
    int delta = anim.arg(1);
    if (!anim.attackerIsPlayer)
        delta = -delta;
    sprite->data[0] = anim.arg(0);
    sprite->data[1] = delta;
    sprite->data[2] = 0;
    sprite->data[3] = AttackerId;
    sprite->data[4] = anim.arg(0);
    sprite->stored = reverseLunge;
    sprite->callback = translateById;
}

void reverseDip(BattleAnimPlayer::Impl& anim, BattleAnimPlayer::Impl::Sprite* sprite)
{
    sprite->data[0] = sprite->data[4];
    sprite->data[2] = -sprite->data[2];
    sprite->stored = destroySprite;
    sprite->callback = translateById;
    (void)anim;
}

void doVerticalDip(BattleAnimPlayer::Impl& anim, BattleAnimPlayer::Impl::Sprite* sprite)
{
    sprite->visible = false;
    sprite->data[0] = anim.arg(0);
    sprite->data[1] = 0;
    sprite->data[2] = anim.arg(1);
    sprite->data[3] = anim.arg(2) == 0 ? AttackerId : TargetId;
    sprite->data[4] = anim.arg(0);
    sprite->stored = reverseDip;
    sprite->callback = translateById;
}

void hitSplat(BattleAnimPlayer::Impl& anim, BattleAnimPlayer::Impl::Sprite* sprite)
{
    anim.startAffine(*sprite, anim.arg(3));
    if (anim.arg(2) == 0)
        anim.initAttacker(*sprite, true);
    else
        anim.initTarget(*sprite, true);
    sprite->stored = destroySprite;
    sprite->callback = runWhenAffineEnds;
}

void hitSplatInvert(BattleAnimPlayer::Impl& anim, BattleAnimPlayer::Impl::Sprite* sprite)
{
    if (!anim.attackerIsPlayer)
        anim.args[1] = -anim.args[1];
    hitSplat(anim, sprite);
}

void hitSplatRandom(BattleAnimPlayer::Impl& anim, BattleAnimPlayer::Impl::Sprite* sprite)
{
    anim.startAffine(*sprite, anim.arg(1) < 0 ? (std::rand() & 3) : anim.arg(1));
    if (anim.arg(0) == 0)
        anim.initAttacker(*sprite, false);
    else
        anim.initTarget(*sprite, false);
    sprite->x2 += (std::rand() % 48) - 24;
    sprite->y2 += (std::rand() % 24) - 12;
    sprite->stored = destroySprite;
    sprite->callback = runWhenAffineEnds;
}

void hitSplatEdge(BattleAnimPlayer::Impl& anim, BattleAnimPlayer::Impl::Sprite* sprite)
{
    bool player = anim.sidePlayer(anim.arg(0));
    sprite->x = anim.logicalX(player, true);
    sprite->y = anim.logicalY(player, true);
    sprite->x2 = anim.arg(1);
    sprite->y2 = anim.arg(2);
    anim.startAffine(*sprite, anim.arg(3));
    sprite->stored = destroySprite;
    sprite->callback = runWhenAffineEnds;
}

void hitSplatPersistent(BattleAnimPlayer::Impl& anim, BattleAnimPlayer::Impl::Sprite* sprite)
{
    hitSplat(anim, sprite);
    sprite->data[0] = anim.arg(4);
    sprite->stored = [](BattleAnimPlayer::Impl& engine, BattleAnimPlayer::Impl::Sprite* current) {
        if (current->data[0]-- <= 0)
            engine.destroySprite(*current);
    };
}

void crossImpact(BattleAnimPlayer::Impl& anim, BattleAnimPlayer::Impl::Sprite* sprite)
{
    if (anim.arg(2) == 0)
        anim.initAttacker(*sprite, true);
    else
        anim.initTarget(*sprite, true);
    sprite->data[0] = anim.arg(3);
    sprite->callback = [](BattleAnimPlayer::Impl& engine, BattleAnimPlayer::Impl::Sprite* current) {
        if (current->data[0]-- <= 0)
            engine.destroySprite(*current);
    };
}

void roarNoiseLine(BattleAnimPlayer::Impl& anim, BattleAnimPlayer::Impl::Sprite* sprite)
{
    int xOffset = anim.arg(0);
    if (!anim.attackerIsPlayer)
        xOffset = -xOffset;
    sprite->x = anim.logicalX(anim.attackerIsPlayer, false) + xOffset;
    sprite->y = anim.logicalY(anim.attackerIsPlayer, false) + anim.arg(1);

    int travelX = 0x280;
    int travelY = 0;
    bool verticalFlip = false;
    if (anim.arg(2) == 0)
        travelY = -0x280;
    else if (anim.arg(2) == 1)
    {
        verticalFlip = true;
        travelY      = 0x280;
    }
    else if (sprite->anims.size() > 1)
        sprite->animNum = 1;

    if (!anim.attackerIsPlayer)
    {
        travelX = -travelX;
        sprite->hFlip = true;
    }
    sprite->vFlip = verticalFlip;
    sprite->data[0] = travelX;
    sprite->data[1] = travelY;
    sprite->data[2] = sprite->hFlip ? 1 : 0;
    sprite->data[3] = verticalFlip ? 1 : 0;
    sprite->callback = [](BattleAnimPlayer::Impl& engine, BattleAnimPlayer::Impl::Sprite* current) {
        current->hFlip = current->data[2] != 0;
        current->vFlip = current->data[3] != 0;
        current->data[6] += current->data[0];
        current->data[7] += current->data[1];
        current->x2 = current->data[6] >> 8;
        current->y2 = current->data[7] >> 8;
        if (++current->data[5] == 14)
            engine.destroySprite(*current);
    };
}

void confusionDuck(BattleAnimPlayer::Impl& anim, BattleAnimPlayer::Impl::Sprite* sprite)
{
    anim.initTarget(*sprite, true);
    sprite->x += anim.arg(0);
    sprite->y += anim.arg(1);
    sprite->data[0] = anim.arg(2);
    int speed       = anim.arg(3);
    if (!anim.attackerIsPlayer)
        speed = -speed;
    sprite->data[1] = speed == 0 ? 3 : speed;
    sprite->data[3] = std::max(1, anim.arg(4));
    if (anim.attackerIsPlayer && sprite->anims.size() > 1)
        sprite->animNum = 1;
    sprite->callback = [](BattleAnimPlayer::Impl& engine, BattleAnimPlayer::Impl::Sprite* current) {
        current->x2     = sin256(current->data[0] + 64, 30);
        current->y2     = sin256(current->data[0], 10);
        current->data[0] = (current->data[0] + current->data[1]) & 255;
        if (++current->data[2] >= current->data[3])
            engine.destroySprite(*current);
    };
}

void flashingHit(BattleAnimPlayer::Impl& anim, BattleAnimPlayer::Impl::Sprite* sprite)
{
    anim.startAffine(*sprite, anim.arg(3));
    if (anim.arg(2) == 0)
        anim.initAttacker(*sprite, true);
    else
        anim.initTarget(*sprite, true);
    sprite->callback = [](BattleAnimPlayer::Impl& engine, BattleAnimPlayer::Impl::Sprite* current) {
        current->visible = !current->visible;
        if (++current->data[0] > 12)
            engine.destroySprite(*current);
    };
}

BattleAnimPlayer::Impl::Battler& mon(BattleAnimPlayer::Impl& anim, int id)
{
    if (id == AttackerId)
        return anim.battlers[anim.attackerIsPlayer ? 1 : 0];
    if (id == TargetId)
        return anim.battlers[anim.targetIsPlayer ? 1 : 0];
    return anim.battlers[anim.sideIndex(id)];
}

void shakeMon(BattleAnimPlayer::Impl& anim, BattleAnimPlayer::Impl::Task* task)
{
    auto& battler = mon(anim, anim.arg(0) == 0 ? AttackerId : TargetId);
    battler.x2 = anim.arg(1);
    battler.y2 = anim.arg(2);
    task->data[0] = anim.arg(0) == 0 ? AttackerId : TargetId;
    task->data[1] = anim.arg(3);
    task->data[2] = anim.arg(4);
    task->data[3] = anim.arg(4);
    task->data[4] = anim.arg(1);
    task->data[5] = anim.arg(2);
    task->callback = [](BattleAnimPlayer::Impl& engine, BattleAnimPlayer::Impl::Task* current) {
        auto& body = mon(engine, current->data[0]);
        if (current->data[3] == 0)
        {
            body.x2 = body.x2 == 0 ? current->data[4] : 0;
            body.y2 = body.y2 == 0 ? current->data[5] : 0;
            current->data[3] = current->data[2];
            if (--current->data[1] == 0)
            {
                body.x2 = 0;
                body.y2 = 0;
                engine.destroyTask(*current);
            }
        }
        else
            --current->data[3];
    };
}

void shakeMon2(BattleAnimPlayer::Impl& anim, BattleAnimPlayer::Impl::Task* task)
{
    shakeMon(anim, task);
    task->callback = [](BattleAnimPlayer::Impl& engine, BattleAnimPlayer::Impl::Task* current) {
        auto& body = mon(engine, current->data[0]);
        if (current->data[3] == 0)
        {
            body.x2 = body.x2 == current->data[4] ? -current->data[4] : current->data[4];
            body.y2 = body.y2 == current->data[5] ? -current->data[5] : current->data[5];
            current->data[3] = current->data[2];
            if (--current->data[1] == 0)
            {
                body.x2 = 0;
                body.y2 = 0;
                engine.destroyTask(*current);
            }
        }
        else
            --current->data[3];
    };
}

void shakeInPlace(BattleAnimPlayer::Impl& anim, BattleAnimPlayer::Impl::Task* task)
{
    int id = anim.arg(0) == 0 ? AttackerId : TargetId;
    auto& battler = mon(anim, id);
    battler.x2 += anim.arg(1);
    battler.y2 += anim.arg(2);
    task->data[0] = id;
    task->data[1] = 0;
    task->data[2] = anim.arg(3);
    task->data[3] = 0;
    task->data[4] = anim.arg(4);
    task->data[5] = anim.arg(1) * 2;
    task->data[6] = anim.arg(2) * 2;
    task->callback = [](BattleAnimPlayer::Impl& engine, BattleAnimPlayer::Impl::Task* current) {
        auto& body = mon(engine, current->data[0]);
        if (current->data[3] == 0)
        {
            int sign = (current->data[1] & 1) ? 1 : -1;
            body.x2 += sign * current->data[5];
            body.y2 += sign * current->data[6];
            current->data[3] = current->data[4];
            if (++current->data[1] >= current->data[2])
            {
                body.x2 = 0;
                body.y2 = 0;
                engine.destroyTask(*current);
            }
        }
        else
            --current->data[3];
    };
}

void scaleMon(BattleAnimPlayer::Impl& anim, BattleAnimPlayer::Impl::Task* task)
{
    int id = anim.arg(3) == 0 ? AttackerId : TargetId;
    task->data[0] = anim.arg(0);
    task->data[1] = anim.arg(1);
    task->data[2] = anim.arg(2);
    task->data[3] = anim.arg(2);
    task->data[4] = id;
    task->data[10] = 256;
    task->data[11] = 256;
    task->callback = [](BattleAnimPlayer::Impl& engine, BattleAnimPlayer::Impl::Task* current) {
        current->data[10] += current->data[0];
        current->data[11] += current->data[1];
        auto& body = mon(engine, current->data[4]);
        body.scaleX = current->data[10];
        body.scaleY = current->data[11];
        if (--current->data[2] == 0)
        {
            if (current->data[3] > 0)
            {
                current->data[0] = -current->data[0];
                current->data[1] = -current->data[1];
                current->data[2] = current->data[3];
                current->data[3] = 0;
            }
            else
            {
                body.scaleX = 256;
                body.scaleY = 256;
                engine.destroyTask(*current);
            }
        }
    };
}

void elliptical(BattleAnimPlayer::Impl& anim, BattleAnimPlayer::Impl::Task* task, bool respectSide)
{
    int width = anim.arg(1);
    if (respectSide && !anim.attackerIsPlayer)
        width = -width;
    int period = 1;
    int speed = std::clamp(anim.arg(4), 0, 5);
    for (int i = 0; i < speed; ++i)
        period *= 2;
    task->data[0] = anim.arg(0) == 0 ? AttackerId : TargetId;
    task->data[1] = width;
    task->data[2] = anim.arg(2);
    task->data[3] = std::max(1, anim.arg(3));
    task->data[4] = period;
    task->callback = [](BattleAnimPlayer::Impl& engine, BattleAnimPlayer::Impl::Task* current) {
        auto& body = mon(engine, current->data[0]);
        body.x2 = sin256(current->data[5], current->data[1]);
        body.y2 = -sin256(current->data[5] + 64, current->data[2]) + current->data[2];
        current->data[5] = (current->data[5] + current->data[4]) & 255;
        if (current->data[5] == 0 && --current->data[3] == 0)
        {
            body.x2 = 0;
            body.y2 = 0;
            engine.destroyTask(*current);
        }
    };
}

void swayMon(BattleAnimPlayer::Impl& anim, BattleAnimPlayer::Impl::Task* task)
{
    int amplitude = anim.arg(1);
    if (!anim.attackerIsPlayer)
        amplitude = -amplitude;
    task->data[0] = anim.arg(0);
    task->data[1] = amplitude;
    task->data[2] = std::max(1, anim.arg(2));
    task->data[3] = std::max(1, anim.arg(3));
    task->data[4] = anim.arg(4) == 0 ? AttackerId : TargetId;
    task->callback = [](BattleAnimPlayer::Impl& engine, BattleAnimPlayer::Impl::Task* current) {
        current->data[5] += current->data[2];
        int wave = sin256(current->data[5] >> 8, current->data[1]);
        auto& body = mon(engine, current->data[4]);
        if (current->data[0] == 0)
            body.x2 = wave;
        else
            body.y2 = std::abs(wave);
        if ((current->data[5] >> 8) > 127 && ++current->data[6] > 0)
        {
            current->data[5] = 0;
            if (--current->data[3] == 0)
            {
                body.x2 = 0;
                body.y2 = 0;
                engine.destroyTask(*current);
            }
        }
    };
}

void shakeByPower(BattleAnimPlayer::Impl& anim, BattleAnimPlayer::Impl::Task* task)
{
    int magnitude = anim.arg(0) == 0 ? anim.power / 12 : anim.damage / 12;
    magnitude = std::clamp(magnitude, 1, 16);
    task->data[0] = TargetId;
    task->data[1] = std::max(1, anim.arg(1));
    task->data[2] = std::max(1, anim.arg(2));
    task->data[3] = magnitude;
    task->data[4] = anim.arg(3);
    task->data[5] = anim.arg(4);
    task->callback = [](BattleAnimPlayer::Impl& engine, BattleAnimPlayer::Impl::Task* current) {
        auto& body = mon(engine, current->data[0]);
        if (++current->data[6] > current->data[1])
        {
            current->data[6] = 0;
            current->data[7] ^= 1;
            if (current->data[4])
                body.x2 = current->data[7] ? current->data[3] / 2 : -(current->data[3] / 2);
            if (current->data[5])
                body.y2 = current->data[7] ? current->data[3] : 0;
            if (--current->data[2] == 0)
            {
                body.x2 = 0;
                body.y2 = 0;
                engine.destroyTask(*current);
            }
        }
    };
}

void tintFrom555(BattleAnimPlayer::Impl::Battler& battler, int color, int coeff)
{
    int red   = ((color) & 31) * 255 / 31;
    int green = ((color >> 5) & 31) * 255 / 31;
    int blue  = ((color >> 10) & 31) * 255 / 31;
    int mix   = std::clamp(coeff, 0, 16);
    battler.tintR = (255 * (16 - mix) + red * mix) / 16;
    battler.tintG = (255 * (16 - mix) + green * mix) / 16;
    battler.tintB = (255 * (16 - mix) + blue * mix) / 16;
    battler.tintA = mix == 0 ? 0 : 255;
}

void clearTint(BattleAnimPlayer::Impl::Battler& battler)
{
    battler.tintR = 255;
    battler.tintG = 255;
    battler.tintB = 255;
    battler.tintA = 0;
}

void blendPal(BattleAnimPlayer::Impl& anim, BattleAnimPlayer::Impl::Task* task)
{
    task->data[0] = anim.arg(0);
    task->data[1] = std::max(0, anim.arg(1));
    task->data[2] = anim.arg(2);
    task->data[3] = anim.arg(3);
    task->data[4] = anim.arg(4);
    task->data[5] = anim.arg(2);
    task->callback = [](BattleAnimPlayer::Impl& engine, BattleAnimPlayer::Impl::Task* current) {
        if (current->data[6]++ < current->data[1])
            return;
        current->data[6] = 0;
        auto paint = [&](int slot) {
            tintFrom555(engine.battlers[slot], current->data[4], current->data[5]);
        };
        int which = current->data[0];
        if (which & 0x2)
            paint(engine.attackerIsPlayer ? 1 : 0);
        if (which & 0x4)
            paint(engine.targetIsPlayer ? 1 : 0);
        if ((which & 0x6) == 0)
        {
            paint(0);
            paint(1);
        }
        if (current->data[5] < current->data[3])
            ++current->data[5];
        else if (current->data[5] > current->data[3])
            --current->data[5];
        else
        {
            clearTint(engine.battlers[0]);
            clearTint(engine.battlers[1]);
            engine.destroyTask(*current);
        }
    };
}

void blendCycle(BattleAnimPlayer::Impl& anim, BattleAnimPlayer::Impl::Task* task)
{
    task->data[0] = anim.arg(0);
    task->data[1] = std::max(1, anim.arg(1));
    task->data[2] = std::max(1, anim.arg(2));
    task->data[3] = anim.arg(4);
    task->data[4] = anim.arg(5);
    task->data[5] = 0;
    task->callback = [](BattleAnimPlayer::Impl& engine, BattleAnimPlayer::Impl::Task* current) {
        if (++current->data[6] < current->data[1])
            return;
        current->data[6] = 0;
        int phase = current->data[5]++ % 2;
        int coeff = phase == 0 ? current->data[3] : 0;
        auto paint = [&](int slot) { tintFrom555(engine.battlers[slot], current->data[4], coeff); };
        if (current->data[0] & 0x2)
            paint(engine.attackerIsPlayer ? 1 : 0);
        if (current->data[0] & 0x4)
            paint(engine.targetIsPlayer ? 1 : 0);
        if ((current->data[0] & 0x6) == 0)
        {
            paint(0);
            paint(1);
        }
        if (current->data[5] >= current->data[2] * 2)
        {
            clearTint(engine.battlers[0]);
            clearTint(engine.battlers[1]);
            engine.destroyTask(*current);
        }
    };
}

void invertScreen(BattleAnimPlayer::Impl& anim, BattleAnimPlayer::Impl::Task* task)
{
    task->data[0] = std::max(4, anim.arg(0));
    task->callback = [](BattleAnimPlayer::Impl& engine, BattleAnimPlayer::Impl::Task* current) {
        tintFrom555(engine.battlers[0], 0x7FFF, 10);
        tintFrom555(engine.battlers[1], 0x7FFF, 10);
        if (--current->data[0] <= 0)
        {
            clearTint(engine.battlers[0]);
            clearTint(engine.battlers[1]);
            engine.destroyTask(*current);
        }
    };
    (void)anim;
}

void holdSprite(BattleAnimPlayer::Impl& anim, BattleAnimPlayer::Impl::Sprite* sprite);

void statsChange(BattleAnimPlayer::Impl& anim, BattleAnimPlayer::Impl::Task* task)
{
    task->data[0] = 0;
    task->data[1] = anim.attackerIsPlayer ? 1 : 0;
    task->callback = [](BattleAnimPlayer::Impl& engine, BattleAnimPlayer::Impl::Task* current) {
        auto& body = engine.battlers[current->data[1]];
        int   t    = current->data[0]++;
        int   lift = t < 10 ? t : 20 - t;
        body.scaleX = body.scaleY = 256 + std::max(0, lift) * 6;
        body.y2     = -std::max(0, lift);
        body.tintA  = 8;
        body.tintR  = 180;
        body.tintG  = 255;
        body.tintB  = 180;
        if (t >= 20)
        {
            body.scaleX = body.scaleY = 256;
            body.y2     = 0;
            body.tintA  = 0;
            engine.destroyTask(*current);
        }
    };
}

void frozenIceCube(BattleAnimPlayer::Impl& anim, BattleAnimPlayer::Impl::Task* task)
{
    if (anim.spawnTemplate("sFrozenIceCubeSpriteTemplate", 4) && !anim.sprites.empty())
    {
        auto& cube   = anim.sprites.back();
        cube.data[0] = 48;
        cube.callback = holdSprite;
    }
    anim.destroyTask(*task);
}

void createRaindrops(BattleAnimPlayer::Impl& anim, BattleAnimPlayer::Impl::Task* task)
{
    task->data[0] = 0;
    task->data[1] = std::max(1, anim.arg(1));
    task->data[2] = std::max(1, anim.arg(2));
    task->callback = [](BattleAnimPlayer::Impl& engine, BattleAnimPlayer::Impl::Task* current) {
        if (++current->data[0] % current->data[1] == 1 && engine.spawnTemplate("gRainDropSpriteTemplate", 4)
            && !engine.sprites.empty())
        {
            auto& drop = engine.sprites.back();
            drop.x     = std::rand() % 240;
            drop.y     = std::rand() % 48;
        }
        if (current->data[0] >= current->data[2])
            engine.destroyTask(*current);
    };
}

void holdSprite(BattleAnimPlayer::Impl& anim, BattleAnimPlayer::Impl::Sprite* sprite)
{
    if (++sprite->life > sprite->data[0])
        anim.destroySprite(*sprite);
}

void projectileSprite(BattleAnimPlayer::Impl& anim, BattleAnimPlayer::Impl::Sprite* sprite)
{
    int step = ++sprite->life;
    int span = std::max(1, sprite->data[0]);
    sprite->x = sprite->data[1] + (sprite->goalX - sprite->data[1]) * step / span;
    sprite->y = sprite->data[2] + (sprite->goalY - sprite->data[2]) * step / span;
    if (step >= span)
        anim.destroySprite(*sprite);
}

} // namespace

bool BattleAnimPlayer::Impl::dispatchSprite(std::string const& name, Sprite& sprite)
{
    if (name == "DoHorizontalLunge")
        doHorizontalLunge(*this, &sprite);
    else if (name == "DoVerticalDip")
        doVerticalDip(*this, &sprite);
    else if (name == "AnimHitSplatBasic")
        hitSplat(*this, &sprite);
    else if (name == "AnimHitSplatHandleInvert")
        hitSplatInvert(*this, &sprite);
    else if (name == "AnimHitSplatRandom")
        hitSplatRandom(*this, &sprite);
    else if (name == "AnimHitSplatOnMonEdge")
        hitSplatEdge(*this, &sprite);
    else if (name == "AnimHitSplatPersistent")
        hitSplatPersistent(*this, &sprite);
    else if (name == "AnimCrossImpact")
        crossImpact(*this, &sprite);
    else if (name == "AnimFlashingHitSplat")
        flashingHit(*this, &sprite);
    else if (name == "AnimConfusionDuck")
        confusionDuck(*this, &sprite);
    else if (name == "AnimRoarNoiseLine")
        roarNoiseLine(*this, &sprite);
    else if (name == "AnimRainDrop")
    {
        sprite.callback = [](BattleAnimPlayer::Impl& engine, BattleAnimPlayer::Impl::Sprite* current) {
            current->y += 6;
            if (current->y > 160)
                engine.destroySprite(*current);
        };
    }
    else
        return false;
    return true;
}

bool BattleAnimPlayer::Impl::dispatchTask(std::string const& name, Task& task)
{
    if (name == "AnimTask_ShakeMon")
        shakeMon(*this, &task);
    else if (name == "AnimTask_ShakeMon2")
        shakeMon2(*this, &task);
    else if (name == "AnimTask_ShakeMonInPlace")
        shakeInPlace(*this, &task);
    else if (name == "AnimTask_ScaleMonAndRestore")
        scaleMon(*this, &task);
    else if (name == "AnimTask_TranslateMonElliptical")
        elliptical(*this, &task, false);
    else if (name == "AnimTask_TranslateMonEllipticalRespectSide")
        elliptical(*this, &task, true);
    else if (name == "AnimTask_SwayMon")
        swayMon(*this, &task);
    else if (name == "AnimTask_ShakeTargetBasedOnMovePowerOrDmg")
        shakeByPower(*this, &task);
    else if (name == "AnimTask_BlendBattleAnimPal" || name == "AnimTask_BlendBattleAnimPalExclude")
        blendPal(*this, &task);
    else if (name == "AnimTask_BlendColorCycle" || name == "AnimTask_BlendColorCycleExclude")
        blendCycle(*this, &task);
    else if (name == "AnimTask_InvertScreenColor")
        invertScreen(*this, &task);
    else if (name == "AnimTask_StatsChange")
        statsChange(*this, &task);
    else if (name == "AnimTask_FrozenIceCube")
        frozenIceCube(*this, &task);
    else if (name == "AnimTask_CreateRaindrops")
        createRaindrops(*this, &task);
    else
        return false;
    return true;
}

void BattleAnimPlayer::Impl::genericSprite(std::string const& name, Sprite& sprite)
{
    auto found = profiles.find(name);
    Profile profile = found == profiles.end() ? Profile{"sprite", "target", "hold"} : found->second;
    if (profile.kind == "projectile" || profile.anchor == "attacker")
        initAttacker(sprite, true);
    else if (profile.anchor == "either")
    {
        if (arg(2) == 0)
            initAttacker(sprite, true);
        else
            initTarget(sprite, true);
    }
    else
        initTarget(sprite, true);

    if (profile.kind == "projectile")
    {
        sprite.data[0] = 24;
        sprite.data[1] = sprite.x;
        sprite.data[2] = sprite.y;
        sprite.goalX = logicalX(targetIsPlayer, true);
        sprite.goalY = logicalY(targetIsPlayer, true);
        sprite.callback = projectileSprite;
        return;
    }
    bool staticSheet = sprite.anims.empty() || (sprite.anims.size() == 1 && !sprite.anims[0].empty() && sprite.anims[0][0].op == "end");
    if (profile.life == "affine" && sprite.affine && !sprite.affineAnims.empty())
    {
        startAffine(sprite, arg(3));
        sprite.stored   = [](Impl& engine, Sprite* current) { engine.destroySprite(*current); };
        sprite.callback = runWhenAffineEnds;
        return;
    }
    sprite.data[0] = profile.life == "timer" ? std::max(1, arg(4)) : (staticSheet ? 18 : 32);
    sprite.callback = holdSprite;
}

void BattleAnimPlayer::Impl::genericTask(std::string const& name, Task& task)
{
    auto found = profiles.find(name);
    std::string kind = found == profiles.end() ? "task" : found->second.kind;
    if (kind == "shake")
    {
        shakeMon(*this, &task);
        return;
    }
    if (kind == "blend")
    {
        blendPal(*this, &task);
        return;
    }
    if (kind == "scale")
    {
        scaleMon(*this, &task);
        return;
    }
    if (kind == "sound" || kind == "task")
    {
        destroyTask(task);
        return;
    }
    task.data[0] = 12;
    task.callback = [](Impl& engine, Task* current) {
        if (--current->data[0] <= 0)
            engine.destroyTask(*current);
    };
}

BattleAnimPlayer::BattleAnimPlayer(SDL_Renderer* renderer)
    : impl(std::make_unique<Impl>(renderer))
{
}

BattleAnimPlayer::~BattleAnimPlayer() = default;

bool BattleAnimPlayer::canPlay(std::string const& moveId) const
{
    auto found = impl->moves.find(moveId);
    if (found == impl->moves.end())
        return false;
    return impl->scriptReady(found->second, 0);
}

bool BattleAnimPlayer::canPlayScript(std::string const& scriptName) const
{
    return impl && impl->scriptReady(scriptName, 0);
}

void BattleAnimPlayer::start(std::string const& moveId,
                             bool                attackerIsPlayer,
                             bool                targetIsPlayer,
                             int                 turn,
                             int                 power,
                             int                 damage)
{
    stop();
    auto found = impl->moves.find(moveId);
    if (found == impl->moves.end() || !impl->scriptReady(found->second, 0))
        return;
    impl->active           = true;
    impl->attackerIsPlayer = attackerIsPlayer;
    impl->targetIsPlayer   = targetIsPlayer;
    impl->turn             = turn;
    impl->power            = power;
    impl->damage           = damage;
    impl->script           = found->second;
    impl->index            = 0;
    impl->elapsed          = 0;
    impl->clearPoses();
    impl->run();
}

void BattleAnimPlayer::startScript(std::string const& scriptName, bool attackerIsPlayer, bool targetIsPlayer)
{
    stop();
    if (!impl || !impl->scriptReady(scriptName, 0))
        return;
    impl->active           = true;
    impl->attackerIsPlayer = attackerIsPlayer;
    impl->targetIsPlayer   = targetIsPlayer;
    impl->turn             = 0;
    impl->power            = 0;
    impl->damage           = 0;
    impl->script           = scriptName;
    impl->index            = 0;
    impl->elapsed          = 0;
    impl->clearPoses();
    impl->run();
}

void BattleAnimPlayer::stop()
{
    if (impl)
        impl->finish();
}

bool BattleAnimPlayer::running() const
{
    return impl && impl->active;
}

void BattleAnimPlayer::tick()
{
    if (impl)
        impl->tick();
}

void BattleAnimPlayer::drawBackground(SDL_Rect const& area) const
{
    if (!impl || impl->background < 0 || impl->backgroundAlpha <= 0)
        return;
    SDL_Texture* texture = impl->backgroundTexture(impl->background);
    if (!texture)
        return;
    SDL_SetTextureAlphaMod(texture, static_cast<Uint8>(impl->backgroundAlpha));
    SDL_RenderCopy(impl->renderer, texture, nullptr, &area);
    SDL_SetTextureAlphaMod(texture, 255);
}

void BattleAnimPlayer::drawSprites(SDL_Rect const& playerSprite, SDL_Rect const& foeSprite) const
{
    if (!impl || !impl->active)
        return;
    float scale = impl->pixelScale(playerSprite, foeSprite);
    std::vector<Impl::Sprite const*> order;
    for (auto const& sprite : impl->sprites)
    {
        if (sprite.alive && sprite.visible && sprite.width > 0)
            order.push_back(&sprite);
    }
    std::stable_sort(order.begin(), order.end(), [](Impl::Sprite const* left, Impl::Sprite const* right) {
        return left->priority < right->priority;
    });
    for (auto const* sprite : order)
    {
        SDL_Texture* texture = impl->frameFor(*sprite);
        if (!texture)
            continue;
        int centerX = 0;
        int centerY = 0;
        impl->screenPoint(sprite->x + sprite->x2, sprite->y + sprite->y2, playerSprite, foeSprite, centerX, centerY);
        float drawScaleX = scale * (sprite->xScale / 256.f);
        float drawScaleY = scale * (sprite->yScale / 256.f);
        SDL_Rect dst;
        dst.w = std::max(1, static_cast<int>(sprite->width * drawScaleX));
        dst.h = std::max(1, static_cast<int>(sprite->height * drawScaleY));
        dst.x = centerX - dst.w / 2;
        dst.y = centerY - dst.h / 2;
        SDL_RendererFlip flip = SDL_FLIP_NONE;
        if (sprite->hFlip)
            flip = static_cast<SDL_RendererFlip>(flip | SDL_FLIP_HORIZONTAL);
        if (sprite->vFlip)
            flip = static_cast<SDL_RendererFlip>(flip | SDL_FLIP_VERTICAL);
        SDL_RenderCopyEx(impl->renderer, texture, nullptr, &dst, sprite->rotation * 360.0 / 256.0, nullptr, flip);
    }
}

int BattleAnimPlayer::offsetX(bool playerSide, int spriteWidth) const
{
    if (!impl || !impl->active)
        return 0;
    return impl->battlers[playerSide ? 1 : 0].x2 * spriteWidth / 64;
}

int BattleAnimPlayer::offsetY(bool playerSide, int spriteHeight) const
{
    if (!impl || !impl->active)
        return 0;
    return impl->battlers[playerSide ? 1 : 0].y2 * spriteHeight / 64;
}

float BattleAnimPlayer::monScaleX(bool playerSide) const
{
    if (!impl || !impl->active)
        return 1.f;
    return impl->battlers[playerSide ? 1 : 0].scaleX / 256.f;
}

float BattleAnimPlayer::monScaleY(bool playerSide) const
{
    if (!impl || !impl->active)
        return 1.f;
    return impl->battlers[playerSide ? 1 : 0].scaleY / 256.f;
}

void BattleAnimPlayer::battlerColor(bool playerSide, Uint8& red, Uint8& green, Uint8& blue) const
{
    red = green = blue = 255;
    if (!impl || !impl->active)
        return;
    auto const& battler = impl->battlers[playerSide ? 1 : 0];
    if (battler.tintA == 0)
        return;
    red   = static_cast<Uint8>(battler.tintR);
    green = static_cast<Uint8>(battler.tintG);
    blue  = static_cast<Uint8>(battler.tintB);
}

Uint8 BattleAnimPlayer::battlerAlpha(bool playerSide) const
{
    if (!impl || !impl->active)
        return 255;
    return static_cast<Uint8>(impl->battlers[playerSide ? 1 : 0].alpha);
}

bool BattleAnimPlayer::battlerHidden(bool playerSide) const
{
    return impl && impl->active && impl->battlers[playerSide ? 1 : 0].hidden;
}
