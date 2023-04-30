#include "../Public/State.h"

using namespace std;

//constructor
State::State()
{
    gameover = 0;
    turn = 0;
    bug.open("./debug.txt");
};

//deconstructor
State::~State()
{
    bug.close();
};

//sets the state up
void State::setup()
{
    grid = vector<vector<Square>>(rows, vector<Square>(cols, Square()));
};

//resets all non-water squares to land and clears the bots ant vector
void State::reset()
{
    myAnts.clear();
    enemyAnts.clear();
    myHills.clear();
    enemyHills.clear();
    food.clear();
    for (int x = 0; x < rows; x++)
        for (int y = 0; y < cols; y++)
            if (!grid[x][y].isWater)
                grid[x][y].reset();
};

//outputs move information to the engine
void State::makeMove(const Location& loc, int direction)
{
    cout << "o " << loc.x << " " << loc.y << " " << CDIRECTIONS[direction] << endl;

    Location nLoc = getLocation(loc, direction);
    grid[nLoc.x][nLoc.y].ant = grid[loc.x][loc.y].ant;
    grid[loc.x][loc.y].ant = -1;
};

//returns the euclidean distance between two locations with the edges wrapped
double State::distance(const Location& loc1, const Location& loc2)
{
    int d1 = abs(loc1.x - loc2.x),
        d2 = abs(loc1.y - loc2.y),
        dr = min(d1, rows-d1),
        dc = min(d2, cols-d2);
    return sqrt(dr * dr + dc * dc);
};

//returns the new location from moving in a given direction with the edges wrapped
Location State::getLocation(const Location& loc, int direction)
{
    return Location((loc.x + DIRECTIONS[direction][0] + rows) % rows,
                    (loc.y + DIRECTIONS[direction][1] + cols) % cols);
};

int State::getDirection
(
    const Location& antLoc,
    const Location& destLoc,
    std::array<int, 2>& directions
)
{
    int ndirs = 0;

    // Check vertical direction
    if (antLoc.x > destLoc.x)
        // Move to the north
        directions[ndirs++] = 0;
    else if (antLoc.x < destLoc.x)
        // Move to the south
        directions[ndirs++] = 2;

    // Check horizontal direction
    if (antLoc.y < destLoc.y)
        // Move to the east
        directions[ndirs++] = 1;
    else if (antLoc.y > destLoc.y)
        // Move to the west
        directions[ndirs++] = 3;

    return ndirs;
}

bool State::isFree(const Location& loc)
{
    for (size_t antIdx = 0; antIdx < myAnts.size(); ++antIdx)
        if (grid[loc.x][loc.y].isWater || loc == myAnts[antIdx])
            return false;

    return true;
}


/*
    This function will update update the lastSeen value for any squares currently
    visible by one of your live ants.
    BE VERY CAREFUL IF YOU ARE GOING TO TRY AND MAKE THIS FUNCTION MORE EFFICIENT,
    THE OBVIOUS WAY OF TRYING TO IMPROVE IT BREAKS USING THE EUCLIDEAN METRIC, FOR
    A CORRECT MORE EFFICIENT IMPLEMENTATION, TAKE A LOOK AT THE GET_VISION FUNCTION
    IN ANTS.PY ON THE CONTESTS GITHUB PAGE.
*/
void State::updateVisionInformation()
{
    std::queue<Location> locQueue;
    Location sLoc, cLoc, nLoc;

    for (size_t antIdx = 0; antIdx < myAnts.size(); ++antIdx)
    {
        sLoc = myAnts[antIdx];
        locQueue.push(sLoc);

        vector<vector<bool>> visited(rows, vector<bool>(cols, 0));
        grid[sLoc.x][sLoc.y].isVisible = 1;
        visited[sLoc.x][sLoc.y] = 1;

        while (!locQueue.empty())
        {
            cLoc = locQueue.front();
            locQueue.pop();

            for (int d = 0; d < TDIRECTIONS; d++)
            {
                nLoc = getLocation(cLoc, d);

                if (!visited[nLoc.x][nLoc.y] && distance(sLoc, nLoc) <= viewradius)
                {
                    grid[nLoc.x][nLoc.y].isVisible = 1;
                    locQueue.push(nLoc);
                }
                visited[nLoc.x][nLoc.y] = 1;
            }
        }
    }
};

