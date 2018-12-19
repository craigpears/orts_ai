#include "MapTile.H"

/*
	Constructor
*/
MapTile::MapTile()
{
	partiallyBlocked = false;
	blocked = false;
	mineral = false;
	controlCenter = false;
	enemyBase = false;
	cost = 1;//pathfinding cost
}

//Mutator Functions
//Minerals occupy an entire tile, so if a tile contains a mineral it Is also blocked
void MapTile::SetBlocked(){partiallyBlocked = true; blocked = true;}
void MapTile::SetPartiallyBlocked(){partiallyBlocked = true;}
void MapTile::SetMineral(){partiallyBlocked = true; blocked = true; mineral = true;}
void MapTile::SetControlCenter(){partiallyBlocked = true; controlCenter = true;}
void MapTile::SetEnemyBase(){enemyBase = true;}
void MapTile::Clear(){partiallyBlocked = enemyBase = blocked = controlCenter = false; cost = 1;}
void MapTile::ResetPFCost(){cost = 1;}
void MapTile::AddCost(sint4 _cost){cost += _cost;}

	
	
//Accessor Functions
bool MapTile::IsBlocked(){return blocked;}
bool MapTile::IsPartiallyBlocked(){return partiallyBlocked;}
bool MapTile::IsMineral(){return mineral;}
bool MapTile::IsControlCenter(){return controlCenter;}
bool MapTile::IsEnemyBase(){return enemyBase;}
uint4 MapTile::GetCost(){ return cost;}
