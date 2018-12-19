#ifndef Constants_H
#define Constants_H

#include "Global.H"

namespace Constants{
	static const uint4 NO_TILES = 64;
	
	//Worker state constants
	static const uint4 STATE_EXPLORE = 0;
	static const uint4 STATE_MOVE_TO_MINE = 1;
	static const uint4 STATE_MINE = 2;
	static const uint4 STATE_MOVE_TO_BASE = 3;
	static const uint4 STATE_BUILD_BARRACKS = 4;
	static const uint4 STATE_BUILD_FACTORY = 5;
		
	//Building types
	static const uint4 BASE_CONTROL = 0;
	static const uint4 BASE_BARRACKS = 1;
	static const uint4 BASE_FACTORY = 2;
		
	//Pathfinding modes
	static const uint4 PATHFINDING_BIG_TILES = 0;
	static const uint4 PATHFINDING_IGNORE_CC = 1;
	static const uint4 PATHFINDING_NO_SMOOTH = 2;
	static const uint4 PATHFINDING_SMALL_TILES = 3;	
	static const uint4 PATHFINDING_IGNORE_ENEMY_BUILDINGS = 4;
	static const uint4 PATHFINDING_GET_CLOSE = 5;
	
	//Unit types for use by the information tracker
	static const uint4 TYPE_WORKER = 0;
	static const uint4 TYPE_MARINE = 0;
	static const uint4 TYPE_TANK = 0;	
	
	//Unit modes
	static const uint4 MODE_DEFEND = 0;
	static const uint4 MODE_ATTACK_UNITS = 1;
	static const uint4 MODE_ATTACK_WORKERS = 2;
	static const uint4 MODE_ATTACK_BASES = 3;
	static const uint4 MODE_ATTACK_ALL = 4;
	static const uint4 MODE_REGROUP = 5;//Not yet implemented
	static const uint4 STOP = 6;
	static const uint4 NUMBER_OF_OBJECTIVES = 5;
	
	//Strategies
	static const uint4 STRATEGY_TURTLE = 0;//Defends the base
	static const uint4 STRATEGY_RUSH = 1;//Attacks quickly
	static const uint4 STRATEGY_TECH_RUSH = 2;//Builds a factory quickly
	static const uint4 STRATEGY_STEAMROLL = 3;//Builds a strong attack quickly
	static const uint4 STRATEGY_BALANCED = 4;//Builds a strong attack quickly
	
	//Goals(sub-components of strategies)
	static const uint4 TECH_L0 = 0;//Build control center
	static const uint4 TECH_L1 = 1;//Build barracks
	static const uint4 TECH_L2 = 2;//Build factory
	
	static const uint4 ECONOMY_L0 = 3;
	static const uint4 ECONOMY_L1 = 4;
	static const uint4 ECONOMY_L2 = 5;
	static const uint4 ECONOMY_L3 = 6;
	
	static const uint4 ECONOMY_L0_MAX_WORKERS = 6;
	static const uint4 ECONOMY_L1_MAX_WORKERS = 9;
	static const uint4 ECONOMY_L2_MAX_WORKERS = 14;
	static const uint4 ECONOMY_L3_MAX_WORKERS = 20;
	
	static const uint4 MILITARY_L0 = 7;//4 Marines(1 tank = 5 marines)
	static const uint4 MILITARY_L1 = 8;//10 marines
	static const uint4 MILITARY_L2 = 9;//25 marines
	static const uint4 MILITARY_L3 = 10;//Unlimited military
		
	static const uint4 MILITARY_L0_MAX_STRENGTH = 4;//Marine = 1, Tank = 5
	static const uint4 MILITARY_L1_MAX_STRENGTH = 10;
	static const uint4 MILITARY_L2_MAX_STRENGTH = 25;
	static const uint4 MILITARY_L3_MAX_STRENGTH = 1000;
	
};

#endif
