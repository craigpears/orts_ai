#include <algorithm>
#include "Blackboard.H"
#include "Constants.H"
#include "MapTile.H"
#include "MiscFunctions.H"
#include "UpdatesHandler.H"

using namespace std;

void UpdatesHandler::Init(GameRepresentation* _gameRep, Pathfinding* _pathFinder)
{
	gameRep = _gameRep;
	pathFinder = _pathFinder;
}

void UpdatesHandler::ComputeActions(const GameChanges &changes, const Game& game)
{
	const sint4 cid = game.get_client_player();
	const Game::ObjCont &objs = game.get_objs(cid);
  
	bool updateBases = false;
	bool updateWP = false;
	bool updateMap = false;
  
	for(uint i = 0; i < changes.vanished_objs.size(); i++)
	{
		GameObj* gob = changes.vanished_objs[i]->get_GameObj();
		string objType = gob->bp_name();
		uint4 objectID = changes.vanished_objs[i]->get_obj_id(changes.vanished_objs[i]);  
		if(objType == "worker")
		gameRep->WorkerVanished(objectID);
		else if(objType == "marine")
		gameRep->MarineVanished(objectID);
		else if(objType == "tank")
		gameRep->TankVanished(objectID);
	}
  
	for(uint i = 0; i < changes.dead_objs.size(); i++)
	{
		uint4 objectID = changes.dead_objs[i]->get_obj_id(changes.dead_objs[i]);
		string objType = changes.dead_objs[i]->bp_name();
		if(objType == "mineral")
		{	
			gameRep->RemoveMineral(objectID);
			UpdateWorkerPlan();
		}
		else if(objType == "worker")
		gameRep->RemoveWorker(objectID);
		else if(objType == "marine")
		gameRep->RemoveMarine(objectID);
		else if(objType == "tank")
		gameRep->RemoveTank(objectID);
		else if(objType == "controlCenter" || objType == "barracks" || objType == "factory")
		gameRep->RemoveBase(objectID);
	}
  
  
	if(!changes.new_objs.empty())
	{
		FORALL(changes.new_objs,it)
		{
			GameObj* gob = (*it)->get_GameObj();
			string objType = gob->bp_name();
			uint4 unitID = game.get_cplayer_info().get_id(gob);
		
			//Search to see if it is your own unit
			bool ownUnit = false;
			FORALL(objs, it)
			{
				if(game.get_cplayer_info().get_id(*it) == unitID)
				{
					ownUnit = true;
					break;
				}
			}
			if(objType == "controlCenter" || objType == "barracks" || objType == "factory")
			updateBases = true;
			else if(objType == "worker")
			{
				gameRep->AddWorker(gob,unitID,ownUnit);
				updateWP = ownUnit || updateWP;
			}
			else if(objType == "marine")
			gameRep->AddMarine(gob,unitID,ownUnit);
			else if(objType == "tank")
			gameRep->AddTank(gob,unitID,ownUnit);
			else if(objType == "mineral")
			{
				pathFinder->AddMineral(gob,unitID);
				updateMap = true;
			}

		}
	}  
	
	if(updateWP)
	UpdateWorkerPlan();

	if(updateBases)
	UpdateBases(game);

	if(!changes.new_boundaries.empty() || updateMap)
	UpdateMap(game);
	
	pathFinder->UpdateObstacles(game);
}

