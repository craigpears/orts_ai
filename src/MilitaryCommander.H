#ifndef MilitaryCommander_H
#define MilitaryCommander_H

#include "Commander.H"
#include "Game.H"
#include "GameObj.H"
#include "GameRepresentation.H"
#include "Structures.H"

class MilitaryCommander: public Commander
{
	public:	
	void ComputeActions(uint4 _gameTick);	
	private:
	void ComputeAction(Unit* unit, uint4 range);
	bool IsCloseToBase(Loc unitLoc);
	bool TargetUnit(Unit* unit, uint4 range);
	void MoveToObjective(Unit* unit);
	bool PursueClosestUnit(Unit* unit, bool defendingBase);
	bool PursueClosestWorker(Unit* unit);
	bool ReturnToBarracks(Unit* unit);
	bool PursueClosestBase(Unit* unit);
	bool AttackUnits(Unit* unit, uint4 range, bool defendingBase);
	bool AttackWorkers(Unit* unit, uint4 range);
	bool AttackBases(Unit* unit, uint4 ranged);
	bool AttackObject(Unit* unit, uint4 targetID, GameObj* targetGob, uint4 enemyUnitsRange);
	uint4 GetNextObjective(uint4 currentObjective, bool targetting);
	uint4 gameTick;
	
};

#endif
