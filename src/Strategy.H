#ifndef Strategy_H
#define Strategy_H

#include "GameRepresentation.H"
#include "Global.H"

class Strategy
{
	public:
		Strategy();
		Vector<uint4> GetPriorities(uint4 strategy);
		static void CalculateTactics(GameRepresentation* gameRep);
	private:
		Vector<uint4> balancedPriorities;
		Vector<uint4> turtlePriorities;
		Vector<uint4> rushPriorities;
		Vector<uint4> techRushPriorities;
		Vector<uint4> steamrollPriorities;		
};

#endif
