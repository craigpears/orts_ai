#include "Constants.H"
#include "MiscFunctions.H"
#include "Pathfinding.H"

using namespace std;

void Pathfinding::Init(GameRepresentation* _gameRep)
{
	gameRep = _gameRep;
	initialised = true;
	UpdateMaps();	
}

void Pathfinding::UpdateObstacles(const Game& game)
{
	uint4 i,j;
	//Reset the costs back to 0
	for(i = 0; i < Constants::NO_TILES; i++)
	{
		for(j = 0; j < Constants::NO_TILES; j++)
		{
			bigTilesMap[i][j].ResetPFCost();
		}
	}
	
	for(i = 0; i < Constants::NO_TILES * 2; i++)
	{
		for(j = 0; j < Constants::NO_TILES * 2; j++)
		{
			smallTilesMap[i][j].ResetPFCost();
		}
	}
	
	Vector<Worker>* workers = gameRep->GetWorkers();
	Vector<Worker>::iterator it = workers->begin();
	while(it != workers->end())
	{	
		GameObj * gob = it->gameObj;	
		sint4 x,y;
		      		
		//Small tiles
		x = *gob->sod.x / 8;
		y = *gob->sod.y / 8;
		if(x > 0)
		{
			assert(x < 128 && y < 128);
			//This is to stop workers trying to get down single tile paths where lots of collisions are likely unless there is no other choice
			if(it->state == Constants::STATE_MINE)
			smallTilesMap[x][y].AddCost(5);
			else
			smallTilesMap[x][y].AddCost(2);
		}
				
		it++;
  }
	
	Vector<Marine>* marines = gameRep->GetMarines();
	Vector<Marine>::iterator it2 = marines->begin();
	while(it2 != marines->end())
	{	
		GameObj * gob = it2->gameObj;	
		sint4 x,y;
				
		//Small tiles
		x = *gob->sod.x / 16;
		y = *gob->sod.y / 16;
		assert(x < 128 && y < 128);
		bigTilesMap[x][y].AddCost(3);

		it2++;
	}

  Vector<Tank>* tanks = gameRep->GetTanks();
	Vector<Tank>::iterator it3 = tanks->begin();
	while(it3 != tanks->end())
	{	
		GameObj * gob = it3->gameObj;	
		sint4 x,y;
		      		
		//Small tiles
		x = *gob->sod.x / 16;
		y = *gob->sod.y / 16;
		assert(x < 128 && y < 128);
		bigTilesMap[x][y].AddCost(3);
				
		it3++;
  }
  
}

/*
	In the pathfinding class because it requires pathfinding and unable to place it in game representation without a circular dependency
*/
void Pathfinding::AddMineral(GameObj* gob, uint4 mineralID)
{
	//cout << "Adding Mineral" << endl;
	GameObj* controlCenter = gameRep->GetControlCenter();
	Loc ccLoc((*controlCenter->sod.x),*controlCenter->sod.y);
	Loc mLoc(*gob->sod.x,*gob->sod.y);
	sint4 pathLength = (FindPath(WCPos(mLoc),WCPos(ccLoc),Constants::PATHFINDING_NO_SMOOTH)).locations.size();
	//cout << "Path Length = " << pathLength << endl;
	if(pathLength > 1)
	{
		Mineral min(gob,pathLength,mineralID);
		gameRep->AddMineral(min);
	}	
}

