#include "Blackboard.H"
#include "Constants.H"
#include "Strategy.H"

using namespace std;

Strategy::Strategy()
{
	//Populate the vector arrays
	//Turtle
	//Highest Priority
	turtlePriorities.push_back(Constants::TECH_L0);//Get the basics(you start the game with these)
	turtlePriorities.push_back(Constants::ECONOMY_L0);
	turtlePriorities.push_back(Constants::TECH_L1);//Quickly build up a weak base defence
	turtlePriorities.push_back(Constants::MILITARY_L0);
	turtlePriorities.push_back(Constants::ECONOMY_L1);
	turtlePriorities.push_back(Constants::MILITARY_L1);
	turtlePriorities.push_back(Constants::ECONOMY_L2);
	turtlePriorities.push_back(Constants::TECH_L2);
	turtlePriorities.push_back(Constants::MILITARY_L2);
	turtlePriorities.push_back(Constants::ECONOMY_L3);
	turtlePriorities.push_back(Constants::MILITARY_L3);	
	//LowestPriority
	//Rush
	//Highest Priority
	rushPriorities.push_back(Constants::TECH_L0);
	rushPriorities.push_back(Constants::ECONOMY_L0);
	rushPriorities.push_back(Constants::TECH_L1);//Quickly build up a force to attack with
	rushPriorities.push_back(Constants::MILITARY_L0);
	rushPriorities.push_back(Constants::MILITARY_L1);
	rushPriorities.push_back(Constants::ECONOMY_L1);
	rushPriorities.push_back(Constants::MILITARY_L2);
	rushPriorities.push_back(Constants::ECONOMY_L2);
	rushPriorities.push_back(Constants::TECH_L2);
	rushPriorities.push_back(Constants::MILITARY_L2);
	rushPriorities.push_back(Constants::ECONOMY_L3);
	rushPriorities.push_back(Constants::MILITARY_L3);	
	//LowestPriority
	//Tech Rush
	//Highest Priority
	techRushPriorities.push_back(Constants::TECH_L0);	
	techRushPriorities.push_back(Constants::ECONOMY_L0);
	techRushPriorities.push_back(Constants::ECONOMY_L1);
	techRushPriorities.push_back(Constants::TECH_L1);
	techRushPriorities.push_back(Constants::MILITARY_L0);
	techRushPriorities.push_back(Constants::TECH_L2);
	techRushPriorities.push_back(Constants::MILITARY_L1);
	techRushPriorities.push_back(Constants::MILITARY_L2);
	techRushPriorities.push_back(Constants::ECONOMY_L2);	
	techRushPriorities.push_back(Constants::ECONOMY_L3);
	techRushPriorities.push_back(Constants::MILITARY_L3);	
	//LowestPriority
	//Steamroll
	//Highest Priority
	steamrollPriorities.push_back(Constants::TECH_L0);
	steamrollPriorities.push_back(Constants::ECONOMY_L0);
	steamrollPriorities.push_back(Constants::ECONOMY_L1);
	steamrollPriorities.push_back(Constants::TECH_L1);
	steamrollPriorities.push_back(Constants::MILITARY_L0);
	steamrollPriorities.push_back(Constants::ECONOMY_L2);
	steamrollPriorities.push_back(Constants::MILITARY_L1);
	steamrollPriorities.push_back(Constants::MILITARY_L2);
	steamrollPriorities.push_back(Constants::TECH_L2);		
	steamrollPriorities.push_back(Constants::ECONOMY_L3);
	steamrollPriorities.push_back(Constants::MILITARY_L3);	
	//LowestPriority
	//Balanced
	//Highest Priority
	balancedPriorities.push_back(Constants::TECH_L0);
	balancedPriorities.push_back(Constants::ECONOMY_L0);
	balancedPriorities.push_back(Constants::ECONOMY_L1);
	balancedPriorities.push_back(Constants::TECH_L1);
	balancedPriorities.push_back(Constants::MILITARY_L0);
	balancedPriorities.push_back(Constants::ECONOMY_L2);
	balancedPriorities.push_back(Constants::MILITARY_L1);
	balancedPriorities.push_back(Constants::TECH_L2);
	balancedPriorities.push_back(Constants::MILITARY_L2);			
	balancedPriorities.push_back(Constants::ECONOMY_L3);
	balancedPriorities.push_back(Constants::MILITARY_L3);	
	//LowestPriority
	
}

Vector<uint4> Strategy::GetPriorities(uint4 strategy)
{
	switch(strategy)
	{
		case Constants::STRATEGY_TURTLE:
			return turtlePriorities;
		case Constants::STRATEGY_RUSH:
			return rushPriorities;
		case Constants::STRATEGY_TECH_RUSH:
			return techRushPriorities;
		case Constants::STRATEGY_STEAMROLL:
			return steamrollPriorities;
		case Constants::STRATEGY_BALANCED:
			return balancedPriorities;
	}
	assert(0);
	return 0;
}

void Strategy::CalculateTactics(GameRepresentation* gameRep)
{
	//Compare your military strength to your enemies military strength and attack if stronger
	uint4 ownStrength = gameRep->GetMarines()->size() + gameRep->GetTanks()->size() * 5;
	uint4 enemyStrength = Blackboard::ENEMY_STRENGTH;
	//cout << "The enemy has " << gameRep->GetEnemyMarines()->size() << " marines and " << gameRep->GetEnemyTanks()->size() << " tanks" << endl;
	//cout << "Enemy Strength = " << enemyStrength << ", My strength = " << ownStrength << endl;
	if(ownStrength >= enemyStrength)
	Blackboard::UNIT_MODE = Constants::MODE_ATTACK_UNITS;
	else
	Blackboard::UNIT_MODE = Constants::MODE_DEFEND;
}
