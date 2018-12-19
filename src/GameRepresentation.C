#include "Blackboard.H"
#include "Constants.H"
#include "GameRepresentation.H"
#include "Options.H"

using namespace std;
	
void GameRepresentation::SetCliffBoundaries(Vector<Line> boundaries){cliffBoundaries = boundaries;}
void GameRepresentation::SetMaps(Vector<Vector<MapTile> > smallTMap, Vector<Vector<MapTile> > bigTMap)
{
	smallTilesMap = smallTMap;
	bigTilesMap = bigTMap;
}
void GameRepresentation::SetVisibleMineralLocations(Vector<Loc> locations){ visibleMineralLocations = locations;}
Vector<Vector<MapTile> > GameRepresentation::GetSmallMap(){ return smallTilesMap;}
Vector<Vector<MapTile> > GameRepresentation::GetBigMap(){ return bigTilesMap;}
Vector<Line> GameRepresentation::GetCliffBoundaries(){return cliffBoundaries;}
Vector<Loc> GameRepresentation::GetVisibleMineralLocations(){ return visibleMineralLocations;}

Loc GameRepresentation::GetBarracksBuildSpot()
{
	//The buildings are 4 wide and 3 high
	GameObj* controlCenter = GetControlCenter();
	
	//Create an influence map for determining the best place to put the control center	
	uint4 influenceMap[Constants::NO_TILES][Constants::NO_TILES];
	
	//First set the values according to their distance from the control center
	Loc controlCenterLoc(*controlCenter->sod.x / 16,*controlCenter->sod.y / 16);
	for(uint4 i = 0; i < Constants::NO_TILES; i++)
	{
		for(uint4 j = 0; j < Constants::NO_TILES; j++)
		{
			uint4 distance = abs((sint4)controlCenterLoc.x - (sint4)i) + abs((sint4)controlCenterLoc.y - (sint4)j);
			uint4 value;
			if(distance < 10)
			value = 0;
			else if(distance < 15)
			value = 20;
			else 
			value = 5;
			
			influenceMap[i][j] = value;
		}
	}
	
	//Next block off all placements that are either blocked by a mineral or would obstruct access to one
	Vector<Mineral>::iterator it = minerals.begin();
	while(it != minerals.end())
	{
		sint4 mineralX,mineralY;
		it->gameObj->get_center(mineralX,mineralY);
		uint4 startX = max(0,mineralX - 4);
		uint4 startY = max(0,mineralY - 4);
		uint4 finishX = min((sint4)Constants::NO_TILES - 1, mineralX + 4);
		uint4 finishY = min((sint4)Constants::NO_TILES - 1, mineralY + 4);
		for(uint4 i = startX; i <= finishX; i++)
		{
			for(uint4 j = startY; j <= finishY; j++)
			influenceMap[i][j] = 0;
		}
		it++;
	}
	
	//Next block off all placements that are overlapping a cliff/building or are at the edge of the map
	for(uint4 i = 0; i < Constants::NO_TILES; i++)
	{
		for(uint4 j = 0; j < Constants::NO_TILES; j++)
		{
			if(bigTilesMap[i][j].IsPartiallyBlocked() || i == 0 || i == Constants::NO_TILES - 1 || j == 0 || j == Constants::NO_TILES - 1)
			{
				uint4 startX = max((sint4)0,(sint4)i - 2);
				uint4 startY = max((sint4)0,(sint4)j - 2);
				uint4 finishX = min((sint4)Constants::NO_TILES - 1, (sint4)i + 3);
				uint4 finishY = min((sint4)Constants::NO_TILES - 1, (sint4)j + 2);
				for(uint4 k = startX; k <= finishX; k++)
				{
					for(uint4 l = startY; l <= finishY; l++)
					influenceMap[k][l] = 0;
				}
			}	
		}
	}
	
	
	//Search through the influence map for the best tile
	uint4 bestValue = 0, x = 0, y = 0;
	for(uint4 i = 0; i < Constants::NO_TILES; i++)
	{
		for(uint4 j = 0; j < Constants::NO_TILES; j++)
		{
			if(influenceMap[i][j] > bestValue)
			{
				x = i;
				y = j;
				bestValue = influenceMap[i][j];
			}
		}
	}	
	
	
	//Return the position in world co-ordinates
	Loc loc(x * 16 + 8, y * 16 + 8);
	return loc;
}
	
