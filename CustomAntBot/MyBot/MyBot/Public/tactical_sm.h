#ifndef TACTICAL_SM_H
#define TACTICAL_SM_H

#include <stdlib.h>
#include <vector>

#include "tactical_smbase.h"

struct Bot;
struct Location;
struct PlayerMove;
template<typename T>
struct BaseSubmap;
struct Submap;
struct State;

struct TacticalSm : TacticalSmBase {
	struct Data;
	struct ShadowAnt;
	struct Theater;

	TacticalSm(Bot & bot_);
	~TacticalSm();

	void init();
	void run();
	void learn();

	bool timeover();
	void generate_theater(const Location & center);
	void run_theater(size_t theateridx);
	void push_moves(size_t theateridx, size_t myidx);
	void pull_moves(size_t theateridx);
	int pull_enemy_moves(size_t theateridx);

	void evaluate_moves(Theater & th, PlayerMove & pm, PlayerMove & enemymove, float & myvalue, float & enemyvalue);
	void evaluate_new_moves(size_t theateridx);
	bool get_improve_pair(const std::vector<PlayerMove *> & moves, size_t& myidx, size_t& enemyidx);
	void improve(size_t theateridx, size_t myidx, size_t enemyidx);

	void choose_strategy(size_t theateridx);
	size_t choose_max_min_move(size_t theateridx);
	size_t choose_max_avg_move(size_t theateridx);

	Data & d;
};

#endif // TACTICAL_SM_H
