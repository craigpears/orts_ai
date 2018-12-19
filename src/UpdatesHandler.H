#ifndef UpdatesHandler_H
#define UpdatesHandler_H

#include "Game.H"
#include "GameChanges.H"
#include "GameRepresentation.H"
#include "Pathfinding.H"

class UpdatesHandler
{
	public:
	void Init(GameRepresentation* _gameRep, Pathfinding* _pathFinder);
	void ComputeActions(const GameChanges &changes,const Game& game);
	void UpdateBases(const Game& game);
	void UpdateMap(const Game& game);	
	private:
	void UpdateWorkerPlan();
	GameRepresentation* gameRep;
	Pathfinding* pathFinder;
};

#endif
