#ifndef Structures_H
#define Structures_H

#include "Constants.H"
#include "Game.H"
#include "GameObj.H"
#include "Global.H"

struct BTPos;
struct WCPos;

struct VectorPoint
{
	double x,y;
	VectorPoint(){}
	VectorPoint(double _x, double _y):x(_x), y(_y){}
	//Assigns the vector the normalised value of the parameters
	void SetNormalised(double _x, double _y)
	{
		double magnitude = sqrt(_x * _x + _y * _y);
		x = _x / magnitude;
		y = _y / magnitude;
	}
	
	void Normalise()
	{
		this->SetNormalised(x,y);
	}
	
	double GetMagnitude()
	{
		return sqrt(x*x + y*y);
	}
	
	VectorPoint operator-(const VectorPoint& other) const
	{
		VectorPoint newVector(x - other.x, y - other.y);
		return newVector;		
	}
	
	VectorPoint operator+(const VectorPoint& other) const
	{
		VectorPoint newVector(x + other.x, y + other.y);
		return newVector;
	}
	
	VectorPoint operator*(const double& scalar) const
	{
		VectorPoint newVector(x * scalar, y * scalar);
		return newVector;
	}
};

struct Loc
{
	uint4 x,y;
	Loc(uint4 _x,uint4 _y):x(_x), y(_y){}
	Loc(const Loc& loc):x(loc.x),y(loc.y){}
	void Set(uint4 _x,uint4 _y){x=_x;y=_y;}
	void Set(const Loc& loc){x = loc.x; y = loc.y;}
	bool operator!=(const Loc& other) const{return !(x/16 == other.x/16 && y/16 == other.y/16);}
	Loc(){}
};

struct STPos //Small Tiles
{
	Loc loc;
	
	STPos(uint4 x, uint4 y):loc(x,y)
	{
		assert(loc.x<128);
		assert(loc.y<128);
	}
	STPos(Loc _loc):loc(_loc)
	{
		assert(loc.x<128);
		assert(loc.y<128);
	}
};

struct BTPos //Big Tiles
{
	Loc loc;
	BTPos(uint4 x, uint4 y):loc(x,y)
	{
		assert(loc.x<64);
		assert(loc.y<64);
	}
	BTPos(Loc _loc):loc(_loc)
	{
		assert(loc.x<64);
		assert(loc.y<64);
	}
};

struct WCPos //World
{
	Loc loc;
	WCPos(uint4 x, uint4 y):loc(x,y)
	{
		assert(loc.x<1024);
		assert(loc.y<1024);
	}
	WCPos(Loc _loc):loc(_loc)
	{
		assert(loc.x<1024);
		assert(loc.y<1024);
	}
	const STPos ToSTPos()
	{
		STPos newPos(loc.x/8,loc.y/8);
		return newPos;
	}
	const BTPos ToBTPos()
	{
		BTPos newPos(loc.x/16,loc.y/16);
		return newPos;		
	}
};

struct Mineral
{
	GameObj* gameObj;
	uint4 distance, assignedWorkers;
	uint4 mineralID;
	Mineral(GameObj* _mineral, uint4 _distance, sint4 _mineralID): gameObj(_mineral), distance(_distance), assignedWorkers(0), mineralID(_mineralID){}
	Mineral(GameObj* _mineral): gameObj(_mineral), assignedWorkers(0){};
	bool operator<(const Mineral& other) const{return distance < other.distance;}
};

struct Line
{
	sint4 x1,x2,y1,y2;
	sint4 type;
	Line(){}
	Line(const Line& line):x1(line.x1),x2(line.x2),y1(line.y1),y2(line.y2),type(line.type){}
	Line(int _x1,int _y1,int _x2,int _y2):x1(_x1),x2(_x2), y1(_y1),y2(_y2) {}
	void Update(int _x1,int _y1,int _x2,int _y2){x1=_x1;x2=_x2;y1=_y1;y2=_y2;}
};

struct Path
{
	Vector<Loc> locations;
	Loc targetLoc;
	sint4 heuristicCost, costSoFar;
	
	sint4 GetCost(){ return heuristicCost + costSoFar;}
	sint4 GetHeuristicCost(){ return heuristicCost;}
	
	sint4 GetHeuristic(Loc startLoc, Loc targetLoc)
	{
		sint4 xDist = abs((sint4)startLoc.x - (sint4)targetLoc.x);
		sint4 yDist = abs((sint4)startLoc.y - (sint4)targetLoc.y);
		return xDist + yDist;
	}
	
	void AddLoc(Loc& newLoc, uint4 cost)
	{
		heuristicCost = GetHeuristic(newLoc,targetLoc);
		locations.push_back(newLoc);
		costSoFar += cost;
		return;
	}
	
	bool AtTarget()
	{
		return heuristicCost == 0;
	}
	
	Path(){}
	
	Path(Loc _startLoc, Loc _targetLoc):targetLoc(_targetLoc.x,_targetLoc.y), costSoFar(0)
	{
		heuristicCost = GetHeuristic(_startLoc, _targetLoc);
		locations.push_back(_startLoc);		
	}
	
	Path WorldCoords(uint4 mode)
	{
		if(mode == Constants::PATHFINDING_SMALL_TILES || mode == Constants::PATHFINDING_IGNORE_CC)
		{
			Loc startLoc((locations[0].x * 8) + 4,(locations[0].y * 8) + 4);
			Path transformedPath(startLoc,targetLoc);
			for(uint4 i = 1; i < locations.size(); i++)
			{	
				Loc newLoc((locations[i].x * 8) + 4,(locations[i].y * 8) + 4);
				transformedPath.AddLoc(newLoc,0);
			}
			return transformedPath;
		}
		else
		{
			Loc startLoc((locations[0].x * 16) + 8,(locations[0].y * 16) + 8);
			Path transformedPath(startLoc,targetLoc);
			for(uint4 i = 1; i < locations.size(); i++)
			{	
				Loc newLoc((locations[i].x * 16) + 8,(locations[i].y * 16) + 8);
				transformedPath.AddLoc(newLoc,0);
			}
			return transformedPath;
		}
	}
	
	
};

struct Unit
{
	uint4 unitID, objectID, state, targetID;
	bool isStuck, deviated;
	Path path;
	GameObj* gameObj;
	Unit(): isStuck(false), deviated(false){}
};

struct Marine: Unit{};
struct Tank: Unit{};

struct Worker: Unit
{		
	uint4 mineralID;//The id of the mineral this worker is assigned to
	bool assigned;
	Loc exploreTarget;
	Worker(): assigned(false), exploreTarget(0,0){}
};

struct TileChange
{
	sint4 x,y;//The tile position where the change is taking effect
	//Values will be true if this changes them to true, false otherwise
	bool partiallyBlocked;
	bool blocked;
	bool mineral;
	bool controlCenter;
	bool isEnemyBase;
	TileChange(sint4 _x, sint4 _y, bool pb, bool b, bool m, bool cc, bool own):x(_x),y(_y),partiallyBlocked(pb),blocked(b),mineral(m),controlCenter(cc), isEnemyBase(!own)
	{assert(x < 128 && y < 128);}
	TileChange(){}
};

struct MapChange
{
	Vector<TileChange> tileChanges;
	Vector<Line> barrierChanges;
};

struct Base: Unit
{
	MapChange changes;
	uint4 type;
	sint4 x,y;//The x and y values are in world co-ordinates
};



#endif
