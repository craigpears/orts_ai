#ifndef MCEventHandler_H
#define MCEventHandler_H

#include "BaseCommander.H"
#include "EventHandler.H"
#include "GameRepresentation.H"
#include "GatherCommander.H"
#include "MilitaryCommander.H"
#include "Random.H"
#include "ScoutCommander.H"
#include "UpdatesHandler.H"

class GameStateModule;
class GfxModule;
class GUI;

// This class gives the event handler access to all game and gfx data

struct AIState {

  GameStateModule * gsm;
  GfxModule * gfxm; // !=0 : gfx used
  GUI * gui;  // !=0 : debug gfx used
  bool just_drew;  // just drew a gfx frame
  bool quit;       // true => quit game
  bool error;      // true <=> error occurred

  AIState(GameStateModule * gsm_, GfxModule * gfxm_, GUI * gui_) :
    gsm(gsm_), gfxm(gfxm_), gui(gui_)
  {
    just_drew = quit = error = false;
  }

};

class MCEventHandler : public EventHandler {

public:
  AIState &state;
  MCEventHandler(AIState &state_);
  bool handle_event(const Event &e);

private:
  bool initialised;
  void ComputeActions();
  GameRepresentation gameRep;  
  UpdatesHandler updatesHandler;
  BaseCommander baseCommander;
  GatherCommander gatherCommander;
  MilitaryCommander militaryCommander;
  Pathfinding pathFinder;
  ScoutCommander scoutCommander;  
};

#endif
