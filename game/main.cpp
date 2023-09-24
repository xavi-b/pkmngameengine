#include "game.h"

int main(int argc, char* argv[])
{
    std::srand(std::time(nullptr));
    Game game(argc, argv);
    return game.exec();
}