/*
	If a new base has been discovered then this function adds all bases that are visible in the game state.  If they already exist they are ignored by the game representation
*/
void UpdatesHandler::UpdateBases(const Game& game)
{
	const sint4 cid = game.get_client_player();
	const Game::ObjCont &objs = game.get_objs(cid);
	//Reset Blackboard properties
	Blackboard::BARRACKS_BUILT = false;
	Blackboard::ENEMY_BASE_FOUND = false;
	Blackboard::FACTORY_BUILT = false; 
	//Add own bases
	FORALL (objs, it) {	
		GameObj * gob = (*it)->get_GameObj();      
		string objType = gob->bp_name();
		uint4 unitID = game.get_cplayer_info().get_id(gob);
      	if(objType == "controlCenter")
      	gameRep->AddBase(gob,unitID,Constants::BASE_CONTROL,true);  
      	if(objType == "barracks")
      	{
      		Blackboard::BARRACKS_BUILT = true;      		
      		gameRep->AddBase(gob,unitID,Constants::BASE_BARRACKS,true);
      	}  		
      	if(objType == "factory")
      	{
      		Blackboard::FACTORY_BUILT = true; 
      		cout << "Factory built" << endl;     		
      		gameRep->AddBase(gob,unitID,Constants::BASE_FACTORY,true);
      	}
	}
	
	//Add enemy bases
	const sint4 eid = (cid + 1) % 2;
	const Game::ObjCont &enemy_objs = game.get_objs(eid);
	FORALL (enemy_objs, it) {	
		GameObj * gob = (*it)->get_GameObj();      
		uint4 unitID = game.get_cplayer_info().get_id(gob);
		string objType = gob->bp_name();			
      	if(objType == "controlCenter")
      	gameRep->AddBase(gob,unitID,Constants::BASE_CONTROL,false);  
      	if(objType == "barracks")
      	gameRep->AddBase(gob,unitID,Constants::BASE_BARRACKS,false);  
      	if(objType == "factory")   		
      	gameRep->AddBase(gob,unitID,Constants::BASE_FACTORY,false);  	
      	if(objType == "controlCenter" || objType == "barracks" || objType == "factory")
			Blackboard::ENEMY_BASE_FOUND = true;
			
	}
}

