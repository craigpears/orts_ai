#ifndef BaseCommander_H
#define BaseCommander_H

#include "Commander.H"
#include "Strategy.H"

class BaseCommander: public Commander
{
	public:
	void ComputeActions(const Game& game);	
	private:
	Strategy strategies;
	bool ExecuteGoal(uint4 goal);
	bool ExecuteMilitaryGoal(uint4 maxStrength);
	bool ExecuteEconomicGoal(uint4 maxWorkers);
	void Build(uint4 type);
	void BuildBarracks();
	void BuildFactory();
	bool TrainWorker();
	bool TrainMarine();
	bool BuildTank();
	uint4 resources;
	
};

#endif