/*
	Note: This assumes that locations are using tile co-ordinates, not point co-ordinates
*/
Path Pathfinding::FindPath(WCPos _startLoc, WCPos _targetLoc, uint4 mode)
{
	Loc startLoc, targetLoc;
	uint4 tileNo = 64;
	
	if(mode == Constants::PATHFINDING_SMALL_TILES || mode == Constants::PATHFINDING_IGNORE_CC)
	{
		//Use small tiles				
		startLoc = _startLoc.ToSTPos().loc;
		targetLoc = _targetLoc.ToSTPos().loc;
		
		tileNo *= 2;
	}
	if(mode != Constants::PATHFINDING_SMALL_TILES && mode != Constants::PATHFINDING_IGNORE_CC)
	{
		//Use big tiles
		startLoc =_startLoc.ToBTPos().loc;
		targetLoc = _targetLoc.ToBTPos().loc;	
	}
	
	Path startPath(startLoc, targetLoc);
	PathPriorityQueue pq;	
	closedList.clear();
	closedList.push_back(VisitedTile(startLoc,0));
	pq.InsertNode(startPath);
		
	if(mode == Constants::PATHFINDING_BIG_TILES && bigTilesMap[targetLoc.x][targetLoc.y].IsPartiallyBlocked())
	{	
		//cout << "destination isn't accessible" << endl;
		return startPath;
	}
	
	while(!pq.myList.empty())
	{
		Path currentPath = pq.front();
		bool goalCondition = false;
		if(mode == Constants::PATHFINDING_IGNORE_CC)
		{
			Loc loc = currentPath.locations.back();
			if(mode == Constants::PATHFINDING_SMALL_TILES || mode == Constants::PATHFINDING_IGNORE_CC)
			{
				if(smallTilesMap[loc.x][loc.y].IsControlCenter() && smallTilesMap[loc.x][loc.y].IsBlocked())//If the path leads to the middle of a control center
				goalCondition = true;
			}
			else
			{
				if(bigTilesMap[loc.x][loc.y].IsControlCenter() && bigTilesMap[loc.x][loc.y].IsBlocked())//If the path leads to the middle of a control center
				goalCondition = true;
			}
		}
		if(mode == Constants::PATHFINDING_GET_CLOSE && currentPath.GetHeuristicCost() < 5)
		goalCondition = true;
		
		if(currentPath.AtTarget())
		goalCondition = true;
		
		if(goalCondition)
		{		
			if(mode == Constants::PATHFINDING_NO_SMOOTH)
			{
				return currentPath.WorldCoords(mode);//Tranforms the path from tile co-ordinates into world co-ordinates
			}
			else
			return SmoothPath(currentPath.WorldCoords(mode), mode);
		}
		
		//Stop searching if it is obvious that a path can't be found
		if(pq.myList.size() > 1000)
		{
			cout << "Path too long" << endl;
			cout << "was trying to find a path from " << startLoc.x << ", " << startLoc.y << " to " << targetLoc.x << ", " << targetLoc.y << endl;
			break;
		}
				
		//Add all adjacent tiles
		Loc currentLoc = currentPath.locations.back();
		assert(currentLoc.x < tileNo && currentLoc.y < tileNo);
		
		if(currentLoc.x > 0)
		{
			if(Traversable(currentLoc.x - 1, currentLoc.y, targetLoc.x, targetLoc.y, mode))
			{
				Path newPath = currentPath;
				Loc newLoc(currentLoc.x - 1, currentLoc.y);				
				newPath.AddLoc(newLoc, GetCost(newLoc, startLoc, mode));
				if(!InClosedList(VisitedTile(newLoc,newPath.GetCost())))
				pq.InsertNode(newPath);
			}
		}
		if((uint4)currentLoc.x < tileNo - 1)
		{
			if(Traversable(currentLoc.x + 1, currentLoc.y, targetLoc.x, targetLoc.y, mode))
			{
				Path newPath = currentPath;
				Loc newLoc(currentLoc.x + 1, currentLoc.y);
				newPath.AddLoc(newLoc, GetCost(newLoc, startLoc, mode));
				if(!InClosedList(VisitedTile(newLoc,newPath.GetCost())))
				pq.InsertNode(newPath);
			}
		}
		if(currentLoc.y > 0)
		{
			if(Traversable(currentLoc.x, currentLoc.y - 1, targetLoc.x, targetLoc.y, mode))
			{
				Path newPath = currentPath;
				Loc newLoc(currentLoc.x, currentLoc.y - 1);
				newPath.AddLoc(newLoc, GetCost(newLoc, startLoc, mode));
				if(!InClosedList(VisitedTile(newLoc,newPath.GetCost())))
				pq.InsertNode(newPath);
			}
		}
		if((uint4)currentLoc.y < tileNo - 1)
		{
			if(Traversable(currentLoc.x, currentLoc.y + 1, targetLoc.x, targetLoc.y, mode))
			{
				Path newPath = currentPath;
				Loc newLoc(currentLoc.x, currentLoc.y + 1);
				newPath.AddLoc(newLoc, GetCost(newLoc, startLoc, mode));
				if(!InClosedList(VisitedTile(newLoc,newPath.GetCost())))
				pq.InsertNode(newPath);
			}
		}
		
	}
	cout << "Couldn't find a path!!" << endl;
	//Can't find a path
	return startPath;
	
}