void UpdatesHandler::UpdateMap(const Game& game)
{
	uint4 i,j;
	//The small map is used by workers as they have a small radius, the big map is used by military units and scouts
	Vector<Vector<MapTile> > smallTilesMap;
	Vector<Vector<MapTile> > bigTilesMap;
	
	MapTile mt;
	Vector<MapTile> vmt;
	for(i = 0; i < Constants::NO_TILES; i++)
	{
		bigTilesMap.push_back(vmt);
		for(j = 0; j < Constants::NO_TILES; j++)
		{
			bigTilesMap[i].push_back(mt);
		}
	}
	
	for(i = 0; i < Constants::NO_TILES * 2; i++)
	{
		smallTilesMap.push_back(vmt);
		for(j = 0; j < Constants::NO_TILES * 2; j++)
		{
			smallTilesMap[i].push_back(mt);
		}
	}
	
	
	Vector<Loc> minerals;
	Vector<Line> cliffBoundaries;
	
	//Set the tiles where buildings are as blocked
	Vector<Base>* myBases = gameRep->GetMyBases();
	Vector<Base>* enemyBases = gameRep->GetEnemyBases();
	Vector<Base>::iterator mbit = myBases->begin();
	while(mbit != myBases->end())
	{
		Vector<TileChange> changes = (*mbit).changes.tileChanges;
		Vector<TileChange>::iterator tcit = changes.begin();
		while(tcit != changes.end())
		{
			sint4 x,y;
			x = tcit->x;
			y = tcit->y;
			assert(x<128 && y<128);
			if(tcit->blocked)
			{
				smallTilesMap[x][y].SetBlocked();
				bigTilesMap[x/2][y/2].SetBlocked();
			}
			if(tcit->controlCenter)
			{
				smallTilesMap[x][y].SetControlCenter();
				bigTilesMap[x/2][y/2].SetControlCenter();
			}
			tcit++;
		}
		mbit++;
	}
	
	Vector<Base>::iterator ebit = enemyBases->begin();
	while(ebit != enemyBases->end())
	{
		Vector<TileChange> changes = (*ebit).changes.tileChanges;
		Vector<TileChange>::iterator tcit = changes.begin();
		while(tcit != changes.end())
		{
			sint4 x,y;
			x = tcit->x;
			y = tcit->y;
			assert(x<128 && y<128);
			if(tcit->blocked)
			{
				smallTilesMap[x][y].SetBlocked();
				bigTilesMap[x/2][y/2].SetBlocked();
			}
			if(tcit->isEnemyBase)
			{
				smallTilesMap[x][y].SetEnemyBase();
				bigTilesMap[x/2][y/2].SetEnemyBase();
			}
			tcit++;
		}
		ebit++;
	}
		
	const Game::ObjCont &np_objs = game.get_objs(game.get_player_num());//Non-player objects
	//TODO: do this once when they are created then load them similar to the buildings
	//Mark the tiles where minerals are
	FORALL (np_objs, it) {	
		
		GameObj * gob = (*it)->get_GameObj();      
		std::string objType = gob->bp_name();
      	if(objType == "mineral")   
      	{
      		//transform into tile co-ordinates
      		int mX = *gob->sod.x / 16;
      		int mY = *gob->sod.y / 16;
      		bigTilesMap[mX][mY].SetMineral();  
      		
      		assert((mX*2)+1 < 128);
      		assert((mY*2)+1 < 128);
      		
      		smallTilesMap[(mX*2)][(mY*2)].SetMineral(); 
      		smallTilesMap[(mX*2)+1][(mY*2)].SetMineral(); 
      		smallTilesMap[(mX*2)][(mY*2)+1].SetMineral(); 
      		smallTilesMap[(mX*2)+1][(mY*2)+1].SetMineral();  
      		
      		//Store the points for use in path smoothing
      		Loc loc(*gob->sod.x,*gob->sod.y);
      		minerals.push_back(loc);      			
      	}	
	}
	
	//Mark all blocked/partially blocked tiles	
    const Game::ObjCont &boundaries = game.get_boundaries();
    Vector<const GameObj*> rel_boundaries;
    //Get rid of all the edge-barriers
    FORALL(boundaries, it)
	{
		const GameObj * gob = (*it)->get_GameObj();     
		sint4 x1,x2,y1,y2;
		x1 = *gob->sod.x1;
		x2 = *gob->sod.x2;
		y1 = *gob->sod.y1;
		y2 = *gob->sod.y2;
		if(!(
			(x1 == x2 && x1 == 0) ||
			(x1 == x2 && x1 == Constants::NO_TILES * 16) ||	
			(y1 == y2 && y1 == 0) ||
			(y1 == y2 && y1 == Constants::NO_TILES * 16)
		))	
		rel_boundaries.push_back(gob);//If it isn't on one of these two borders then we are either interested in it or it won't do any harm, so add it	
	}
	Vector<Line> lines;
	//All the boundaries are of one tile length,combine them so that they are easier to work with.
	//Convert everything into lines
    for(i = 0; i < rel_boundaries.size(); i++)
    {
    	Line line;
    	line.x1 = *rel_boundaries[i]->sod.x1;
    	line.x2 = *rel_boundaries[i]->sod.x2;
    	line.y1 = *rel_boundaries[i]->sod.y1;
    	line.y2 = *rel_boundaries[i]->sod.y2;
    	if(line.x1 == line.x2)
    	line.type = 0;
    	else if(line.y1 == line.y2)
    	line.type = 1;
    	else if(line.y1 < line.y2)
    	line.type = 2;
    	else
    	line.type = 3;
    	
    	lines.push_back(line);
    }
    //Combine the lines together
    Vector<Line> finishedLines;
    do
    {
    	Line currentLine = lines.back();
    	lines.pop_back();
    	
    	
    	
    	for(i = 0; i < lines.size(); i++)
    	{
    		if(currentLine.type != lines[i].type)
    		continue;
    		
    		if(currentLine.x1 == lines[i].x1 && currentLine.y1 == lines[i].y1)
    		{
    			currentLine.x1 = lines[i].x2;
    			currentLine.y1 = lines[i].y2;
    			lines.erase(lines.begin()+i);
    			i--;    			
    		}
    		else if(currentLine.x1 == lines[i].x2 && currentLine.y1 == lines[i].y2)
    		{
    			currentLine.x1 = lines[i].x1;
    			currentLine.y1 = lines[i].y1;
    			lines.erase(lines.begin()+i);
    			i--; 
    		}
    		else if(currentLine.x2 == lines[i].x1 && currentLine.y2 == lines[i].y1)
    		{
    			currentLine.x2 = lines[i].x2;
    			currentLine.y2 = lines[i].y2;
    			lines.erase(lines.begin()+i);
    			i--; 
    		}
    		else if(currentLine.x2 == lines[i].x2 && currentLine.y2 == lines[i].y2)
    		{
    			currentLine.x2 = lines[i].x1;
    			currentLine.y2 = lines[i].y1;
    			lines.erase(lines.begin()+i);
    			i--; 
    		}
    	}
    	
    	finishedLines.push_back(currentLine);
    	cliffBoundaries.push_back(currentLine);
    }
    while(!lines.empty());
    
    //Get rid of all closed polygons
    do
    {	
    	sint4 x1,x2,y1,y2;
    	Line firstLine = finishedLines.back();
    	Vector<Line> linesInPolygon;
    	linesInPolygon.push_back(firstLine);
    	finishedLines.pop_back();
    	x1=firstLine.x1;
    	x2=firstLine.x2;
    	y1=firstLine.y1;
    	y2=firstLine.y2;
    	bool madeChange = true;
    	while(madeChange)
    	{
    		madeChange = false;
			for(i = 0; i < finishedLines.size(); i++)
			{
				//Check to see if this line connects with the start of the polygon
				if(x1 == finishedLines[i].x1 && y1 == finishedLines[i].y1)
				{
					x1 = finishedLines[i].x2;
					y1 = finishedLines[i].y2;
					madeChange = true;
					linesInPolygon.push_back(finishedLines[i]);
					finishedLines.erase(finishedLines.begin()+i);
					i--;    			
				}
				else if(x1 == finishedLines[i].x2 && y1 == finishedLines[i].y2)
				{
					x1 = finishedLines[i].x1;
					y1 = finishedLines[i].y1;
					madeChange = true;
					linesInPolygon.push_back(finishedLines[i]);
					finishedLines.erase(finishedLines.begin()+i);
					i--; 
				}
				//Check if this line connects with the end of the polygon
				else if(x2 == finishedLines[i].x1 && y2 == finishedLines[i].y1)
				{
					x2 = finishedLines[i].x2;
					y2 = finishedLines[i].y2;
					madeChange = true;
					linesInPolygon.push_back(finishedLines[i]);
					finishedLines.erase(finishedLines.begin()+i);
					i--; 
				}
				else if(x2 == finishedLines[i].x2 && y2 == finishedLines[i].y2)
				{
					x2 = finishedLines[i].x1;
					y2 = finishedLines[i].y1;
					madeChange = true;
					linesInPolygon.push_back(finishedLines[i]);
					finishedLines.erase(finishedLines.begin()+i);
					i--; 
				}
			}
    	}
    	//Check if the polygon has been closed
    	if(x1==x2 && y1==y2)
    	{
    		//cout << "Found a closed polygon with a point at: " << x1/16 << ", " << y1/16 << endl;
    		while(!linesInPolygon.empty())
    		{
    			lines.push_back(linesInPolygon.back());
    			linesInPolygon.pop_back();
    		}
    	}
    	else
    	{
    		//cout << "There was an unclosed polygon: " << x1/16 << ", " << y1/16 << " -> " << x2/16 << ", " << y2/16 << endl; 	
    		sint4 noBorderConnections = 0;
    		if(x1==0)
    		noBorderConnections++;
    		if(x2==0)
    		noBorderConnections++;
    		if(y1==0)
    		noBorderConnections++;
    		if(y2==0)
    		noBorderConnections++;
    		
    		//It wasn't closed, we need to add more lines
    		//If it is just one line we need to add  		
    		if(x1==x2 || y1==y2)
    		{    		    			
    			Line newLine;
    			newLine.x1=x1;
    			newLine.x2=x2;
    			newLine.y1=y1;
    			newLine.y2=y2;
    			lines.push_back(newLine);
    		}	
    		else if(noBorderConnections != 2)
    		{ 	
    			//This is a diagonal part of an undiscovered polygon, just complete it in a triangle until the rest is discovered
    			Line newLine1;
    			Line newLine2;
    			newLine1.x1=x1;
    			newLine1.y1=y1;
    			newLine2.x1=x2;
    			newLine2.y1=y2;
    			
    			uint4 yVal;
    			if(min(x1,x2) == x1)
    			yVal = y2;
    			else
    			yVal = y1;
    			newLine1.x2 = min(x1,x2);
    			newLine2.x2 = min(x1,x2);
    			newLine1.y2 = yVal;
    			newLine2.y2 = yVal;
    			lines.push_back(newLine1);
    			lines.push_back(newLine2);
    		}
    		else if(noBorderConnections == 2)//We need to add two lines
    		{ 
    			//TODO: problem occurs if only one border connection has been found and the other hasn't, it makes it look like there is a gap at the edge of the map
    			Line newLine1;
    			Line newLine2;
    			newLine1.x1=x1;
    			newLine1.y1=y1;
    			newLine2.x1=x2;
    			newLine2.y1=y2;
    			if(x1==0||x2==0)//It is on the left side of the map
    			{
    				newLine1.x2=0;
    				newLine2.x2=0;
    				if(y1==0||y2==0)//It is on the top edge
    				{
    					newLine1.y2=0;
    					newLine2.y2=0;
    				}
    				else//It is on the bottom edge
    				{
    					newLine1.y2=Constants::NO_TILES*16;
    					newLine2.y2=Constants::NO_TILES*16;
    				}
    			}
    			else//It is on the right side
    			{
    				newLine1.x2=Constants::NO_TILES*16;
    				newLine2.x2=Constants::NO_TILES*16;
    				if(y1==0||y2==0)//It is on the top edge
    				{
    					newLine1.y2=0;
    					newLine2.y2=0;
    				}
    				else//It is on the bottom edge
    				{
    					newLine1.y2=Constants::NO_TILES*16;
    					newLine2.y2=Constants::NO_TILES*16;
    				}
    			}
    			//cout << "Was determined to need two lines: " << newLine1.x1/16 << ", " << newLine1.y1/16 << " -> " << newLine1.x2/16 << ", " << newLine1.y2/16 << endl;
    			//cout << "And: " << newLine2.x1/16 << ", " << newLine2.y1/16 << " -> " << newLine2.x2/16 << ", " << newLine2.y2/16 << endl;
    			lines.push_back(newLine1);
    			lines.push_back(newLine2);
    		}
    		else
    		{	
    			assert(0);//Don't think its even possible to get to here right now
    			assert(linesInPolygon.size() == 1);
    			linesInPolygon.pop_back();
    		}
    		
    		while(!linesInPolygon.empty())
    		{
    			lines.push_back(linesInPolygon.back());
    			linesInPolygon.pop_back();
    		}
    		
    	}
    }
    while(!finishedLines.empty());
    
    //Make the small map
    Vector<sint4> intersect_points;
    sint4 yVal = 4;
    for(i = 0; i < Constants::NO_TILES * 2; i++, yVal+=8)//One scan line for each row
    {
    	intersect_points.clear();
    	//cout << "yVal=" << yVal << endl;
    	for(j = 0; j < lines.size(); j++)
    	{     
    		sint4 temp = MiscFunctions::Intersects(yVal, lines[j]);
    		if(temp+1)
    		intersect_points.push_back(temp);      		
    	}
		
    	//Sort the vector
    	sort(intersect_points.begin(),intersect_points.end());
    	
    	assert(intersect_points.size() % 2 == 0);
    	for(j = 0; j < intersect_points.size(); j++)
    	{
    		//cout << "intersect at " << intersect_points[j];
    		//cout << ", " << intersect_points[j+1] << endl;
    		
    		
    		uint4 intersect1 = intersect_points[j];
    		uint4 intersect2 = intersect_points[++j];
    		    		
    		assert(intersect1 <= 1024);
    		assert(intersect2 <= 1024);
    		
    		
    		if(intersect1==intersect2)
    		continue;
    		
    		//Check to see if this only occludes half of one of the tiles
    		if((intersect1 % 8) != 0)
    		{
    			//The first tile is partly blocked
    			//Mark it as partly blocked and round up the intersect
    			//cout << "Intersect one on a diagonal, marking " << intersect1/16 << " as partially blocked " << endl;
    			smallTilesMap[intersect1/8][i].SetPartiallyBlocked();
    			intersect1 += 4;
    		}
    		if((intersect2 % 8) != 0)
    		{
    			//The second tile is partly blocked
    			//Mark it as partly blocked and round down the intersect
    			//cout << "Intersect two on a diagonal, marking " << intersect2/16 << " as partially blocked " << endl;
    			smallTilesMap[intersect2/8][i].SetPartiallyBlocked();
    			intersect2 -= 4;
    		}
    		uint4 k;
    		for(k = intersect1/8; k < intersect2/8; k++)
    		{
    			//cout << "Blocked: " << k << endl;
    			assert(k < 128 && i < 128);
    			smallTilesMap[k][i].SetBlocked();
    		}
    	}
    }
    //Do the big map
    //TODO: base this off the small map to reduce time complexity
	intersect_points.clear();
    yVal = 8;
    for(i = 0; i < Constants::NO_TILES; i++, yVal+=16)//One scan line for each row
    {
    	intersect_points.clear();
    	//cout << "yVal=" << yVal << endl;
    	for(j = 0; j < lines.size(); j++)
    	{     
    		sint4 temp = MiscFunctions::Intersects(yVal, lines[j]);
    		if(temp+1)
    		intersect_points.push_back(temp);      		
    	}
		
    	//Sort the vector
    	sort(intersect_points.begin(),intersect_points.end());
    	
    	assert(intersect_points.size() % 2 == 0);
    	for(j = 0; j < intersect_points.size(); j++)
    	{
    		//cout << "intersect at " << intersect_points[j];
    		//cout << ", " << intersect_points[j+1] << endl;
    		
    		
    		uint4 intersect1 = intersect_points[j];
    		uint4 intersect2 = intersect_points[++j];
    		    		
    		assert(intersect1 <= 1024);
    		assert(intersect2 <= 1024);
    		
    		
    		if(intersect1==intersect2)
    		continue;
    		
    		//Check to see if this only occludes half of one of the tiles
    		if((intersect1 % 16) != 0)
    		{
    			//The first tile is partly blocked
    			//Mark it as partly blocked and round up the intersect
    			//cout << "Intersect one on a diagonal, marking " << intersect1/16 << " as partially blocked " << endl;
    			bigTilesMap[intersect1/16][i].SetPartiallyBlocked();
    			intersect1 += 8;
    		}
    		if((intersect2 % 16) != 0)
    		{
    			//The second tile is partly blocked
    			//Mark it as partly blocked and round down the intersect
    			//cout << "Intersect two on a diagonal, marking " << intersect2/16 << " as partially blocked " << endl;
    			bigTilesMap[intersect2/16][i].SetPartiallyBlocked();
    			intersect2 -= 8;
    		}
    		uint4 k;
    		for(k = intersect1/16; k < intersect2/16; k++)
    		{
    			//cout << "Blocked: " << k << endl;
    			assert(k < 64 && i < 64);
    			bigTilesMap[k][i].SetBlocked();
    		}
    	}
    }
    
	//Print out the view of the map
	//For some reason it needs to be flipped around, TODO: Find out why
	
	for(i = 0; i < Constants::NO_TILES+2; i++)
	cout << "#";
	cout << endl;
	for(i = 0; i < Constants::NO_TILES; i++)
	{
		cout << "#";
		for(j = 0; j < Constants::NO_TILES; j++)
		{
			if(bigTilesMap[j][i].IsControlCenter())
			cout << "C";			
			else if(bigTilesMap[j][i].IsMineral())
			cout << "M";
			else if(bigTilesMap[j][i].IsBlocked())
			cout << "#";
			else if(bigTilesMap[j][i].IsPartiallyBlocked())
			cout << "@";
			else
			cout << " ";
		}
		cout << "#" << endl;
	}
	for(i = 0; i < Constants::NO_TILES+2; i++)
	cout << "#";
	cout << endl;	
	
	gameRep->SetMaps(smallTilesMap, bigTilesMap);
	gameRep->SetCliffBoundaries(cliffBoundaries);
	gameRep->SetVisibleMineralLocations(minerals);
	pathFinder->UpdateMaps();
}