Worker* GameRepresentation::GetWorker(uint4 workerID)
{
	for(uint4 i = 0; i < workers.size(); i++)
	{
		if(workerID == workers[i].unitID)
		return &workers[i];
	}
	cout << "WARNING!! Worker assignment not found" << endl;
	return &workers[0];//Should never get here, just to stop the compiler throwing a warning
}

Marine* GameRepresentation::GetMarine(uint4 unitID)
{
	for(uint4 i = 0; i < marines.size(); i++)
	{
		if(unitID == marines[i].unitID)
		return &marines[i];
	}
	cout << "WARNING!! Marine not found" << endl;
	return &marines[0];//Should never get here, just to stop the compiler throwing a warning
}

Mineral* GameRepresentation::GetMineral(uint4 mineralID)
{
	for(uint4 i = 0; i < minerals.size(); i++)
	{
		if(mineralID == minerals[i].mineralID)
		return &minerals[i];
	}
	cout << "WARNING!! Marine not found" << endl;
	return &minerals[0];//Should never get here, just to stop the compiler throwing a warning
}

Vector<Mineral>* GameRepresentation::GetMineralsList(){	return &minerals;}
Vector<Worker>* GameRepresentation::GetWorkers(){ return &workers;}
Vector<Worker>* GameRepresentation::GetEnemyWorkers(){ return &enemyWorkers;}
Vector<Marine>* GameRepresentation::GetMarines(){ return &marines;}
Vector<Marine>* GameRepresentation::GetEnemyMarines(){ return &enemyMarines;}
Vector<Tank>* GameRepresentation::GetTanks(){ return &tanks;}
Vector<Tank>* GameRepresentation::GetEnemyTanks(){ return &enemyTanks;}
Vector<Base>* GameRepresentation::GetMyBases(){ return &myBases;}
Vector<Base>* GameRepresentation::GetEnemyBases(){ return &enemyBases;}

void GameRepresentation::AddBase(GameObj* gob, uint4 unitID, uint4 type, bool own)
{
	//Check to see if the base already exists
	if(own)
	{
		Vector<Base>::iterator it = myBases.begin();
		while(it != myBases.end())
		{
			if(it->x == *gob->sod.x && it->y == *gob->sod.y)
			return;
			it++;
		}
	}
	else
	{
		Vector<Base>::iterator it = enemyBases.begin();
		while(it != enemyBases.end())
		{
			if(it->x == *gob->sod.x && it->y == *gob->sod.y)
			{
				it->unitID = unitID;
				it->objectID = gob->get_obj_id(gob);
				it->gameObj = gob;
				return;
			}
			it++;
		}
	}
		
	if(!own)
	cout << "Found enemy base" << endl;			
	
	MapChange changes;
	sint4 x1,x2,y1,y2;
	gob->get_p1(x1,y1); // upper left corner
  	gob->get_p2(x2,y2); // lower right corner
  	  	  	
  	Vector<Line> barriers;
  	Line line1(x1,y1,x1,y2),line2(x1,y1,x2,y1),line3(x2,y2,x1,y2),line4(x2,y2,x2,y1);
  	barriers.push_back(line1);
  	barriers.push_back(line2);
  	barriers.push_back(line3);
  	barriers.push_back(line4);
  	changes.barrierChanges = barriers;
  	
	Vector<TileChange> tileChanges;
	bool cc = type == Constants::BASE_CONTROL && own;
	for(sint4 i = x1/8; i < (x2/8)+1; i++)
	{
		for(sint4 j = y1/8; j < (y2/8)+1; j++)
		{			
			TileChange tileChange(i,j,true,true,false,cc,own);
			tileChanges.push_back(tileChange);
		}
	}
	changes.tileChanges = tileChanges;
  	  	
	Base newBase;
  	newBase.changes = changes;
  	newBase.type = type;
  	newBase.gameObj = gob;
  	newBase.unitID = unitID;
  	newBase.objectID = gob->get_obj_id(gob);
  	gob->get_center(newBase.x,newBase.y);
	
	if(own)
	myBases.push_back(newBase);
	else
	enemyBases.push_back(newBase);
}

