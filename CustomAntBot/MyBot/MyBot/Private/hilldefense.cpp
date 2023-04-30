#include "..\Public\hilldefense.h"

#include <algorithm>
#include <cassert>
#include <limits>

#include "..\Public\Bot.h"
#include "..\Public\zoc.h"

using namespace std;

static const size_t DefenseEnemyDistance = 20;
static const size_t DefensePanicRadius2 = 110;
static const size_t MinDefenders = 3;
static const size_t EquivalentDangerSlack = 4;

struct HillData {
	struct CandidateDefender {
		CandidateDefender(size_t idx, size_t d) : antidx(idx), dist(d) {}

		bool operator<(const CandidateDefender & o) const {return dist > o.dist;}

		size_t antidx;
		size_t dist;
	};

	Location pos;
	Map<size_t> dist;
	bool needupdate;
	size_t nrdefenders;
	vector<CandidateDefender> candidates;
	vector<size_t> defenders;

	HillData(State & state, const Location & p) :
		pos(p),
		dist(state.rows, state.cols),
		needupdate(true)
	{
		dist.fill(0);
	}
};

struct HillDefense::Data {
	vector<HillData *> hills;
	size_t lastupdated;
	bool hilldestroyed;

	vector<Location> updatequeue;

	Data() :
		lastupdated(0),
		hilldestroyed(false)
	{
	}

	~Data()
	{
		while (!hills.empty()) {
			delete hills.back();
			hills.pop_back();
		}
	}
};

HillDefense::HillDefense(Bot & b) :
	bot(b),
	d(*new Data),
	state(b.state)
{
}

HillDefense::~HillDefense()
{
	delete &d;
}

void HillDefense::init()
{
}

void HillDefense::update_hill_distances(size_t hillidx)
{
	HillData * hd = d.hills[hillidx];

	state.bug << "Update distance map for hill " << hillidx << " at " << hd->pos << endl;

	hd->dist.fill(numeric_limits<size_t>::max());

	d.updatequeue.clear();

	hd->dist[hd->pos] = 0;
	d.updatequeue.push_back(hd->pos);

	size_t queue_head = 0;
	while (queue_head < d.updatequeue.size()) {
		Location cur = d.updatequeue[queue_head++];
		size_t dist = hd->dist[cur];

		const int * dirperm = getdirperm();
		for (int predir = 0; predir < TDIRECTIONS; ++predir) {
			int dir = dirperm[predir];
			Location n = state.getLocation(cur, dir);

			if (state.grid[n.row][n.col].isWater)
				continue;

			if (dist + 1 < hd->dist[n]) {
				hd->dist[n] = dist + 1;
				d.updatequeue.push_back(n);
			}
		}
	}
}

void HillDefense::update_hills()
{
	d.hilldestroyed = false;

	if (state.newwater) {
		for (size_t hillidx = 0; hillidx < d.hills.size(); ++hillidx)
			d.hills[hillidx]->needupdate = true;
	}

	// check if hills are still present
	for (size_t hillidx = 0; hillidx < d.hills.size(); ++hillidx) {
		HillData * hd = d.hills[hillidx];
		Square & sq = state.grid[hd->pos.row][hd->pos.col];

		if (sq.isVisible && sq.hillPlayer != 0) {
			state.bug << "Lost hill at " << hd->pos << endl;
			delete hd;

			d.hills[hillidx] = d.hills.back();
			d.hills.pop_back();
			hillidx--;
			d.hilldestroyed = true;
		}
	}

	// check if new hills are known now
	for (size_t statehillidx = 0; statehillidx < state.myHills.size(); ++statehillidx) {
		Location pos = state.myHills[statehillidx];

		for (size_t hillidx = 0; hillidx < d.hills.size(); ++hillidx) {
			if (d.hills[hillidx]->pos == pos)
				goto known;
		}

		state.bug << "Found new hill at " << pos << endl;
		d.hills.push_back(new HillData(state, pos));

	known: ;
	}

	if (!d.hills.size())
		return;

	// update the distance map of one hill
	if (d.lastupdated >= d.hills.size())
		d.lastupdated = 0;
	size_t hillidx = d.lastupdated;
	do {
		hillidx++;
		if (hillidx >= d.hills.size())
			hillidx = 0;

		if (d.hills[hillidx]->needupdate) {
			update_hill_distances(hillidx);
			d.lastupdated = hillidx;
			break;
		}
	} while (hillidx != d.lastupdated);
}

