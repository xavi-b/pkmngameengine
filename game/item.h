#ifndef ITEM_H
#define ITEM_H

#include <string>

class Item
{
public:
    enum Type
    {
        Ball
    };

    Item();

private:
    std::string name;
    Type        type;
    size_t      count = 0;
};

#endif // ITEM_H
