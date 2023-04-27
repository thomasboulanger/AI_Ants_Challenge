#ifndef STATE_H_
#define STATE_H_

#include <iostream>
#include <stdio.h>
#include <cstdlib>
#include <cmath>
#include <array>
#include <string>
#include <vector>
#include <queue>
#include <stack>
#include <stdint.h>

#include "Timer.h"
#include "Bug.h"
#include "Square.h"
#include "Location.h"

using namespace std;
/*
    constants
*/
const int TDIRECTIONS = 4;
const char CDIRECTIONS[4] = {'N', 'E', 'S', 'W'};
const int DIRECTIONS[4][2] = {{-1, 0}, {0, 1}, {1, 0}, {0, -1}}; //{N, E, S, W}

/*
    struct to store current state information
*/
struct State
{
    /*
        Variables
    */
    int rows, cols,
        turn, turns,
        noPlayers;
    double attackradius, spawnradius, viewradius;
    double loadtime, turntime;
    vector<double> scores;
    bool gameover;
    int64_t seed;

    vector<vector<Square>> grid;
    vector<Location> myAnts, enemyAnts, myHills, enemyHills, food;

    Timer timer;
    Bug bug;

    /*
        Functions
    */
    State();
    ~State();

    void setup();
    void reset();

    void makeMove(const Location& loc, int direction);
    double distance(const Location& loc1, const Location& loc2);
    Location getLocation(const Location& startLoc, int direction);

    bool isFree(const Location& loc);
    int getDirection(const Location & antLoc,const Location & destLoc,array< int, 2 > & directions);

    void updateVisionInformation();
};

ostream& operator<<(ostream& os, const State& state);
istream& operator>>(istream& is, State& state);

#endif //STATE_H_
