#ifndef LAYER_H
#define LAYER_H

#include <vector>
#include <memory>

template <class T>
class Layer
{
public:
    Layer(int nCol, int nRow) : nCol(nCol), nRow(nRow)
    {
        elements.resize(nCol);
        for (int i = 0; i < nCol; ++i)
            elements[i].resize(nRow);
    }

    std::unique_ptr<T>& operator()(int i, int j)
    {
        return elements[i][j];
    }

protected:
    std::vector<std::vector<std::unique_ptr<T>>> elements;

    int nCol, nRow;
};

#endif // LAYER_H