Vector<Line> GameRepresentation::GetBuildingBoundaries()
{
	Vector<Line> lines;
	Vector<Base>::iterator it = myBases.begin();
	while(it != myBases.end())
	{
		Vector<Line> buildingBoundaries = it->changes.barrierChanges;
		FORALL(buildingBoundaries,lit)
		lines.push_back(*lit);
		it++;
	}
	return lines;
}

void GameRepresentation::AddMineral(Mineral min)
{		
	//Check that this mineral doesn't already exist
	FORALL(minerals,it)
	{
		if(it->gameObj->sod.x == min.gameObj->sod.x && it->gameObj->sod.y == min.gameObj->sod.y)
		{
			it->gameObj = min.gameObj;
			it->mineralID = min.mineralID;
			return;
		}		
	}
	minerals.push_back(min);
	sort(minerals.begin(),minerals.end());
}

GameObj* GameRepresentation::GetControlCenter()
{
	Vector<Base>::iterator it = myBases.begin();
	while(it != myBases.end())
	{
		if(it->type == Constants::BASE_CONTROL)
		return it->gameObj;
		it++;
	}
	assert(0);
	return myBases[0].gameObj;
}

GameObj* GameRepresentation::GetBarracks()
{
	Vector<Base>::iterator it = myBases.begin();
	while(it != myBases.end())
	{
		if(it->type == Constants::BASE_BARRACKS)
		return it->gameObj;
		it++;
	}
	assert(0);
	return myBases[0].gameObj;
}

GameObj* GameRepresentation::GetFactory()
{
	Vector<Base>::iterator it = myBases.begin();
	while(it != myBases.end())
	{
		if(it->type == Constants::BASE_FACTORY)
		return it->gameObj;
		it++;
	}
	assert(0);
	return myBases[0].gameObj;
}

Loc GameRepresentation::GetEnemyBaseLoc()
{	
	Loc loc(enemyBases[0].x,enemyBases[0].y);
	return loc;
}

void GameRepresentation::RemoveMineral(uint4 objectID)
{
	Vector<Mineral>::iterator it = minerals.begin();
	while(it != minerals.end())
	{
		if(it->mineralID == objectID)
		{
			//Mark all workers that were working here as unassigned
			Vector<Worker>::iterator wit = workers.begin();
			while(wit != workers.end())
			{
				if(wit->mineralID == objectID)
				wit->assigned = false;
				wit++;
			}
			//Delete the mineral
			minerals.erase(it);
		}
		it++;
	}
}

void GameRepresentation::AddWorker(GameObj* gob, uint4 unitID, bool own)
{
	Worker w;		
	w.gameObj = gob;
	w.unitID = unitID;
	w.objectID = gob->get_obj_id(gob);
	
	
	if(own)
	{
		bool game1;
  		Options::get("-game1", game1);
  		
		if(noExplorers == 2 || game1)
		w.state = Constants::STATE_MOVE_TO_MINE;
		else
		{
			w.state = Constants::STATE_EXPLORE;
			noExplorers++;
		}
		workers.push_back(w);
	}
	else
	{		
		enemyWorkers.push_back(w);
		CheckStrength();
	}
}

void GameRepresentation::AddMarine(GameObj* gob, uint4 unitID, bool own)
{
	Marine marine;
	marine.gameObj = gob;
	marine.unitID = unitID;
	marine.objectID = gob->get_obj_id(gob);
	if(own)
	marines.push_back(marine);
	else
	{		
		enemyMarines.push_back(marine);
		CheckStrength();
	}
}

