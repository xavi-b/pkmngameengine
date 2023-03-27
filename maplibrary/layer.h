#ifndef LAYER_H
#define LAYER_H

#include <vector>
#include <memory>

template <class T>
class Layer
{
public:
    Layer()
    {
    }

protected:
    std::vector<std::unique_ptr<T>> elements;
};

#endif // LAYER_H
