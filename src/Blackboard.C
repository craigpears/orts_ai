#include "Blackboard.H"
#include "Constants.H"

bool Blackboard::BARRACKS_BUILT = false;
bool Blackboard::FACTORY_BUILT = false;
bool Blackboard::ENEMY_BASE_FOUND = false;
uint4 Blackboard::UNIT_MODE = Constants::MODE_ATTACK_UNITS;
uint4 Blackboard::STRATEGY = Constants::STRATEGY_TECH_RUSH;
uint4 Blackboard::ENEMY_STRENGTH = 0;
