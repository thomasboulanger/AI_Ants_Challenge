#include "../Public/Astar.h"

#include <map>

//constructor
Astar::Astar(State* state)
{
    _state = state;
}

//get the heuristic value
int Astar::heuristic(Location u, Location v)
{
    //manhattan distance
    int d1 = abs(u.x - v.x),
        d2 = abs(u.y - v.y),
        dx = min(d1, _state->rows - d1),
        dy = min(d2, _state->cols - d2);
    return dx + dy;
}

//A* function that return the direction from the current position to move to the end position
int Astar::astar(Location start, Location end)
{
    // _state->bug << "AStar " << start.x << ":" << start.y << " -> " << end.x << ":" << end.y << endl;
    
    map<Location, int> scoreByDistance;
    map<Location, int> scoreByHeuristic;
    map<Location, int> directionToReachLocation;

    auto sortFunction = [&](Location a, Location b)
    {
        return scoreByHeuristic[a] > scoreByHeuristic[b];
    };

    priority_queue<Location, vector<Location>, decltype(sortFunction)> priorityQ(sortFunction);

    //initialize the first value (start position)
    priorityQ.push(start);
    scoreByDistance[start] = 0;
    scoreByHeuristic[start] = scoreByDistance[start] + heuristic(start, end);

    while (!priorityQ.empty())
    {
        Location current = priorityQ.top();
        priorityQ.pop();

        // _state->bug << "Pop " << current.x << ":" << current.y << endl;

        //end of the path
        if (current == end)
        {
            // _state->bug << "Path found" << endl;
            int currentDir = -2;
            
            while (current != start)
            {
                currentDir = (directionToReachLocation[current] + TDIRECTIONS/2) % TDIRECTIONS; //(dir + 2) % 4 work as well
                current = _state->getLocation(current, currentDir);

                // _state->bug << "Go back to " << current.x << ":" << current.y << " using " << CDIRECTIONS[currentDir] << endl;
            }

            // _state->bug << "Path finished" << endl;
            if (currentDir == -2) return -2;
            return (currentDir + TDIRECTIONS / 2) % TDIRECTIONS;
        }

        //increment the cost (each cells count as 1)
        int nextPosScoreByDistance = scoreByDistance[current] + 1;

        if (nextPosScoreByDistance > 16)
            return -1;

        //iterate on each direction to add the bests value to the priority queue
        for (int dir = 0; dir < TDIRECTIONS; dir++)
        {
            Location nextPos = _state->getLocation(current, dir);
            // _state->bug << "Push " << current.x << ":" << current.y << " -> " << nextPos.x << ":" << nextPos.y << " " << CDIRECTIONS[dir] << endl;
            
            if (_state->grid[nextPos.x][nextPos.y].isWater)
                continue;
            
            if ((scoreByDistance.count(nextPos) > 0) && (nextPosScoreByDistance >= scoreByDistance[nextPos]))
                continue;
            
            scoreByDistance[nextPos] = nextPosScoreByDistance;
            scoreByHeuristic[nextPos] = scoreByDistance[nextPos] + heuristic(nextPos, end);
            directionToReachLocation[nextPos] = dir;
            
            // _state->bug << "SECOND " << current.x << ":" << current.y << " -> " << nextPos.x << ":" << nextPos.y << " " << CDIRECTIONS[directionToReachLocation[nextPos]] << nextPosScoreByDistance << endl;

            priorityQ.push(nextPos);
        }
    }

    return -1;
}
