#ifndef LOCATION_H_
#define LOCATION_H_

#include <ostream>

using namespace std;

//struct for representing locations in the grid.
struct Location
{
    int x, y;

    Location()
    {
        x = y = 0;
    }

    Location(int r, int c)
    {
        x = r;
        y = c;
    }

    bool operator==(const Location& other) const
    {
        return (x == other.x && y == other.y);
    }

    bool operator!=(const Location& other) const
    {
        return (x != other.x || y != other.y);
    }
    
    bool operator<(const Location& other) const
    {
        if (x == other.x)
            return y < other.y;

        return x < other.x;
    }
};

ostream& operator<<(ostream& os, const Location& loc);

#endif //LOCATION_H_
