#include "Commander.H"
#include "Structures.H"

using namespace std;

bool Commander::CalculateAvoiding(Unit* u, Game::ObjCont objs, Game::ObjCont neutral_objs)
{	
	GameObj* gob = u->gameObj;
	FORALL(objs, it)
	{	
		GameObj* gob2 = (*it)->get_GameObj();
		sint4 distance = MiscFunctions::distance(*gob->sod.x,*gob->sod.y,*gob2->sod.x,*gob2->sod.y);
		if(distance < 8 && distance > 0)
		{
			MoveRandomly(u);
			return true;
		}
		
	}
	FORALL(neutral_objs, it)
	{	
		GameObj* gob2 = (*it)->get_GameObj();
		if(gob2->bp_name() == "sheep")
		{
			sint4 distance = MiscFunctions::distance(*gob->sod.x,*gob->sod.y,*gob2->sod.x,*gob2->sod.y);
			if(distance < 8 && distance > 0)
			{
				MoveRandomly(u);
				return true;
			}
		}
	}
	return false;
}

void Commander::MoveRandomly(Unit* u)
{
	GameObj* gob = u->gameObj;
	ScalarPoint t;
	gob->get_center(t.x, t.y);
	
	t.x += rand.ranged_sint4(-8, 8);
	t.y += rand.ranged_sint4(-8, 8);
	
	
	//Clamp the values to the map region TODO: this doesn't work
	t.x = max(0,t.x);
	t.x = min(64*16,t.x);
	t.y = max(0,t.y);
	t.y = min(64*16,t.y);
	Vector<sint4> params;
	params.push_back(t.x);
	params.push_back(t.y);
	gob->set_action("move", params);
	
	u->deviated = true;
	u->isStuck = false;
}

bool Commander::MoveToObject(Unit* unit,GameObj* gob2, sint4 mode)
{
	Loc targetLoc(*gob2->sod.x,*gob2->sod.y);	
  	return MoveToLocation(unit, targetLoc, mode);
}

bool Commander::MoveToLocation(Unit* unit, Loc targetLoc, sint4 mode)
{
	GameObj* gob = unit->gameObj;
	Loc unitLoc(*gob->sod.x,*gob->sod.y);
	//If the unit has had to deviate for some reason or has no path, repath
	if(unit->deviated || unit->path.locations.empty())
	{
		unit->path = pathFinder->FindPath(WCPos(unitLoc),WCPos(targetLoc),mode);
		unit->deviated = false;
	}
		
	//Remove the first location if the unit has reached it
	double distanceToFirstLoc = MiscFunctions::distance(unitLoc.x,unitLoc.y,unit->path.locations[0].x,unit->path.locations[0].y);
	if(distanceToFirstLoc <	1.0)
	unit->path.locations.erase(unit->path.locations.begin());
	
  	if(unit->path.locations.size() >= 1)
  	{
	  	Loc firstTargetLoc = unit->path.locations[0];
		Vector<sint4> params;		
		params.push_back(firstTargetLoc.x);
		params.push_back(firstTargetLoc.y);		
		gob->set_action("move", params);	
		return true;	
	}
	return false;
}
