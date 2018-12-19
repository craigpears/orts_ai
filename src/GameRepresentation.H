#ifndef GameRepresentation_H
#define GameRepresentation_H

#include "Constants.H"
#include "Game.H"
#include "GameObj.H"
#include "MapTile.H"
#include "Structures.H"

class GameRepresentation
{
	public:
	GameRepresentation():noExplorers(0){}
	
	void SetMaps(Vector<Vector<MapTile> > smallTMap,Vector<Vector<MapTile> > bigTMap);
	void SetCliffBoundaries(Vector<Line> boundaries);
	void SetVisibleMineralLocations(Vector<Loc> locations);
	Loc GetBarracksBuildSpot();
	Vector<Vector<MapTile> > GetSmallMap();
	Vector<Vector<MapTile> > GetBigMap();
	Vector<Line> GetCliffBoundaries();
	Vector<Loc> GetVisibleMineralLocations();
	
	void AddBase(GameObj* gob, uint4 unitID, uint4 type, bool own);
	void AddMineral(Mineral min);
	void AddWorker(GameObj* gob, uint4 unitID, bool own);
	void AddMarine(GameObj* gob, uint4 unitID, bool own);
	void AddTank(GameObj* gob, uint4 unitID, bool own);
	void CheckStrength();
	void RemoveMineral(uint4 objectID);
	void RemoveBase(uint4 objectID);
	void RemoveWorker(uint4 objectID);
	void RemoveMarine(uint4 objectID);
	void RemoveTank(uint4 objectID);
		
	void WorkerVanished(uint4 objectID);
	void MarineVanished(uint4 objectID);
	void TankVanished(uint4 objectID);

	Worker* GetWorker(uint4 workerID);
	Marine* GetMarine(uint4 unitID);
	Mineral* GetMineral(uint4 mineralID);
	GameObj* GetControlCenter();	
	GameObj* GetBarracks();
	GameObj* GetFactory();
	Loc GetEnemyBaseLoc();

	Vector<Base>* GetMyBases();
	Vector<Base>* GetEnemyBases();
	Vector<Mineral>* GetMineralsList();
	Vector<Worker>* GetWorkers();
	Vector<Worker>* GetEnemyWorkers();
	Vector<Marine>* GetMarines();
	Vector<Marine>* GetEnemyMarines();
	Vector<Tank>* GetTanks();
	Vector<Tank>* GetEnemyTanks();
	Vector<Line> GetBuildingBoundaries();
	private:
	Vector<Vector<MapTile> > smallTilesMap;
	Vector<Vector<MapTile> > bigTilesMap;
	Vector<Line> cliffBoundaries;
	Vector<Loc> visibleMineralLocations;
	
	Vector<Worker> workers;
	Vector<Worker> enemyWorkers;
	Vector<Marine> marines;
	Vector<Marine> enemyMarines;
	Vector<Tank> tanks;
	Vector<Tank> enemyTanks;
	Vector<Mineral> minerals;
	Vector<Base> myBases;
	Vector<Base> enemyBases;
	uint4 noExplorers;
};

#endif