void HillDefense::run()
{
	state.bug << "Defense turn " << state.turn << endl;

	update_hills();

	//
	vector<HillData *> defense;

	for (size_t hillidx = 0; hillidx < d.hills.size(); ++hillidx) {
		HillData * hd = d.hills[hillidx];
		if (bot.m_zoc.m_enemy[hd->pos] >= DefenseEnemyDistance)
			continue;

		hd->nrdefenders = MinDefenders;
		hd->candidates.clear();
		hd->defenders.clear();

		for (size_t enemyantidx = 0; enemyantidx < state.enemyAnts.size(); ++enemyantidx) {
			if (state.eucliddist2(state.enemyAnts[enemyantidx], hd->pos) <= DefensePanicRadius2)
				hd->nrdefenders++;
		}

		for (size_t antidx = 0; antidx < bot.m_ants.size(); ++antidx) {
			Ant & ant = bot.m_ants[antidx];
			hd->candidates.push_back(HillData::CandidateDefender(antidx, hd->dist[ant.where]));
		}
		sort(hd->candidates.begin(), hd->candidates.end());

		state.bug << "  defense at " << hd->pos << " requires " << hd->nrdefenders << " defenders" << endl;
		defense.push_back(hd);
	}

	//
	vector<bool> claimed;
	claimed.insert(claimed.begin(), bot.m_ants.size(), false);

	while (!defense.empty()) {
		int besthillidx = -1;
		int bestantidx = -1;
		size_t bestdistance = numeric_limits<size_t>::max();

		size_t p = getprime();
		size_t ofs = fastrng() % defense.size();
		for (size_t prehillidx = 0; prehillidx < defense.size(); ++prehillidx) {
			size_t hillidx = (p * prehillidx + ofs) % defense.size();
			HillData * hd = defense[hillidx];

			while (!hd->candidates.empty()) {
				size_t antidx = hd->candidates.back().antidx;
				size_t dist = hd->candidates.back().dist;

//				state.bug << "  hill at " << hd->pos << " candidate ant " << antidx
//					<< " at " << bot.m_ants[antidx].where << " dist " << dist << endl;

				if (claimed[antidx]) {
//					state.bug << "    already claimed" << endl;
					hd->candidates.pop_back();
					continue;
				}

				if (dist < bestdistance) {
//					state.bug << "   best so far" << endl;
					bestdistance = dist;
					bestantidx = antidx;
					besthillidx = hillidx;
				}
				break;
			}
		}

		if (besthillidx < 0)
			break;

		HillData * hd = defense[besthillidx];

		hd->nrdefenders--;
		if (hd->nrdefenders == 0) {
			defense[besthillidx] = defense.back();
			defense.pop_back();
		}

		claimed[bestantidx] = true;

		//
		size_t hilldanger = bot.m_zoc.m_enemy[hd->pos];
		Ant & ant = bot.m_ants[bestantidx];

		state.bug << "  assign ant " << bestantidx << " at " << ant.where << " to defend " << hd->pos << endl;

		if (ant.assigneddirection)
			continue;

		size_t equivdanger = bot.m_zoc.m_enemy[ant.where] + hd->dist[ant.where];
		if (equivdanger <= hilldanger + EquivalentDangerSlack) {
			state.bug << "    equivalent hilldanger " << hilldanger << " is low enough" << endl;
			continue;
		}

		int bestdirection = -1;
		size_t bestequivdanger = equivdanger;
		const int * dirperm = getdirperm();
		for (int predir = 0; predir < TDIRECTIONS; ++predir) {
			int dir = dirperm[predir];
			Location n = state.getLocation(ant.where, dir);
			size_t newequivdanger = bot.m_zoc.m_enemy[n] + hd->dist[n];
			if (newequivdanger < bestequivdanger) {
				bestequivdanger = newequivdanger;
				bestdirection = dir;
			}
		}

		if (bestdirection == -1) {
			size_t bestdist = hd->dist[ant.where];
			for (int predir = 0; predir < TDIRECTIONS; ++predir) {
				int dir = dirperm[predir];
				Location n = state.getLocation(ant.where, dir);
				size_t newdist = hd->dist[n];
				if (newdist < bestdist) {
					bestdist = newdist;
					bestdirection = dir;
				}
			}
		}

		//assert(bestdirection != -1);
		state.bug << "    defense move: " << cdir(bestdirection) << endl;

		ant.direction = bestdirection;
		ant.assigneddirection = true;
	}
}

size_t HillDefense::getnrhills()
{
	return d.hills.size();
}

const Location & HillDefense::gethill(size_t idx)
{
	//assert(idx < d.hills.size());
	return d.hills[idx]->pos;
}

bool HillDefense::hilldestroyed()
{
	return d.hilldestroyed;
}
