#include "keyboard.h"

Keyboard::Keyboard(SDL_Renderer* renderer, std::string& textOut, size_t maxSize)
    : renderer(renderer), textOut(textOut), maxSize(maxSize)
{
    bgSurface         = IMG_Load("resources/Graphics/Pictures/Naming/bg.png");
    bgTexture         = SDL_CreateTextureFromSurface(renderer, bgSurface);
    keyboardBgSurface = IMG_Load("resources/Graphics/Pictures/Naming/overlay_tab_1.png");
    keyboardBgTexture = SDL_CreateTextureFromSurface(renderer, keyboardBgSurface);
    cursorSurface     = IMG_Load("resources/Graphics/Pictures/Naming/cursor_1.png");
    cursorTexture     = SDL_CreateTextureFromSurface(renderer, cursorSurface);
}

Keyboard::~Keyboard()
{
    SDL_DestroyTexture(bgTexture);
    SDL_FreeSurface(bgSurface);
    SDL_DestroyTexture(keyboardBgTexture);
    SDL_FreeSurface(keyboardBgSurface);
    SDL_DestroyTexture(cursorTexture);
    SDL_FreeSurface(cursorSurface);
}

void Keyboard::init()
{
    currentPos = 0;
}

void Keyboard::update(Inputs const* inputs)
{
    int col = currentPos % NbRowBoxes;
    int row = currentPos / NbRowBoxes;

    static constexpr int maxCol = MaxPos % NbRowBoxes;
    static constexpr int maxRow = MaxPos / NbRowBoxes;

    if (inputs->A)
    {
        if (textOut.size() < maxSize)
        {
            char startChar = textOut.empty() ? 'A' : 'a';
            textOut.push_back(startChar + currentPos);
        }
        else
        {
            leave = true;
        }
    }
    else if (inputs->up)
    {
        row = row - 1;
        if (row < 0)
        {
            row = maxRow;
            if (row == maxRow && col > maxCol)
                row = maxRow - 1;
        }
        currentPos = row * NbRowBoxes + col;
    }
    else if (inputs->down)
    {
        row = row + 1;
        if (row > maxRow)
            row = 0;
        else if (row == maxRow && col > maxCol)
            row = 0;
        currentPos = row * NbRowBoxes + col;
    }
    else if (inputs->left)
    {
        col = (col - 1 + NbRowBoxes) % NbRowBoxes;
        if (row == maxRow && col > maxCol)
            col = maxCol;
        currentPos = row * NbRowBoxes + col;
    }
    else if (inputs->right)
    {
        col = (col + 1) % NbRowBoxes;
        if (row == maxRow && col > maxCol)
            col = 0;
        currentPos = row * NbRowBoxes + col;
    }
    else if (inputs->B)
    {
        textOut.erase(textOut.size() - 1, 1);
    }
    else if (inputs->start)
    {
        if (!textOut.empty())
            leave = true;
    }
}

void Keyboard::draw(Fps const* /*fps*/, RenderSizes rs)
{
    SDL_RenderCopy(renderer, bgTexture, NULL, NULL);

    int const topMargin    = 96;
    int const dstTopMargin = topMargin * rs.wh / rs.ah;

    SDL_Rect dstRect;
    dstRect.x = (rs.ww - keyboardBgSurface->w * rs.ww / rs.aw) / 2;
    dstRect.y = dstTopMargin + (rs.wh - dstTopMargin - keyboardBgSurface->h * rs.wh / rs.ah) / 2;
    dstRect.w = keyboardBgSurface->w * rs.ww / rs.aw;
    dstRect.h = keyboardBgSurface->h * rs.wh / rs.ah;
    SDL_RenderCopy(renderer, keyboardBgTexture, NULL, &dstRect);

    int boxWidth  = 32;
    int boxHeight = 38;
    int startX    = 60;
    int startY    = 52;
    int fontSize  = 24;

    for (int pos = 0; pos <= MaxPos; ++pos)
    {
        int col = pos % NbRowBoxes;
        int row = pos / NbRowBoxes;

        SDL_Rect textRect;
        textRect.x     = dstRect.x + (startX + col * boxWidth) * rs.ww / rs.aw;
        textRect.y     = dstRect.y + (startY + row * boxHeight + (boxHeight - fontSize - 6) / 2) * rs.wh / rs.ah;
        textRect.w     = boxWidth * rs.ww / rs.aw;
        char startChar = textOut.empty() ? 'A' : 'a';
        RenderUtils::drawWhiteTextCentered(renderer, rs, std::string() + char(pos + startChar), fontSize, textRect);

        if (pos == currentPos)
        {
            SDL_Rect cursorRect;
            cursorRect.x = dstRect.x + (startX + col * boxWidth) * rs.ww / rs.aw;
            cursorRect.y = dstRect.y + (startY + row * boxHeight) * rs.wh / rs.ah;
            cursorRect.w = boxWidth * rs.ww / rs.aw;
            cursorRect.h = boxHeight * rs.wh / rs.ah;
            SDL_RenderCopy(renderer, cursorTexture, NULL, &cursorRect);
        }
    }

    std::string text = textOut;
    text.append(maxSize - text.size(), '_');
    SDL_Rect textRect;
    textRect.x = 0;
    textRect.y = (dstTopMargin - fontSize * rs.wh / rs.ah) / 2;
    textRect.w = rs.ww;
    RenderUtils::drawWhiteTextCentered(renderer, rs, text, fontSize, textRect);
}

bool Keyboard::isFinished() const
{
    return leave;
}