void GameRepresentation::AddTank(GameObj* gob, uint4 unitID, bool own)
{
	Tank tank;
	tank.gameObj = gob;
	tank.unitID = unitID;
	tank.objectID = gob->get_obj_id(gob);
	if(own)
	tanks.push_back(tank);
	else
	{
		enemyTanks.push_back(tank);
		CheckStrength();
	}
}

void GameRepresentation::CheckStrength()
{
	uint4 strength = enemyMarines.size() + enemyTanks.size() * 5;
	Blackboard::ENEMY_STRENGTH = max(strength, Blackboard::ENEMY_STRENGTH);
}

void GameRepresentation::WorkerVanished(uint4 objectID)
{
	Vector<Worker>::iterator it = enemyWorkers.begin();
	while(it != enemyWorkers.end())
	{
		if(it->objectID == objectID)
		{
			enemyWorkers.erase(it);
			return;
		}
		it++;
	}
	assert(0);
}

void GameRepresentation::MarineVanished(uint4 objectID)
{
	Vector<Marine>::iterator it = enemyMarines.begin();
	while(it != enemyMarines.end())
	{
		if(it->objectID == objectID)
		{
			enemyMarines.erase(it);
			return;
		}
		it++;
	}	
	assert(0);
}

void GameRepresentation::TankVanished(uint4 objectID)
{
	Vector<Tank>::iterator it = enemyTanks.begin();
	while(it != enemyTanks.end())
	{
		if(it->objectID == objectID)
		{
			enemyTanks.erase(it);
			return;
		}
		it++;
	}
	assert(0);
}

void GameRepresentation::RemoveWorker(uint4 objectID)
{
	Vector<Worker>::iterator it = enemyWorkers.begin();
	while(it != enemyWorkers.end())
	{
		if(it->objectID == objectID)
		{
			enemyWorkers.erase(it);
			return;
		}
		it++;
	}
	
	it = workers.begin();
	while(it != workers.end())
	{
		if(it->objectID == objectID)
		{
			workers.erase(it);
			return;
		}
		it++;
	}
	assert(0);
}

void GameRepresentation::RemoveMarine(uint4 objectID)
{
	Vector<Marine>::iterator it = enemyMarines.begin();
	while(it != enemyMarines.end())
	{
		if(it->objectID == objectID)
		{
			enemyMarines.erase(it);			
			Blackboard::ENEMY_STRENGTH--;
			return;
		}
		it++;
	}
	
	it = marines.begin();
	while(it != marines.end())
	{
		if(it->objectID == objectID)
		{
			marines.erase(it);
			return;
		}
		it++;
	}
	assert(0);
}

void GameRepresentation::RemoveTank(uint4 objectID)
{
	Vector<Tank>::iterator it = enemyTanks.begin();
	while(it != enemyTanks.end())
	{
		if(it->objectID == objectID)
		{
			enemyTanks.erase(it);
			
			Blackboard::ENEMY_STRENGTH -= 5;
			return;
		}
		it++;
	}
	
	it = tanks.begin();
	while(it != tanks.end())
	{
		if(it->objectID == objectID)
		{
			tanks.erase(it);
			return;
		}
		it++;
	}
	assert(0);
}

void GameRepresentation::RemoveBase(uint4 objectID)
{
	Vector<Base>::iterator it = enemyBases.begin();
	while(it != enemyBases.end())
	{
		if(it->objectID == objectID)
		{
			enemyBases.erase(it);
			if(enemyBases.empty())
			Blackboard::ENEMY_BASE_FOUND = false;
			return;
		}
		it++;
	}
	
	it = myBases.begin();
	while(it != myBases.end())
	{
		if(it->objectID == objectID)
		{
			myBases.erase(it);
			return;
		}
		it++;
	}
	assert(0);
}
