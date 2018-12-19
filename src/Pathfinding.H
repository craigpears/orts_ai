#ifndef Pathfinding_H
#define Pathfinding_H

#include "Game.H"
#include "GameRepresentation.H"
#include "PathPriorityQueue.H"
#include "Structures.H"

struct VisitedTile
{
	uint4 x,y,cost;
	VisitedTile(uint4 _x,uint4 _y,uint4 _cost):x(_x),y(_y),cost(_cost){assert(x < 128 && y < 128);}
	VisitedTile(Loc loc,uint4 _cost):x(loc.x),y(loc.y),cost(_cost){assert(x < 128 && y < 128);}
	bool operator==(VisitedTile& otherTile){ return x == otherTile.x && y == otherTile.y;}
};

class Pathfinding
{
	public:
	Pathfinding(){initialised = false;}
	void Init(GameRepresentation* _gameRep);
	void AddMineral(GameObj* gob, uint4 mineralID);
	void UpdateMaps();
	void UpdateObstacles(const Game& game);
	Path FindPath(WCPos startPoint, WCPos target, uint4 mode);
	private:
	bool initialised;
	uint4 GetCost(Loc loc, Loc startLoc, uint4 mode);
	bool Traversable(uint4 x, uint4 y, uint4 tX, uint4 tY, uint4 mode);
	bool InClosedList(VisitedTile tile);
	Path SmoothPath(Path path, uint4 mode);
	Vector<VisitedTile> closedList;
	Vector<Vector<MapTile> > smallTilesMap;
	Vector<Vector<MapTile> > bigTilesMap;
	GameRepresentation* gameRep;
};

#endif
