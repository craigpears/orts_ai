#ifndef Commander_H
#define Commander_H

#include "Game.H"
#include "GameRepresentation.H"
#include "MiscFunctions.H"
#include "Pathfinding.H"
#include "Random.H"

class Commander
{
public:
	void InitCommander(GameRepresentation* _gameRep, Pathfinding* _pathFinder, Random _rand){gameRep = _gameRep; pathFinder = _pathFinder; rand = _rand;}
	bool CalculateAvoiding(Unit* u, Game::ObjCont objs, Game::ObjCont neutral_objs);
	bool MoveToObject(Unit* u,GameObj* gob2, sint4 mode);
	bool MoveToLocation(Unit* u,Loc targetLoc, sint4 mode);
	void MoveRandomly(Unit* u);
	protected:
	GameRepresentation* gameRep;
	Pathfinding* pathFinder;
	Random rand;
};

#endif
