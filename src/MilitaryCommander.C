#include "Blackboard.H"
#include "Constants.H"
#include "GameRepresentation.H"
#include "MilitaryCommander.H"

using namespace std;

void MilitaryCommander::ComputeActions(uint4 _gameTick)
{
	gameTick = _gameTick;
	Vector<Marine>* marines = gameRep->GetMarines();
	Vector<Marine>::iterator it = marines->begin();
	
	while(it != marines->end())
	{
		ComputeAction(&(*it),64);
		it++;
	}
	
	Vector<Tank>* tanks = gameRep->GetTanks();
	Vector<Tank>::iterator it2 = tanks->begin();
	while(it2 != tanks->end())
	{
		ComputeAction(&(*it2),112);
		it2++;
	}
}

void MilitaryCommander::ComputeAction(Unit* unit, uint4 range)
{		
	//Check to see if the unit is stuck
	if(unit->isStuck && *(unit->gameObj)->sod.speed == 0)
	MoveRandomly(unit);
	else
	{
		unit->isStuck = *(unit->gameObj)->sod.speed == 0;
		//Check to see if the unit can fire on an objective target
		bool attacked = TargetUnit(unit, range);
		//If it hasn't, then move towards an objective target
		if(!attacked)
		MoveToObjective(unit);			
	}	
}

bool MilitaryCommander::IsCloseToBase(Loc unitLoc)
{	
	Vector<Base>* myBases = gameRep->GetMyBases();
	Vector<Base>::iterator it = myBases->begin();
	Loc baseLoc;
	while(it != myBases->end())
	{
		baseLoc.Set(it->x,it->y);
		uint4 manhattanDistance = abs((sint4)unitLoc.x - (sint4)baseLoc.x) + abs((sint4)unitLoc.y - (sint4)baseLoc.y);
		if(manhattanDistance < 10 * 16)
		{
			//cout << "Unit at " << unitLoc.x << ", " << unitLoc.y << " is close to the base at " << baseLoc.x << ", " << baseLoc.y << endl;
			return true;
		}
		it++;
	}
	
	return false;
}

/*
	This function is responsible for attacking any nearby threatening units or to retreat from them if the unit can't fire if they are a threat
	Returning true stops the unit from moving towards its objective, returning false causes the unit to continue moving towards its objective
	If the unit fails to fire upon a target of the type indicated by the objective, then it should return false to allow such a target to come within range
*/
bool MilitaryCommander::TargetUnit(Unit* unit, uint4 range)
{
	
	uint4 objective = Blackboard::UNIT_MODE;
	bool returnType = true;
	uint4 objectivesChecked = 0;
	/*
		Target priorities are in the order: Units(Tanks->Marines)->Workers->Bases->Units
		Priorities will go in that order until either a target is found or all types have been checked
	*/
	
	while(objectivesChecked < Constants::NUMBER_OF_OBJECTIVES)//One of the objectives is attack all which the others will never enter
	{
		switch(objective)
		{
			case Constants::MODE_DEFEND:
				if(AttackUnits(unit, range, true))
				return returnType;
				break;
			case Constants::MODE_ATTACK_UNITS:
				if(AttackUnits(unit, range, false))
				return returnType;
				break;
			case Constants::MODE_ATTACK_WORKERS:
				if(AttackWorkers(unit, range))
				return returnType;
				break;
			case Constants::MODE_ATTACK_BASES:
				if(AttackBases(unit, range))
				return returnType;
				break;
			case Constants::MODE_ATTACK_ALL:
				if(AttackUnits(unit, range, false))
				return true;
				else if(AttackWorkers(unit, range))
				return true;
				else if(AttackBases(unit, range))
				return true;
				else return false;
		}
		
		returnType = false;
		objectivesChecked++;	
		objective = GetNextObjective(objective, true);	
	}
	
	return false;
}

