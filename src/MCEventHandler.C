#include "GameStateModule.H"
#include "GfxModule.H"
#include "Game.H"
#include "GameChanges.H"
#include "GameTile.H"
#include "ServerObjData.H"
#include "Options.H"
#include "GUI.H"

#include "Blackboard.H"
#include "DrawOnTerrain2D.H"
#include "MCEventHandler.H"
#include "MiscFunctions.H"
#include "Strategy.H"

using namespace std;

MCEventHandler::MCEventHandler(AIState &state_) : state(state_)
{
	initialised = false;
}

/*
	This is the function that gets called every time a new frame has been calculated by the server.  It performs any initialisation functions that may be neccessary
	and calls the ComputeActions function which handles the changes
*/
bool MCEventHandler::handle_event(const Event& e)
{
	if(e.get_who() == GameStateModule::FROM) 
	{
		if (e.get_what() == GameStateModule::STOP_MSG) {
			state.quit = true; return true;
		}

		if (e.get_what() == GameStateModule::READ_ERROR_MSG) {
			state.quit = state.error = true; return true;
		}
	
		if (e.get_what() == GameStateModule::VIEW_MSG) 
		{
			/*
				If this is the first time that the function has been called, initialise all classes.
				Constructors can't be used because they require other things to be initialised within the ORTS engine first.
			*/
			const Game &game = state.gsm->get_game();
			if(!initialised)
			{						
				sint4 seed;
				Options::get("-seed", seed);
				if (!seed) seed = time(0);
				Options::set("-seed", seed);
				
				bool attack;
				Options::get("-attack", attack);
				if(attack)
				Blackboard::UNIT_MODE = Constants::MODE_ATTACK_UNITS;
				else
				Blackboard::UNIT_MODE = Constants::MODE_DEFEND;
				
				updatesHandler.Init(&gameRep, &pathFinder);
				updatesHandler.UpdateBases(game);
				updatesHandler.UpdateMap(game);
				pathFinder.Init(&gameRep);
				baseCommander.InitCommander(&gameRep, &pathFinder, seed);
				gatherCommander.InitCommander(&gameRep, &pathFinder, seed);				
				militaryCommander.InitCommander(&gameRep, &pathFinder, seed);
				scoutCommander.InitCommander(&gameRep, &pathFinder, seed);
				initialised = true;
			}
			ComputeActions();
			state.gsm->send_actions();
			
			// gfx and frame statistics

			if (state.gfxm) state.gfxm->process_changes();

			sint4 vf = game.get_view_frame();
			sint4 af = game.get_action_frame();
			sint4 sa = game.get_skipped_actions();
			sint4 ma = game.get_merged_actions();

			//if ((vf % 20) == 0) cout << "[frame " << vf << "]" << endl;

			if (vf != af || sa || ma > 1) 
			{
				cout << "frame " << vf;
				if (af < 0) cout << " [no action]";
				else if (vf != af) cout << " [behind by " << (vf - af) << " frame(s)]";
				if (sa) cout << " [skipped " << sa << "]";
				if (ma > 1) cout << " [merged " << ma << "]";
				cout << endl;
			}

			if (state.gfxm) 
			{
				// don't draw if we are far behind
				if (abs(vf-af) < 10) {
				state.gfxm->draw();
				state.just_drew = true;
				}
			}

			if (state.gui) {
				state.gui->event();
				//state.gui->display();
				if (state.gui->quit) exit(0);
			}

			// as a hack for flickering, gui->display() is called in dot.start()
			// so call everything that draws to the debug window between start() and end()
			DrawOnTerrain2D dot(state.gui);
			dot.start();
			dot.draw_line(-60, 10, -20, 10, Vec3<real4>(1.0, 1.0, 1.0));
			dot.draw_line(-60, 10, -65, 15, Vec3<real4>(1.0, 1.0, 1.0));
			dot.draw_line(-44, 10, -56, 40, Vec3<real4>(1.0, 1.0, 1.0));
			dot.draw_line(-28, 10, -36, 40, Vec3<real4>(1.0, 1.0, 1.0));
			dot.draw_line(-36, 40, -30, 34, Vec3<real4>(1.0, 1.0, 1.0));

			dot.draw_circle(-40, 25, 35, Vec3<real4>(1.0, 0.0, 1.0));

			dot.end();

			return true;
		}
	}

return false;
}

/*
	This function is called each frame to compute new actions.  It delegates responsibility for each area to the appropriate commander.
*/
void MCEventHandler::ComputeActions()
{
	const Game &game = state.gsm->get_game();	
	const GameChanges &changes = state.gsm->get_changes();
	
	//The updates handler deals with any new objects that have appeared and vanished or destroyed objects updating the game representation.
	updatesHandler.ComputeActions(changes, game);
	//Decide on what tactics are going to be
	Strategy::CalculateTactics(&gameRep);
	//The base commander is responsible for training units and constructing buildings
	baseCommander.ComputeActions(game);
	//The scout commander is responsible for directing those units that have been assigned to scout
	scoutCommander.ComputeActions();
	//The military commander directs military units
	militaryCommander.ComputeActions(game.tick());
	//The gather commander directs workers to construct any buildings that have been requested and to gather workers
	gatherCommander.ComputeActions(game);
	
}
	
