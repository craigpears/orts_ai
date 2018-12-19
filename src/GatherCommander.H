#ifndef GatherCommander_H
#define GatherCommander_H

#include "Commander.H"
#include "Game.H"

class GatherCommander: public Commander
{
	public:
	void ComputeActions(const Game& game);
};

#endif