void MilitaryCommander::MoveToObjective(Unit* unit)
{
	uint4 objective = Blackboard::UNIT_MODE;
	uint4 objectivesChecked = 0;
	bool inPursuit;
	
	//The only time that this should fail to do an action of some form is if it is defending or there is a hidden enemy building
	while(objectivesChecked < 4 && objective != Constants::STOP)
	{
		switch(objective)
		{
			case Constants::MODE_DEFEND:
				/*
					When a unit is defending its first priority is to hunt down any enemy units that are within 20 tiles of the control center
					If it can't find an enemy to attack then it will first make sure that it is within ten tiles of a mining worker, then it will group up with the closest
					unit that is also near a miner.
				*/
				inPursuit = PursueClosestUnit(unit, true);
				if(!inPursuit)
				{
					bool movingToWorkers = ReturnToBarracks(unit);
					if(!movingToWorkers)
					unit->isStuck = false;
				}
				return;
			case Constants::MODE_ATTACK_UNITS:
				inPursuit = PursueClosestUnit(unit, false);
				if(inPursuit)
				return;
				break;
			case Constants::MODE_ATTACK_WORKERS:
				inPursuit = PursueClosestWorker(unit);
				if(inPursuit)
				return;
			case Constants::MODE_ATTACK_BASES:
				inPursuit = PursueClosestBase(unit);
				if(inPursuit)
				return;
				break;
			case Constants::MODE_ATTACK_ALL:
				inPursuit = PursueClosestUnit(unit, false);
				if(!inPursuit)
				inPursuit = PursueClosestWorker(unit);
				if(!inPursuit)
				inPursuit = PursueClosestBase(unit);
				return;				
		}
		
		objectivesChecked++;
		objective = GetNextObjective(objective, false);
	}
	unit->isStuck = false;
	return;
}

bool MilitaryCommander::ReturnToBarracks(Unit* unit)
{
	//Find the closest worker and move close to it
	GameObj* barracks = gameRep->GetBarracks();
	Loc targetLoc(*barracks->sod.x,*barracks->sod.y);
	return MoveToLocation(unit, targetLoc, Constants::PATHFINDING_GET_CLOSE);
	
	
}

bool MilitaryCommander::PursueClosestUnit(Unit* unit, bool defendingBase)
{
	Vector<Marine>* enemyMarines = gameRep->GetEnemyMarines();
	Vector<Tank>* enemyTanks = gameRep->GetEnemyTanks();
		
	//Quick rejection
	if(enemyTanks->empty() && enemyMarines->empty())
	return false;
	
	uint4 distanceToClosestTarget = 10000;
	GameObj* gob = unit->gameObj;
	bool foundATarget = false;
	uint4 targetID;
	Loc targetLoc(0,0);
	
	//Check if there is a tank nearby to pursue
	Vector<Tank>::iterator it = enemyTanks->begin();
	while(it != enemyTanks->end())
	{
		GameObj* gob2 = it->gameObj;
		double distanceFromUnit = MiscFunctions::distance(*gob->sod.x,*gob->sod.y,*gob2->sod.x,*gob2->sod.y);
		bool closeToBase = IsCloseToBase(Loc(*gob->sod.x,*gob->sod.y));
		if(distanceFromUnit < distanceToClosestTarget && (closeToBase || !defendingBase))
		{
			foundATarget = true;
			targetID = it->unitID;
			distanceToClosestTarget = distanceFromUnit;
			targetLoc.Set(*gob2->sod.x,*gob2->sod.y);
		}
		it++;
	}
	
	//Check if there is a marine nearby to pursue
	Vector<Marine>::iterator it2 = enemyMarines->begin();
	while(it2 != enemyMarines->end())
	{
		GameObj* gob2 = it2->gameObj;
		double distanceFromUnit = MiscFunctions::distance(*gob->sod.x,*gob->sod.y,*gob2->sod.x,*gob2->sod.y);
		bool closeToBase = IsCloseToBase(Loc(*gob->sod.x,*gob->sod.y));
		if(distanceFromUnit < distanceToClosestTarget && (closeToBase || !defendingBase))
		{
			foundATarget = true;
			targetID = it2->unitID;
			distanceToClosestTarget = distanceFromUnit;
			targetLoc.Set(*gob2->sod.x,*gob2->sod.y);
		}
		it2++;
	}
	
	if(foundATarget)
	{
		if(unit->targetID != targetID)
		{
			unit->targetID = targetID;
			unit->deviated = true;
		}
		MoveToLocation(unit, targetLoc, Constants::PATHFINDING_GET_CLOSE);
		return true;
	}
	else
	return false;	
}

