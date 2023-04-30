#ifndef MAP_H
#define MAP_H

#include "State.h"

using namespace  std;
template<typename T>
struct Map {
    typedef T Type;
    typedef vector<Type> Vector;

    Map(size_t r = 0, size_t c = 0)
    {
        resize(r, c);
    }

    void resize(size_t r, size_t c)
    {
        rows = r;
        cols = c;
        m_data.resize(rows * cols);
    }

    void fill(const Type & t)
    {
        m_data.assign(rows * cols, t);
    }

    typename Vector::reference operator[](const Location & pos)
    {
        return m_data[pos.x * cols + pos.y];
    }

    typename Vector::const_reference operator[](const Location & pos) const
    {
        return m_data[pos.x * cols + pos.y];
    }

    size_t rows;
    size_t cols;
    vector<Type> m_data;
};


#endif // MAP_H