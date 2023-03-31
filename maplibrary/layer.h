#ifndef LAYER_H
#define LAYER_H

#include <vector>
#include <memory>

#include <boost/json.hpp>
namespace js = boost::json;

template <class T>
class Layer
{
public:
    Layer(size_t nCol, size_t nRow) : nCol(nCol), nRow(nRow)
    {
        elements.resize(nCol);
        for (size_t i = 0; i < nCol; ++i)
            elements[i].resize(nRow);
    }

    T& operator()(size_t i, size_t j)
    {
        return elements[i][j];
    }

    void setNCol(size_t v)
    {
        nCol = v;
        elements.resize(v);
        for (size_t i = 0; i < nCol; ++i)
            elements[i].resize(nRow);
    }

    void setNRow(size_t v)
    {
        nRow = v;
        for (size_t i = 0; i < elements.size(); ++i)
            elements[i].resize(v);
    }

protected:
    std::vector<std::vector<T>> elements;

    size_t nCol, nRow;
};

#endif // LAYER_H