bool MilitaryCommander::PursueClosestWorker(Unit* unit)
{	
	GameObj* gob = unit->gameObj; 	
	Vector<Worker>* enemyWorkers = gameRep->GetEnemyWorkers();
	//Quick reject
	if(enemyWorkers->empty())
	return false;
	
	uint4 distanceToClosestTarget = 10000;
	bool foundATarget = false;
	Loc targetLoc(0,0);
	uint4 targetID;
	Vector<Worker>::iterator it = enemyWorkers->begin();
	
	while(it != enemyWorkers->end())
	{
		GameObj* gob2 = it->gameObj;
		double distanceFromUnit = MiscFunctions::distance(*gob->sod.x,*gob->sod.y,*gob2->sod.x,*gob2->sod.y);
		if(distanceFromUnit < distanceToClosestTarget)
		{
			foundATarget = true;
			targetID = it->unitID;
			distanceToClosestTarget = distanceFromUnit;
			targetLoc.Set(*gob2->sod.x,*gob2->sod.y);
		}
		it++;
	}
	
	if(foundATarget)
	{
		if(unit->targetID != targetID)
		{
			unit->targetID = targetID;
			unit->deviated = true;
		}
		MoveToLocation(unit, targetLoc, Constants::PATHFINDING_GET_CLOSE);
		return true;
	}
	else
	return false;
	
}

bool MilitaryCommander::PursueClosestBase(Unit* unit)
{
	//Move towards the enemy base
	
	Vector<Base>* enemyBases = gameRep->GetEnemyBases();
	//Quick reject
	if(enemyBases->empty())
	return false;
	
	uint4 distanceToClosestTarget = 10000;
	GameObj* gob = unit->gameObj;
	bool foundATarget = false;
	uint4 targetID;
	Loc targetLoc(0,0);
	Vector<Base>::iterator it = enemyBases->begin();
	
	while(it != enemyBases->end())
	{
		GameObj* gob2 = it->gameObj;
		double distanceFromUnit = MiscFunctions::distance(*gob->sod.x,*gob->sod.y,*gob2->sod.x,*gob2->sod.y);
		if(distanceFromUnit < distanceToClosestTarget)
		{
			foundATarget = true;
			targetID = it->unitID;
			distanceToClosestTarget = distanceFromUnit;
			targetLoc.Set(*gob2->sod.x,*gob2->sod.y);
		}
		it++;
	}
	
	if(foundATarget)
	{
		if(unit->targetID != targetID)
		{
			unit->targetID = targetID;
			unit->deviated = true;
		}
		MoveToLocation(unit, targetLoc, Constants::PATHFINDING_GET_CLOSE);
		return true;
	}
	else
	return false;
}


bool MilitaryCommander::AttackUnits(Unit* unit, uint4 range, bool defendingBase)
{
	uint4 targetID = 0;
	GameObj* gob = unit->gameObj;
	GameObj* targetGob;
	uint4 targetHealth = 1000;
	uint4 enemyUnitRange;
	Vector<Marine>* enemyMarines = gameRep->GetEnemyMarines();
	Vector<Tank>* enemyTanks = gameRep->GetEnemyTanks();
	
	//First check for enemy tanks
	Vector<Tank>::iterator it = enemyTanks->begin();
	while(it != enemyTanks->end())
	{
		GameObj* gob2 = it->gameObj;
		double distanceFromUnit = MiscFunctions::distance(*gob->sod.x,*gob->sod.y,*gob2->sod.x,*gob2->sod.y);
		bool inRange = distanceFromUnit <= range;
		bool closeToBase = IsCloseToBase(Loc(*gob->sod.x,*gob->sod.y));
		bool weakestTarget = (uint4)(it->gameObj->get_int("hp")) < targetHealth;
		if(inRange && (closeToBase || !defendingBase) && weakestTarget)
		{
			targetID = it->unitID;
			targetGob = it->gameObj;
			targetHealth = it->gameObj->get_int("hp");	
			enemyUnitRange = 112;		
			break;
		}
		it++;
	}
	//Else look for enemy marines
	if(!targetID)
	{
		Vector<Marine>::iterator it = enemyMarines->begin();
		while(it != enemyMarines->end())
		{
			GameObj* gob2 = it->gameObj;
			double distanceFromUnit = MiscFunctions::distance(*gob->sod.x,*gob->sod.y,*gob2->sod.x,*gob2->sod.y);
			bool inRange = distanceFromUnit <= range;
			bool closeToBase = IsCloseToBase(Loc(*gob->sod.x,*gob->sod.y));
			bool weakestTarget = (uint4)(it->gameObj->get_int("hp")) < targetHealth;
			if(inRange && (closeToBase || !defendingBase) && weakestTarget)
			{
				targetID = it->unitID;
				targetGob = it->gameObj;
				targetHealth = it->gameObj->get_int("hp");
				enemyUnitRange = 64;
			}
			it++;
		}
	}
	
	if(targetID)
	return AttackObject(unit, targetID, targetGob, enemyUnitRange);
	else
	return false;
}

