#include "Blackboard.H"
#include "Constants.H"
#include "GatherCommander.H"
#include "MiscFunctions.H"

using namespace std;

void GatherCommander::ComputeActions(const Game& game)
{
	const Game::ObjCont &neutral_objs = game.get_objs(game.get_player_num());
	const Game::ObjCont &objs = game.get_objs(game.get_client_player());
	Vector<Worker>* workers = gameRep->GetWorkers();
	Vector<Worker>::iterator it = workers->begin();
	
	while(it != workers->end())
	{
		Worker* worker = &(*it);
		GameObj * gob = worker->gameObj;

		if (gob && gob->sod.in_game && *gob->sod.x >= 0 && !gob->is_dead()) 
		{
			if(worker->state == Constants::STATE_BUILD_BARRACKS || worker->state == Constants::STATE_BUILD_FACTORY)
			{
				Loc buildLoc = gameRep->GetBarracksBuildSpot();
				real8 distanceToBuildSpot = MiscFunctions::distance(*(worker->gameObj)->sod.x,*(worker->gameObj)->sod.y,buildLoc.x,buildLoc.y);
				if((Blackboard::BARRACKS_BUILT && worker->state == Constants::STATE_BUILD_BARRACKS) ||
				   (Blackboard::FACTORY_BUILT && worker->state == Constants::STATE_BUILD_FACTORY))
				{
					cout << "Finished building, going to mine" << endl;
					worker->state = Constants::STATE_MOVE_TO_MINE;
				}
				else if(distanceToBuildSpot <= 10)
				{		  	
					Vector<sint4> args;
					args.push_back(buildLoc.x);
					args.push_back(buildLoc.y);
					if(worker->state == Constants::STATE_BUILD_BARRACKS)
					worker->gameObj->set_action("build_barracks", args);
					else
					worker->gameObj->set_action("build_factory", args);
					
					worker->state = Constants::STATE_MOVE_TO_MINE;
				}
				else
				{
					bool avoiding = CalculateAvoiding(worker,objs,neutral_objs);
					if(!avoiding)
					{
						MoveToLocation(worker,buildLoc,Constants::PATHFINDING_BIG_TILES);
						if(worker->isStuck)
						MoveRandomly(worker);
						worker->isStuck = true;
					}
				}
				worker->isStuck = *(worker->gameObj)->sod.speed == 0;
			}
			else //Worker is a miner
			{
				if (*gob->sod.speed == 0) 
				{      			
					Mineral* mineral = gameRep->GetMineral(worker->mineralID);
					if(worker->state == Constants::STATE_MOVE_TO_MINE)
					{					
						if(MiscFunctions::distance(*(mineral->gameObj)->sod.x,*(mineral->gameObj)->sod.y,*gob->sod.x,*gob->sod.y) <= 10)
						worker->state = Constants::STATE_MINE;		
						else
						{
							//See if it is too close to any other workers, if so move away
							bool avoiding = CalculateAvoiding(worker,objs,neutral_objs);				
							if(!avoiding)	
							{	  	
								MoveToObject(worker, mineral->gameObj,Constants::PATHFINDING_SMALL_TILES);
								if(worker->isStuck)
								MoveRandomly(worker);	
								worker->isStuck = true;
							}
						}
					}
					if(worker->state == Constants::STATE_MINE)
					{					
						if((int)gob->get_int("minerals") == 10)
						worker->state = Constants::STATE_MOVE_TO_BASE;
						else if(MiscFunctions::distance(*(mineral->gameObj)->sod.x,*(mineral->gameObj)->sod.y,*gob->sod.x,*gob->sod.y) > 10)
						worker->state = Constants::STATE_MOVE_TO_MINE;
						else
						{
							sint4 mineralID = game.get_cplayer_info().get_id(mineral->gameObj);
							Vector<sint4> params;
							params.push_back(mineralID);
							gob->component("pickaxe")->set_action("mine", params);
						}
					}
					if(worker->state == Constants::STATE_MOVE_TO_BASE)
					{					
						//At the control center?
						bool atCC = false;
						sint4 ccX = *gameRep->GetControlCenter()->sod.x;
						sint4 ccY = *gameRep->GetControlCenter()->sod.y;
						sint4 x1,x2,y1,y2,unitX,unitY;
						x1 = ccX - 34;
						x2 = ccX + 34;
						y1 = ccY - 34;
						y2 = ccY + 34;		
						unitX = *gob->sod.x;
						unitY = *gob->sod.y;  		

						if (unitX >= x1 && unitX <= x2) {
							if (unitY >= y1 && unitY <= y2) {
							atCC = true;
							}
						}

						if(atCC)
						{		  			
							Vector<sint4> params;
							sint4 ccID = game.get_cplayer_info().get_id(gameRep->GetControlCenter());
							params.push_back(ccID);
							gob->set_action("return_resources", params);
							worker->state = Constants::STATE_MOVE_TO_MINE;
							cout << "minerals: " <<game.get_cplayer_info().global_obj("player")->get_int("minerals") + 10 << endl;
						}
						else
						{
							//See if it is too close to any other workers, if so move away
							bool avoiding = CalculateAvoiding(worker,objs,neutral_objs);
							if(!avoiding)
							{
								MoveToObject(worker, gameRep->GetControlCenter(),Constants::PATHFINDING_IGNORE_CC);
								if(worker->isStuck)
								MoveRandomly(worker);					
								worker->isStuck = true;
							}
						}
					}
				}
				else if(worker->isStuck)
				worker->isStuck = false;

			}
		}
		it++;
	}
}
