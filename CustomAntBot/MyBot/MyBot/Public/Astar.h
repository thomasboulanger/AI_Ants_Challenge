#ifndef ASTAR_H
#define ASTAR_H

#include <iostream>
#include <vector>
#include <queue>
#include <algorithm>
#include <vector>

#include "Location.h"
#include "State.h"

using namespace std;

struct Astar
{
    State* _state;
    Astar(State* state);
    int heuristic(Location u, Location v);
    int astar(Location start, Location end);
};
#endif // ASTAR_H
