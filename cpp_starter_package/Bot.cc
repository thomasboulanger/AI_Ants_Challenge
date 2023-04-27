#include "Bot.h"

using namespace std;

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

	//continues making moves while the game is not over
	while (cin >> state)
	{
		state.updateVisionInformation();
		makeMoves();
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

//finishes the turn
void Bot::endTurn()
{
	if (state.turn > 0)
		state.reset();
	state.turn++;

	cout << "go" << endl;
};