bool MilitaryCommander::AttackWorkers(Unit* unit, uint4 range)
{
	uint4 targetID = 0;
	GameObj* gob = unit->gameObj;
	GameObj* targetGob;
	uint4 targetHealth = 1000;	
	Vector<Worker>* enemyWorkers = gameRep->GetEnemyWorkers();	
	
	Vector<Worker>::iterator it = enemyWorkers->begin();
	while(it != enemyWorkers->end())
	{
		GameObj* gob2 = it->gameObj;
		double distanceFromUnit = MiscFunctions::distance(*gob->sod.x,*gob->sod.y,*gob2->sod.x,*gob2->sod.y);
		bool inRange = distanceFromUnit <= range;
		bool weakestTarget = (uint4)(it->gameObj->get_int("hp")) < targetHealth;
		if(inRange && weakestTarget)
		{
			targetID = it->unitID;
			targetGob = it->gameObj;
			targetHealth = it->gameObj->get_int("hp");
		}
		it++;
	}
	
	if(targetID)
	return AttackObject(unit, targetID, targetGob, 5);
	else
	return false;
}

bool MilitaryCommander::AttackBases(Unit* unit, uint4 range)
{
	uint4 targetID = 0;
	GameObj* gob = unit->gameObj;
	GameObj* targetGob;
	Vector<Base>* enemyBases = gameRep->GetEnemyBases();
	
	Vector<Base>::iterator it = enemyBases->begin();
	while(it != enemyBases->end())
	{
		GameObj* gob2 = it->gameObj;
		double distanceFromUnit = MiscFunctions::distance(*gob->sod.x,*gob->sod.y,*gob2->sod.x,*gob2->sod.y);
		bool inRange = distanceFromUnit <= range;
		if(inRange)
		{
			targetID = it->unitID;
			targetGob = it->gameObj;
			break;
		}
		it++;
	}
	
	if(targetID)
	return AttackObject(unit, targetID, targetGob, 0);
	else
	return false;
}

bool MilitaryCommander::AttackObject(Unit* unit, uint4 targetID, GameObj* targetGob, uint4 enemyUnitsRange)
{
	GameObj* gob = unit->gameObj;
	//Check to see if the unit can fire, if it can't then run away from the target
	bool canFire = true;
	sint4 lastFired = gob->component("weapon")->get_int("last_fired");
	if (lastFired >= 0) {
		sint4 cooldown = gob->component("weapon")->get_int("cooldown");
		int timeToShoot = cooldown - (gameTick - (uint4)lastFired);

		if (timeToShoot > 0) 
		canFire = false;
	}
	
	

	if(canFire)
	{
		Vector<sint4> args;
		args.push_back(targetID);
		gob->component("weapon")->set_action("attack", args);
		return true;
	}
	else 
	{
		//Run away to a position just outside the enemy units range
		VectorPoint relativePos(*targetGob->sod.x - *gob->sod.x,*targetGob->sod.y - *gob->sod.y);
		relativePos.Normalise();
		relativePos = relativePos * enemyUnitsRange;
		Loc moveTo(*targetGob->sod.x - relativePos.x,*targetGob->sod.y - relativePos.y);
		unit->deviated = true;
		Vector<sint4> params;		
		params.push_back(moveTo.x);
		params.push_back(moveTo.y);		
		gob->set_action("move", params);	
		return true;
	}
}

/*
	When there is nothing to do for the current objective then this function returns the next objective, or a constant signalling to stop
	The boolean indicates whether the calling function is handling unit targetting or movement
*/
uint4 MilitaryCommander::GetNextObjective(uint4 currentObjective, bool targetting)
{
	if(targetting)
	{
		switch(currentObjective)
		{
			case Constants::MODE_DEFEND:
				return Constants::MODE_ATTACK_UNITS;
			case Constants::MODE_ATTACK_UNITS:
				return Constants::MODE_ATTACK_WORKERS;
			case Constants::MODE_ATTACK_WORKERS:
				return Constants::MODE_ATTACK_BASES;
			case Constants::MODE_ATTACK_BASES:
				return Constants::MODE_ATTACK_UNITS;
		}
	}
	else
	{
		switch(currentObjective)
		{
			case Constants::MODE_DEFEND:
				return Constants::STOP;
			case Constants::MODE_ATTACK_UNITS:
				return Constants::MODE_ATTACK_WORKERS;
			case Constants::MODE_ATTACK_WORKERS:
				return Constants::MODE_ATTACK_BASES;
			case Constants::MODE_ATTACK_BASES:
				return Constants::MODE_ATTACK_UNITS;
			case Constants::STOP:
			case Constants::MODE_ATTACK_ALL:
				return Constants::STOP;
			
		}
	}
	assert(0);
	return 0;
}
