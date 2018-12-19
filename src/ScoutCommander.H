#ifndef ScoutCommander_H
#define ScoutCommander_H

#include "Commander.H"

class ScoutCommander: public Commander
{
	public:	
	ScoutCommander();
	void ComputeActions();
	void InitLocations();
	private:
	Vector<Loc> exploreTargets;
	bool exploredAllLocs;
};

#endif
