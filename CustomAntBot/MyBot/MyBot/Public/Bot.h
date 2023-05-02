#ifndef BOT_H_
#define BOT_H_

#include <map>
#include <set>
#include <array>
#include <algorithm>

#include "Bimap.h"
#include "State.h"
#include "Location.h"
#include "Astar.h"

using namespace std;
typedef tuple< Location, Location, double > Route;


//This Script represents your bot in the game of Ants
struct Bot
{
    State state;
    map< Location, Location > orders;
    set< Location > unseen;
    Bimap< Location, Location > targets;
    Astar astarObj;

    Bot();

    void playGame();    //plays a single game of Ants
    void makeMoves();   //makes moves for a single turn

    bool doMoveDirection(const Location & antPos, int direction);   //move from an initial position with a direction
    bool doMoveLocation(const Location & antPos, const Location & destinationPos); //move an ant from an initial position to another position
    void doTurn();  //all the logic that drive some pixel to pretend to be other things, like ants ? 
    static bool cmpRoutes(const Route & lhs, const Route & rhs);
    
    void endTurn();     //indicates to the engine that it has made its moves
};
ostream & operator<<(ostream & os, const Route & route);

#endif //BOT_H_
