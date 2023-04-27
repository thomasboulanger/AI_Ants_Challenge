#include "../Public/Bot.h"

//constructor
Bot::Bot()
{
};

//plays a single game of Ants.
void Bot::playGame()
{
    //reads the game parameters and sets up
    cin >> state;
    state.setup();
    endTurn();

    // All the locations are unseen at the beginning
    for (int x = 0; x < state.rows; ++x)
        for (int y = 0; y < state.cols; ++y)
            unseen.insert(Location(x, y));
    state.bug << "after the constructor " << unseen.size() << endl;

    //continues making moves while the game is not over
    while (cin >> state)
    {
        state.updateVisionInformation();
        doTurn();
        //makeMoves();
        endTurn();
    }
};

//makes the bots moves for the turn
void Bot::makeMoves()
{
    state.bug << "turn " << state.turn << ":" << endl;
    state.bug << state << endl;

    //picks out moves for each ant
    for (int ant = 0; ant < (int)state.myAnts.size(); ant++)
    {
        for (int dir = 0; dir < TDIRECTIONS; dir++)
        {
            Location antPos = state.getLocation(state.myAnts[ant], dir);

            if (!state.grid[antPos.x][antPos.y].isWater)
            {
                state.makeMove(state.myAnts[ant], dir);
                break;
            }
        }
    }

    state.bug << "time taken: " << state.timer.getTime() << "ms" << endl << endl;
};

bool Bot::doMoveDirection(const Location& antPos, int dir)
{
    Location newPos = state.getLocation(antPos, dir);
    if (state.isFree(newPos) && orders.find(newPos) == orders.end())
    {
        state.makeMove(antPos, dir);
        orders.insert(pair<Location, Location>(newPos, antPos));
        return true;
    }
    else
        return false;
};

bool Bot::doMoveLocation(const Location& antPos, const Location& destinationPos)
{
    array<int, 2> directions; // Initialization

    int ndirs = state.getDirection(antPos, destinationPos, directions);
    for (int i = 0; i < ndirs; ++i)
        if (doMoveDirection(antPos, directions[i]))
        {
            targets.insert(destinationPos, antPos);
            return true;
        }

    return false;
};

void Bot::doTurn()
{
    int distance = 0;
    const UINT nFood = state.food.size(), nMyAnts = state.myAnts.size();
    Route route;
    vector<Route> foodRoutes(nFood * nMyAnts, make_tuple(Location(), Location(), 0));
    vector<Location>::iterator hillIterator;
    map<Location, Location>::iterator ordersIterator;
    bool movingOut = false;

    orders.clear();
    targets.clear();

    // Prevent stepping on our own hills
    for (hillIterator = state.myHills.begin(); hillIterator < state.myHills.end(); ++hillIterator)
        orders.insert(pair<Location, Location>(*hillIterator, Location()));

    // Food gathering
    for (UINT foodIdx = 0; foodIdx < nFood; ++foodIdx)
        for (UINT antIdx = 0; antIdx < nMyAnts; ++antIdx)
        {
            distance = state.distance(state.myAnts[antIdx], state.food[foodIdx]);
            route = make_tuple(state.myAnts[antIdx], state.food[foodIdx], distance);
            foodRoutes[foodIdx * nMyAnts + antIdx] = route;
        }

    sort(foodRoutes.begin(), foodRoutes.end(), cmpRoutes);

#ifdef DEBUG
    state.bug << ">> " << nMyAnts*nFood << " routes computed!" << endl << endl;
    state.bug << ">> Sorted routes:" << endl;
    for ( uint routeIdx = 0 ; routeIdx < nFood*nMyAnts ; ++routeIdx )
        state.bug << ">>>> " << foodRoutes[ routeIdx ] << endl;
    state.bug << endl;
#endif

    vector<Route>::iterator routeIt;
    for (routeIt = foodRoutes.begin(); routeIt < foodRoutes.end(); ++routeIt)
    {
        if (!targets.containsKey(get<1>(*routeIt))) // food already queried?
        {
            if (!targets.containsValue(get<0>(*routeIt))) // ant already doing something?
                if (doMoveLocation(get<0>(*routeIt), get<1>(*routeIt)))
                {
#ifdef DEBUG
                    state.bug << ">> Added move from " << get<0>( *routeIt ) << " to " 
                                                       << get<1>( *routeIt ) << endl;
#endif
                }
        }
    }
    // Remove the locations that are visible
    state.bug << "there are " << unseen.size() << " unseen locations here" << endl;

    set<Location>::iterator unseenIt = unseen.begin();
    while (unseenIt != unseen.end())
    {
        if (state.grid[unseenIt->x][unseenIt->y].isVisible)
            unseen.erase(unseenIt++);
        else
            ++unseenIt;
    }

    state.bug << "there are " << unseen.size() << " unseen locations here" << endl;

    // Explore unseen areas
    vector<Route> unseenRoutes(unseen.size(), make_tuple(Location(), Location(), 0));
    for (UINT antIdx = 0; antIdx < state.myAnts.size(); ++antIdx)
    {
        // Check that we are not already moving the ant
        movingOut = false;
        for (ordersIterator = orders.begin(); ordersIterator != orders.end(); ++ordersIterator)
            if (ordersIterator->second == state.myAnts[antIdx])
            {
                movingOut = true;
                break;
            }

        if (!movingOut)
        {
            int i;
            for (i = 0, unseenIt = unseen.begin(); unseenIt != unseen.end(); ++unseenIt)
            {
                distance = state.distance(state.myAnts[antIdx], *unseenIt);
                route = make_tuple(state.myAnts[antIdx], *unseenIt, distance);
                unseenRoutes[i++] = route;
            }

            sort(unseenRoutes.begin(), unseenRoutes.end(), cmpRoutes);

            for (routeIt = unseenRoutes.begin(); routeIt != unseenRoutes.end(); ++routeIt)
                if (doMoveLocation(get<0>(*routeIt), get<1>(*routeIt))) break;
        }
    }

    // Move out from our hills
    for (hillIterator = state.myHills.begin(); hillIterator != state.myHills.end(); ++hillIterator)
    {
        vector<Location>::iterator it = find(state.myAnts.begin(), state.myAnts.end(), *hillIterator);
        if (it != state.myAnts.end())
        {
            // Check that we are not already moving the ant
            movingOut = false;
            for (ordersIterator = orders.begin(); ordersIterator != orders.end(); ++ordersIterator)
                if (ordersIterator->second == *hillIterator)
                {
                    movingOut = true;
                    break;
                }

            if (!movingOut)
                for (int d = 0; d < TDIRECTIONS; ++d)
                {
                    if (doMoveDirection(*hillIterator, d))
                        break;
                }
        }
    }
};

bool Bot::cmpRoutes(const Route& lhs, const Route& rhs)
{
    return get<2>(lhs) < get<2>(rhs);
}

ostream& operator<<(ostream& os, const Route& route)
{
    os << get<0>(route) << " " << get<1>(route) << " " << get<2>(route);
    return os;
};

//finishes the turn
void Bot::endTurn()
{
    if (state.turn > 0)
        state.reset();
    state.turn++;

    cout << "go" << endl;
};
