#include "BaseCommander.H"
#include "Blackboard.H"
#include "Constants.H"
#include "Options.H"

using namespace std;

void BaseCommander::ComputeActions(const Game& game)
{
	bool game1;
	Options::get("-game1", game1);
	if(game1)
	return;


	resources = game.get_cplayer_info().global_obj("player")->get_int("minerals");
	Vector<uint4> priorities = strategies.GetPriorities(Blackboard::STRATEGY);
	for(uint i = 0; i < priorities.size(); i++)
	{
		if(ExecuteGoal(priorities[i]))//Check each goal in turn to see if it can be executed, when one can then return
		break;
	}
}

bool BaseCommander::ExecuteGoal(uint4 goal)
{	
	//Returning false allows the next goal to be checked, returning true stops the checks
	switch(goal)
	{
		case Constants::TECH_L0:
			//TODO: Not yet implemented
			return false;
		case Constants::TECH_L1:
			if(Blackboard::BARRACKS_BUILT)
			return false;
			else if(resources < 400)
			return true;//Block any further actions from stealing resources
			else
			{
				BuildBarracks();
				return resources <= 450;//Stop the resources needed to build the barracks from being used
			}
		case Constants::TECH_L2:
			if(Blackboard::FACTORY_BUILT)
			return false; 
			else if(resources < 400)
			return true;//Block any further actions from stealing resources
			else
			{
				BuildFactory();
				return resources <= 450;//Stop the resources needed to build the barracks from being used
			}
		case Constants::ECONOMY_L0:
			return ExecuteEconomicGoal(Constants::ECONOMY_L0_MAX_WORKERS);
		case Constants::ECONOMY_L1:
			return ExecuteEconomicGoal(Constants::ECONOMY_L1_MAX_WORKERS);
		case Constants::ECONOMY_L2:
			return ExecuteEconomicGoal(Constants::ECONOMY_L2_MAX_WORKERS);
		case Constants::ECONOMY_L3:
			return ExecuteEconomicGoal(Constants::ECONOMY_L3_MAX_WORKERS);
		case Constants::MILITARY_L0:
			return ExecuteMilitaryGoal(Constants::MILITARY_L0_MAX_STRENGTH);
		case Constants::MILITARY_L1:
			return ExecuteMilitaryGoal(Constants::MILITARY_L1_MAX_STRENGTH);
		case Constants::MILITARY_L2:
			return ExecuteMilitaryGoal(Constants::MILITARY_L2_MAX_STRENGTH);
		case Constants::MILITARY_L3:
			return ExecuteMilitaryGoal(Constants::MILITARY_L3_MAX_STRENGTH);
			
	}
	assert(0);
	return true;
}

bool BaseCommander::ExecuteEconomicGoal(uint4 maxWorkers)
{
	uint4 economyStrength = gameRep->GetWorkers()->size();
	if(economyStrength >= maxWorkers)
	return false; 
	else if(resources < 50)
	return true;//Block any further actions from stealing resources
	else
	{
		TrainWorker();
		return false;
	}
}

bool BaseCommander::ExecuteMilitaryGoal(uint4 maxStrength)
{
	if(!Blackboard::BARRACKS_BUILT && !Blackboard::FACTORY_BUILT)
	return false;
	uint4 militaryStrength = gameRep->GetMarines()->size() + gameRep->GetTanks()->size() * 5;//TODO take hp into account
	uint4 resourcesNeeded;
	if(!Blackboard::FACTORY_BUILT)
	resourcesNeeded = 50;
	else
	resourcesNeeded = 200;
	
	
	if(militaryStrength >= maxStrength)
	return false;
	else if(resources < resourcesNeeded)
	return true;
	else
	{
		if(Blackboard::FACTORY_BUILT)
		BuildTank();
		else
		TrainMarine();
		
		return false;
	}
	
}


void BaseCommander::BuildBarracks()
{
	Build(Constants::STATE_BUILD_BARRACKS);
}

void BaseCommander::BuildFactory()
{
	Build(Constants::STATE_BUILD_FACTORY);
}

void BaseCommander::Build(uint4 type)
{
	Vector<Worker>* workers = gameRep->GetWorkers();
	Vector<Worker>::iterator it = workers->begin();	
	//Find the worker closest to the build site
	uint4 closestDistance = 10000;
	Worker* worker = 0;
	Loc buildSite = gameRep->GetBarracksBuildSpot();
	
	resources -= 400;
	
	while(it != workers->end())
	{
		if(it->state == type)//Don't assign another worker if one is already doing it
		return;
		
		uint4 distance = abs(*(it->gameObj)->sod.x - (sint4)buildSite.x) + abs(*(it->gameObj)->sod.y - (sint4)buildSite.y);
		if(it->state != Constants::STATE_EXPLORE && distance < closestDistance)
		{
			worker = &(*it);
			closestDistance = distance;
		}
		
		it++;
	}
	
	if(worker)
	worker->state = type;
}

bool BaseCommander::TrainWorker()
{
	GameObj* controlCenter = gameRep->GetControlCenter();
	if(controlCenter->get_int("amount_built") != controlCenter->get_int("build_time")){
		//Building not ready
		return false;
	}

	Vector<sint4> args;
	if (controlCenter->get_int("task_done") != 0) {
		return false;
	}
	controlCenter->set_action("train_worker", args);
	cout << "Training Worker" << endl;
	resources -= 50;
	return true;
}

bool BaseCommander::TrainMarine()
{
	GameObj* barracks = gameRep->GetBarracks();
	if(barracks->get_int("amount_built") != barracks->get_int("build_time")){
		//Building not ready
		return false;
	}

	Vector<sint4> args;
	if (barracks->get_int("task_done") != 0) {
		return false;
	}
	barracks->set_action("train_marine", args);
	cout << "Training Marine" << endl;
	resources -= 50;
	return true;
}

bool BaseCommander::BuildTank()
{
	GameObj* factory = gameRep->GetFactory();
	if(factory->get_int("amount_built") != factory->get_int("build_time")){
		//Building not ready
		return false;
	}

	Vector<sint4> args;
	if (factory->get_int("task_done") != 0) {
		return false;
	}
	factory->set_action("build_tank", args);
	cout << "Building Tank" << endl;
	resources -= 200;
	return true;
}