uint4 Pathfinding::GetCost(Loc loc, Loc startLoc, uint4 mode)
{ 
	if(mode == Constants::PATHFINDING_SMALL_TILES || mode == Constants::PATHFINDING_IGNORE_CC)
	{
		assert(loc.x < 128 && loc.y < 128);
		//If the tile is in the same big tile region as the start, then reduce the cost by ten so that the unit doesn't try and avoid itself
		uint4 adjustment = 0;
		if(startLoc.x / 2 == loc.x / 2 && startLoc.y / 2 == loc.y / 2)
		adjustment = 2;
		return bigTilesMap[loc.x/2][loc.y/2].GetCost() - adjustment;
	}
	else
	{
		assert(loc.x < 64 && loc.y < 64);
		return bigTilesMap[loc.x][loc.y].GetCost();
	}
}

bool Pathfinding::Traversable(uint4 x, uint4 y, uint4 tX, uint4 tY, uint4 mode)//target x and y
{
	if(mode == Constants::PATHFINDING_SMALL_TILES || mode == Constants::PATHFINDING_IGNORE_CC)
	{
		//Use small tiles		
		assert(x < 128 && y < 128);
		assert(tX < 128 && tY < 128);
		bool pB =  smallTilesMap[x][y].IsPartiallyBlocked();
		bool isMineral = smallTilesMap[x][y].IsMineral();
		bool isTarget = x/2 == tX/2 && y/2 == tY/2;
		bool targetIsControlCenter = smallTilesMap[tX][tY].IsControlCenter();
		bool isControlCenter = smallTilesMap[x][y].IsControlCenter();
		return !pB || (isMineral && isTarget) || (targetIsControlCenter && isControlCenter);
	}
	else
	{
		//Use big tiles
		assert(x < 64 && y < 64);
		assert(tX < 64 && tY < 64);
		bool pB =  bigTilesMap[x][y].IsPartiallyBlocked();
		bool isMineral = bigTilesMap[x][y].IsMineral();
		bool isTarget = x == tX && y == tY;
		bool isControlCenter = bigTilesMap[x][y].IsControlCenter();
		bool targetIsControlCenter = bigTilesMap[tX][tY].IsControlCenter();
		bool isEnemyBase = bigTilesMap[x][y].IsEnemyBase();
		return isEnemyBase || !pB || (isMineral && isTarget) || (targetIsControlCenter && isControlCenter);
	}	
}

bool Pathfinding::InClosedList(VisitedTile tile)
{
	for(uint4 i = 0; i < closedList.size(); i++)
	{
		if(tile == closedList[i] && tile.cost >= closedList[i].cost)
		{
			closedList[i].cost = tile.cost;
			return true;
		}
	}
	closedList.push_back(tile);
	return false;
}

