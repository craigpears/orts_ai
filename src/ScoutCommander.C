#include "Blackboard.H"
#include "Constants.H"
#include "ScoutCommander.H"
#include "MiscFunctions.H"
#include "Structures.H"

using namespace std;

ScoutCommander::ScoutCommander()
{
	InitLocations();
	exploredAllLocs = false;
}

void ScoutCommander::InitLocations()
{
	//Create a list of areas for scouts to explore, a 9x9 grid for now TODO: improve this
	for(uint4 i = 0; i < 4; i++)
	{
		for(uint4 j = 0; j < 4; j++)
		{
			Loc target(i * 256 + 128,j * 256 + 128);
			exploreTargets.push_back(target);
		}
	}
}

void ScoutCommander::ComputeActions()
{
	Vector<Worker>* workers = gameRep->GetWorkers();
	Vector<Worker>::iterator it = workers->begin();
	/*
  		Workers only explore to find the enemy base, as soon as it is discovered they go back to mining and marines take
  		over scouting duties so they can harass enemy workers/buildings at the same time
  	*/
  	//TODO: we know that the enemy base is next to a mineral patch, so if a worker identifies a mineral patch somewhere then it should be investigated
  	//For now, just explore a random place on the map at first then the closest after that
  	while(it != workers->end())
  	{
  		if(it->state == Constants::STATE_EXPLORE)
  		{
  			Worker* worker = &(*it);
		  	GameObj* gob = worker->gameObj;
		  	
		  	if(Blackboard::ENEMY_BASE_FOUND && exploredAllLocs)
		  	{
		  		it->state = Constants::STATE_MOVE_TO_MINE;
		  	}
		  	
		  	if(worker->exploreTarget.x == 0 && worker->exploreTarget.y == 0)
		  	{  		
		  		real8 distance = 2000;
  				uint4 index = 0;
  				for(uint4 i = 0; i < exploreTargets.size(); i++)
  				{
  					real8 distanceToLoc = MiscFunctions::distance(*(worker->gameObj)->sod.x,*(worker->gameObj)->sod.y,exploreTargets[i].x,exploreTargets[i].y);
  					if(distanceToLoc < distance)
  					{
  						distance = distanceToLoc;
  						index = i;
  					}
  				}		
		  		worker->exploreTarget = exploreTargets[index];
		  		exploreTargets.erase(exploreTargets.begin()+index);
		  	}
		  	if(*gob->sod.speed == 0)
		  	{
		  		if(worker->isStuck)
		  		MoveRandomly(worker);
		  		else
		  		{
		  			bool moved = MoveToLocation(worker, worker->exploreTarget, Constants::PATHFINDING_GET_CLOSE);
		  			if(!moved)//You can't find a path, pick a new target
		  			{
		  				if(exploreTargets.size() == 0)
		  				{
		  					exploredAllLocs = true;
		  					InitLocations();
		  				}
		  				
		  				real8 distance = 2000;
		  				uint4 index = 0;
		  				for(uint4 i = 0; i < exploreTargets.size(); i++)
		  				{
		  					real8 distanceToLoc = MiscFunctions::distance(*(worker->gameObj)->sod.x,*(worker->gameObj)->sod.y,exploreTargets[i].x,exploreTargets[i].y);
		  					if(distanceToLoc < distance)
		  					{
		  						distance = distanceToLoc;
		  						index = i;
		  					}
		  				}		
				  		worker->exploreTarget = exploreTargets[index];
				  		exploreTargets.erase(exploreTargets.begin()+index);
		  			}
		  		}
		  		worker->isStuck = true;
		  	}
		  	else
		  	worker->isStuck = false;
		}
	  	
	  	it++;
	}
}
