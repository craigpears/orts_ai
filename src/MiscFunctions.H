#ifndef MiscFunctions_H
#define MiscFunctions_H

#include "Global.H"
#include "Structures.H"


class MiscFunctions
{
	public:
	static real8 distance(sint4 x1, sint4 y1, sint4 x2, sint4 y2); //Optimized distance method taken from tankbattle5
	static sint4 Intersects(sint4 yVal,Line line);
	static bool Intersects(Line line1, Line line2);
	static bool Intersects(Line line, Loc point, uint4 radius);
	static float XProduct(ScalarPoint v1, ScalarPoint v2);
	private:
	static const real8 preCalc[101];
};

#endif