Path Pathfinding::SmoothPath(Path path, uint4 mode)
{
	if(path.locations.size() <= 2)
	return path;//A path without at least three nodes can't be smoothed
	
	Path outputPath(path.locations[0], path.locations.back());
	uint4 inputIndex = 2;
	
	if(mode == Constants::PATHFINDING_SMALL_TILES || mode == Constants::PATHFINDING_IGNORE_CC)
	while(inputIndex < path.locations.size() - 1)
	{
		Loc startLoc = outputPath.locations.back();
		Loc targetLoc = path.locations[inputIndex];
		if(startLoc.x != targetLoc.x && startLoc.y != targetLoc.y)
		outputPath.AddLoc(path.locations[inputIndex-1],0);
		
		inputIndex++;
	}
	else
	{
		Vector<Line> buildingBoundaries = gameRep->GetBuildingBoundaries();
		Vector<Line> cliffBoundaries = gameRep->GetCliffBoundaries();
		Vector<Loc> minerals = gameRep->GetVisibleMineralLocations();
		while(inputIndex < path.locations.size() - 1)
		{		
			double xAdjust, yAdjust;//The amount to move the raycast lines from the unit center
			Loc startLoc = outputPath.locations.back();
			Loc targetLoc = path.locations[inputIndex];		
			//TODO:Add a parameter for unit radius
			xAdjust = -(startLoc.y - targetLoc.y);
			yAdjust = startLoc.x - targetLoc.x;
			//Set the vector length to 3 (workers radius)
			real8 vectorSize = sqrt(xAdjust * xAdjust + yAdjust * yAdjust);
			xAdjust /= vectorSize / 3;
			yAdjust /= vectorSize / 3;
			//The adjusts are going to get changed to ints as that is how lines are stored, so if they are close to the next number then round them up
			if(xAdjust - (int)xAdjust > 0.75)
			xAdjust += 1.0;
			if(yAdjust - (int)yAdjust > 0.75)
			yAdjust += 1.0;
		
			Line rayCastLine1(startLoc.x-xAdjust,startLoc.y-yAdjust,targetLoc.x-xAdjust,targetLoc.y-yAdjust);
			Line rayCastLine2(startLoc.x+xAdjust,startLoc.y+yAdjust,targetLoc.x+xAdjust,targetLoc.y+yAdjust);
			Line rayCastLine3(startLoc.x,startLoc.y,targetLoc.x,targetLoc.y);
			bool clear = true;
			//Check if this line Intersects any cliffBoundaries
			for(uint4 i = 0; i < cliffBoundaries.size(); i++)
			{
				if(MiscFunctions::Intersects(rayCastLine1,cliffBoundaries[i])
				|| MiscFunctions::Intersects(rayCastLine2,cliffBoundaries[i])
				|| MiscFunctions::Intersects(rayCastLine3,cliffBoundaries[i]))
				{
					clear = false;				
					break;
				}
			}
			if(clear)//No point checking more if one has already been intersected
			{
				for(uint4 i = 0; i < minerals.size(); i++)
				{
					//Don't test a mineral intersection if it is the target
					if(mode == Constants::PATHFINDING_SMALL_TILES)
					{
						Loc minLoc = minerals[i];
						Loc targetLoc = path.locations.back();
						if(minLoc.x / 16 == targetLoc.x / 16 && minLoc.y / 16 == targetLoc.y / 16)
						continue;
					}
					if(MiscFunctions::Intersects(rayCastLine3,minerals[i],9))
					{
						clear = false;
						break;
					}
				}
			}
			if(clear && mode != Constants::PATHFINDING_IGNORE_CC)
			{			
				for(uint4 i = 0; i < buildingBoundaries.size(); i++)
				{				
					if(MiscFunctions::Intersects(rayCastLine1,buildingBoundaries[i])
					|| MiscFunctions::Intersects(rayCastLine2,buildingBoundaries[i])
					|| MiscFunctions::Intersects(rayCastLine3,buildingBoundaries[i]))
					{
						clear = false;
						break;
					}
				}
			}
			if(!clear)
			{			
				outputPath.AddLoc(path.locations[inputIndex-1],0);
			}
			inputIndex++;
		
		}
	}
	outputPath.AddLoc(path.locations.back(),0);	
	return outputPath;
}

void Pathfinding::UpdateMaps()
{	
	if(initialised)
	{
		smallTilesMap = gameRep->GetSmallMap();
		bigTilesMap = gameRep->GetBigMap();
	}
}
