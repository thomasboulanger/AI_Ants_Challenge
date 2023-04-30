#ifndef HILLDEFENSE_H
#define HILLDEFENSE_H

#include <cstdlib>

struct Bot;
struct Location;
struct State;

struct HillDefense {
	HillDefense(Bot & b);
	~HillDefense();

	void init();
	void run();

	bool hilldestroyed();
	size_t getnrhills();
	const Location & gethill(size_t idx);

	void update_hills();
	void update_hill_distances(size_t hillidx);

	struct Data;

	Bot & bot;
	Data & d;
	State & state;
};

#endif // HILLDEFENSE_H