void UpdatesHandler::UpdateWorkerPlan()
{
	//cout << "Updating worker plan" << endl;
	Vector<Worker>* workers = gameRep->GetWorkers();	
	Vector<Mineral>* minerals = gameRep->GetMineralsList();
	uint4 maxWorkers;
	uint4 noWorkers = workers->size();
	uint4 noMinerals = minerals->size();
	if(noWorkers < noMinerals * 2)
	maxWorkers = 2;
	else
	maxWorkers = 3;
	Vector<Worker>::iterator wit = workers->begin();
	Vector<Mineral>::iterator it = minerals->begin();
	/*while(it != minerals->end())
	{
		cout << it->mineralID << " with " << it->assignedWorkers << " workers" << endl;
		it++;
	}*/
	while(wit != workers->end())
	{	
		if(!wit->assigned)
		{
			//Find the mineral that is closest to the control center that has free slots
			Mineral* closestMineral;
			Vector<Mineral>::iterator mit = minerals->begin();
			while(mit != minerals->end())
			{
				if(mit->assignedWorkers < maxWorkers && mit->distance > 1)//If distance == 1 then it isn't accessible
				{
					closestMineral = &(*mit);
					wit->assigned = true;
					break;
				}	
				mit++;		
			}
			closestMineral->assignedWorkers++;
			wit->mineralID = closestMineral->mineralID;	
			//cout << "assigning worker to " << wit->mineralID << endl;		
			//cout << "this mineral has " << closestMineral->assignedWorkers << " workers" << endl;
		}
		wit++;
	}
}
