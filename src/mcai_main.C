// $Id: sampleai_main.C 5301 2007-06-25 19:36:28Z lanctot $

// This is an ORTS file (c) Michael Buro, licensed under the GPL

// this is the main file of the sample ORTS AI client
// options are specified by the opt.put lines below
// to test it start the ORTS server (bin/orts) and then run bin/sampleai [options]

// SampleEventHandler.C contains the code for accessing the game state

#include "Global.H"
#include "GfxGlobal.H"
#include "GfxModule.H"
#include "GameStateModule.H"
#include "Options.H"
#include "ShutDown.H"
#include "SDLinit.H"
#include "Game.H"
#include "GUI.H"

#include "MCEventHandler.H"

using namespace std;

//===================================================================

static const sint4 GFX_X = 900;  // graphics window size
static const sint4 GFX_Y = 740;

//===================================================================

void add_options()
{
  Options opt("SampleClient");
  opt.put("-disp", "2d display");
  opt.put("-game1", "Disables all functionality other than resource gathering");
  opt.put("-attack", "AI will start playing in attack units mode");
  opt.put("-defend", "AI will start playing in defensive mode");
}

//===================================================================

// loop is called

class Looper : public LoopFunctor
{
	public:
	Looper(AIState * state_) : state(state_){}

	void loop() const 
	{

		// looks for server messages
		// if one or more arrived, calls handlers
		// if not, sleeps for one millisecond
		if (!state->gsm->recv_view()) SDL_Delay(1);
	}

	private:
	AIState * state;
};

//===================================================================

int main(int argc, char * * argv)
{
  signals_shut_down(true);

  // populate opt with options from various modules

  add_options();
  GameStateModule::Options::add();
  Game::Options::add();
  GUI::add_options();

  // process command line arguments
  if (Options::process(argc, argv, cerr, "")) exit(20);

  // initialize SDL
  SDLinit::video_init();
  SDLinit::network_init();

  // populate option objects with command line values
  // and create modules

  GameStateModule::Options gsmo;
  GameStateModule gsm(gsmo);
  
  // debug graphics

  GUI * gui = 0;
  bool disp;
  Options::get("-disp", disp);
  if (disp) {
    SDLinit::video_init();
    gui = new GUI;
  }

  // gives event handler access to the state
  AIState state(&gsm, 0, gui);

  MCEventHandler eh(state);
 
  // gsm events are handled in sev
  gsm.add_handler(&eh);

  // connect game state module to server
  if (!gsm.connect()) ERR("connection problems");

  if (gui) {
    gui->init(GFX_X, GFX_Y, gsm.get_game());
    gui->display();
  }

  Looper l(&state);  
  while (!state.quit) l.loop();

  return 0;
}
