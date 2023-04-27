#ifndef LOCATION_H_
#define LOCATION_H_

/*
    struct for representing locations in the grid.
*/
struct Location
{
    int x, y;

    Location()
    {
        x = y = 0;
    };

    Location(int r, int c)
    {
        x = r;
        y = c;
    };
};

#endif //LOCATION_H_
