#ifndef MapTile_H
#define MapTile_H

#include "GameObj.H"

using namespace std;

/*
	This class stores information about a game tile in the abstract representation of the game state
*/

class MapTile{
	public:
	MapTile(); 
	
	//Mutator Functions
	void SetBlocked();
	void SetPartiallyBlocked();
	void SetMineral();
	void SetEnemyBase();
	void SetControlCenter();
	void AddCost(sint4 _cost);
	void Clear();
	void ResetPFCost();
	
	//Accessor Functions
	bool IsBlocked();
	bool IsPartiallyBlocked();
	bool IsMineral();
	bool IsEnemyBase();
	bool IsControlCenter();
	uint4 GetCost();
	
	
	private:	
	bool partiallyBlocked;//Whether it is partly blocked, means that only one unit at a time should be on this tile rather than two, should always be true if blocked is
	bool blocked;//Whether this tile is completely blocked by barriers or not
	bool mineral;//Whether or not this tile contains a mineral
	bool controlCenter;//Whether or not this tile is on the edge of a command center (ones in the middle are blocked tiles)
	bool enemyBase;
	uint4 cost;//Pathfinding cost for going to this tile
	
};

#endif
