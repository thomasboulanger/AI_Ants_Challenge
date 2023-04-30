#ifndef ZOC_H
#define ZOC_H

#include "map.h"

struct State;

/**
 * Zones of control: determine field reachable by the enemy and us.
 */
struct Zoc {
	Zoc(State & state_);
	~Zoc();

	void init();
	void update();

	struct Data;

	State & state;
	Data & d;
	Map<size_t> m_me;
	Map<size_t> m_enemy;
};

#endif //ZOC_H
