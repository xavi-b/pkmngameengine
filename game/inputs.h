#ifndef INPUTS_H
#define INPUTS_H

#include <SDL.h>
#include <cstddef>

struct Inputs
{
    static size_t newToken()
    {
        static size_t next = 1;
        return next++;
    }

    // Same handler: a hold repeats after RepeatGapTicks.
    // A new handler ignores keys until release, except when both handlers are map scenes.
    enum class Handler
    {
        Exclusive,
        MapScene
    };

    void setHandler(size_t token, Handler handler = Handler::Exclusive) const;

    void update();
    void clear();

    bool A      = false;
    bool B      = false;
    bool up     = false;
    bool down   = false;
    bool left   = false;
    bool right  = false;
    bool start  = false;
    bool select = false;
    bool debug  = false;

    // Continuous hold for walking and running. The flags above are discrete consumes.
    bool bHeld     = false;
    bool upHeld    = false;
    bool downHeld  = false;
    bool leftHeld  = false;
    bool rightHeld = false;

private:
    // Logic ticks are 100 ms, so 3 ticks is 300 ms between repeated consumes.
    static constexpr int RepeatGapTicks = 3;

    struct Key
    {
        bool blocked   = false;
        bool wasDown   = false;
        int  ticksHeld = 0;
        bool physical  = false;
    };

    void adoptHandler(size_t token, Handler handler);
    bool consume(bool down, Key& key);
    void latch(bool& pulsed, Key& key);

    size_t  activeToken   = 0;
    Handler activeHandler = Handler::Exclusive;
    Key     keyA;
    Key     keyB;
    Key     keyUp;
    Key     keyDown;
    Key     keyLeft;
    Key     keyRight;
    Key     keyStart;
    Key     keySelect;
    Key     keyDebug;
};

#endif // ITEM_H
