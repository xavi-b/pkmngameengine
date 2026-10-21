#ifndef BATTLEANIM_H
#define BATTLEANIM_H

#include <SDL.h>

#include <memory>
#include <string>

class BattleAnimPlayer
{
public:
    struct Impl;

    explicit BattleAnimPlayer(SDL_Renderer* renderer);
    ~BattleAnimPlayer();

    bool canPlay(std::string const& moveId) const;
    bool canPlayScript(std::string const& scriptName) const;
    void start(std::string const& moveId,
               bool                attackerIsPlayer,
               bool                targetIsPlayer,
               int                 turn,
               int                 power,
               int                 damage);
    void startScript(std::string const& scriptName, bool attackerIsPlayer, bool targetIsPlayer);
    void stop();
    bool running() const;
    void tick();

    void drawBackground(SDL_Rect const& area) const;
    void drawSprites(SDL_Rect const& playerSprite, SDL_Rect const& foeSprite) const;

    int   offsetX(bool playerSide, int spriteWidth) const;
    int   offsetY(bool playerSide, int spriteHeight) const;
    float monScaleX(bool playerSide) const;
    float monScaleY(bool playerSide) const;
    void  battlerColor(bool playerSide, Uint8& red, Uint8& green, Uint8& blue) const;
    Uint8 battlerAlpha(bool playerSide) const;
    bool  battlerHidden(bool playerSide) const;

private:
    std::unique_ptr<Impl> impl;
};

#endif // BATTLEANIM_H