size_t State::manhattanDistance(const Location& loc1, const Location& loc2) const
{
    int d1 = abs(loc1.x-loc2.x);
    int d2 = abs(loc1.y-loc2.y);
    int dr = min(d1, rows-d1);
    int dc = min(d2, cols-d2);
    return dr + dc;
}

/*
    This is the output function for a state. It will add a char map
    representation of the state to the output stream passed to it.
    For example, you might call "cout << state << endl;"
*/
ostream& operator<<(ostream& os, const State& state)
{
    for (int x = 0; x < state.rows; x++)
    {
        for (int y = 0; y < state.cols; y++)
        {
            if (state.grid[x][y].isWater)
                os << '%';
            else if (state.grid[x][y].isFood)
                os << '*';
            else if (state.grid[x][y].isHill)
                os << (char)('A' + state.grid[x][y].hillPlayer);
            else if (state.grid[x][y].ant >= 0)
                os << (char)('a' + state.grid[x][y].ant);
            else if (state.grid[x][y].isVisible)
                os << '.';
            else
                os << '?';
        }
        os << endl;
    }

    return os;
};

//input function
istream& operator>>(istream& is, State& state)
{
    int x, y, player;
    string inputType, junk;

    //finds out which turn it is
    while (is >> inputType)
    {
        if (inputType == "end")
        {
            state.gameover = 1;
            break;
        }
        else if (inputType == "turn")
        {
            is >> state.turn;
            break;
        }
        else //unknown line
            getline(is, junk);
    }

    if (state.turn == 0)
    {
        //reads game parameters
        while (is >> inputType)
        {
            if (inputType == "loadtime")
                is >> state.loadtime;
            else if (inputType == "turntime")
                is >> state.turntime;
            else if (inputType == "rows")
                is >> state.rows;
            else if (inputType == "cols")
                is >> state.cols;
            else if (inputType == "turns")
                is >> state.turns;
            else if (inputType == "player_seed")
                is >> state.seed;
            else if (inputType == "viewradius2")
            {
                is >> state.viewradius;
                state.viewradius = sqrt(state.viewradius);
            }
            else if (inputType == "attackradius2")
            {
                is >> state.attackradius;
                state.attackradius = sqrt(state.attackradius);
            }
            else if (inputType == "spawnradius2")
            {
                is >> state.spawnradius;
                state.spawnradius = sqrt(state.spawnradius);
            }
            else if (inputType == "ready") //end of parameter input
            {
                state.timer.start();
                break;
            }
            else //unknown line
                getline(is, junk);
        }
    }
    else
    {
        //reads information about the current turn
        while (is >> inputType)
        {
            if (inputType == "w") //water square
            {
                is >> x >> y;
                state.grid[x][y].isWater = 1;
            }
            else if (inputType == "f") //food square
            {
                is >> x >> y;
                state.grid[x][y].isFood = 1;
                state.food.push_back(Location(x, y));
            }
            else if (inputType == "a") //live ant square
            {
                is >> x >> y >> player;
                state.grid[x][y].ant = player;
                if (player == 0)
                    state.myAnts.push_back(Location(x, y));
                else
                    state.enemyAnts.push_back(Location(x, y));
            }
            else if (inputType == "d") //dead ant square
            {
                is >> x >> y >> player;
                state.grid[x][y].deadAnts.push_back(player);
            }
            else if (inputType == "h")
            {
                is >> x >> y >> player;
                state.grid[x][y].isHill = 1;
                state.grid[x][y].hillPlayer = player;
                if (player == 0)
                    state.myHills.push_back(Location(x, y));
                else
                    state.enemyHills.push_back(Location(x, y));
            }
            else if (inputType == "players") //player information
                is >> state.noPlayers;
            else if (inputType == "scores") //score information
            {
                state.scores = vector<double>(state.noPlayers, 0.0);
                for (int p = 0; p < state.noPlayers; p++)
                    is >> state.scores[p];
            }
            else if (inputType == "go") //end of turn input
            {
                if (state.gameover)
                    is.setstate(ios::failbit);
                else
                    state.timer.start();
                break;
            }
            else //unknown line
                getline(is, junk);
        }
    }
    return is;
};
