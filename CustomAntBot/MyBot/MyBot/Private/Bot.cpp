#include "../Public/Bot.h"

//constructor
Bot::Bot()
    : astarObj(&state)
{
}

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
        endTurn();
    }
}

bool Bot::doMoveDirection(const Location& antPos, int dir)
{
    Location newPos = state.getLocation(antPos, dir);
    if (state.isFree(newPos) && orders.find(newPos) == orders.end())
    {
        state.makeMove(antPos, dir);
        orders.insert(pair<Location, Location>(newPos, antPos));
        return true;
    }
    return false;
}

bool Bot::doMoveLocation(const Location& antPos, const Location& destinationPos)
{
    array<int, 2> directions{}; // Initialization

    int ndirs = state.getDirection(antPos, destinationPos, directions);
    for (int i = 0; i < ndirs; ++i)
        if (doMoveDirection(antPos, directions[i]))
        {
            targets.insert(destinationPos, antPos);
            return true;
        }

    return false;
}

//game logic for each ant on each turn
void Bot::doTurn()
{
    //variable initialization
    int distance = 0;
    const size_t nFood = state.food.size(), nMyAnts = state.myAnts.size();
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
    for (size_t foodIdx = 0; foodIdx < nFood; ++foodIdx)
        for (size_t antIdx = 0; antIdx < nMyAnts; ++antIdx)
        {
            distance = state.distance(state.myAnts[antIdx], state.food[foodIdx]);
            route = make_tuple(state.myAnts[antIdx], state.food[foodIdx], distance);
            foodRoutes[foodIdx * nMyAnts + antIdx] = route;
        }

    sort(foodRoutes.begin(), foodRoutes.end(), cmpRoutes);

#ifdef DEBUG
    state.bug << ">> " << nMyAnts * nFood << " routes computed!" << endl << endl;
    state.bug << ">> Sorted routes:" << endl;
    for (size_t routeIdx = 0; routeIdx < nFood * nMyAnts; ++routeIdx)
        state.bug << ">>>> " << foodRoutes[routeIdx] << endl;
    state.bug << endl;
#endif

    vector<Route>::iterator routeIterator;
    state.bug << "COMPUTE ASTAR " << state.timer.getTime() << endl;
    for (routeIterator = foodRoutes.begin(); routeIterator < foodRoutes.end(); ++routeIterator)
    {
        if (!targets.containsKey(get<1>(*routeIterator))) // food already queried?
        {
            if (!targets.containsValue(get<0>(*routeIterator))) // ant already doing something?
            {
                if (state.timer.getTime() > 550)
                    break;
                int shortestPathDirection = astarObj.astar(get<0>(*routeIterator), get<1>(*routeIterator));
                if (shortestPathDirection >= 0) doMoveDirection(get<0>(*routeIterator), shortestPathDirection);

#ifdef DEBUG
                state.bug << ">> Added move from " << get<0>(*routeIterator) << " to "
                    << get<1>(*routeIterator) << endl;
#endif
            }
        }
    }

    state.bug << "COMPUTE ASTAR END " << state.timer.getTime() << endl;

    // Remove the locations that are visible
    state.bug << "there are " << unseen.size() << " unseen locations here" << endl;

    set<Location>::iterator unseenIterator = unseen.begin();
    while (unseenIterator != unseen.end())
    {
        if (state.grid[unseenIterator->x][unseenIterator->y].isVisible)
            unseen.erase(unseenIterator++);
        else
            ++unseenIterator;
    }

    state.bug << "there are " << unseen.size() << " unseen locations here" << endl;

    // Explore unseen areas
    vector<Route> unseenRoutes(unseen.size(), make_tuple(Location(), Location(), 0));
    for (size_t antIdx = 0; antIdx < state.myAnts.size(); ++antIdx)
    {
        if (state.timer.getTime() > 650)
            break;
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
            for (i = 0, unseenIterator = unseen.begin(); unseenIterator != unseen.end(); ++unseenIterator)
            {
                distance = state.distance(state.myAnts[antIdx], *unseenIterator);
                route = make_tuple(state.myAnts[antIdx], *unseenIterator, distance);
                unseenRoutes[i++] = route;
            }

            sort(unseenRoutes.begin(), unseenRoutes.end(), cmpRoutes);

            for (routeIterator = unseenRoutes.begin(); routeIterator != unseenRoutes.end(); ++routeIterator)
                if (doMoveLocation(get<0>(*routeIterator), get<1>(*routeIterator))) break;
        }
    }

    // Move out from our hills
    for (hillIterator = state.myHills.begin(); hillIterator != state.myHills.end(); ++hillIterator)
    {
        vector<Location>::iterator it = find(state.myAnts.begin(), state.myAnts.end(), *hillIterator);
        if (it != state.myAnts.end())
        {
            if (state.timer.getTime() > 950)
                break;
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
}

//finishes the turn
void Bot::endTurn()
{
    if (state.turn > 0)
        state.reset();
    state.turn++;

    cout << "go" << endl;
}
