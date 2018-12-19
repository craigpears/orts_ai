#include "MiscFunctions.H"

using namespace std;

//Pre-calculated square-root constants to speed up distance calculations. - from tankbattle5
const real8 MiscFunctions::preCalc[101] = {
1.00, 1.00, 1.00, 1.001, 1.001, 1.001, 1.002, 1.003, 1.004, 1.004, 1.005, 1.006, 1.008, 1.009, 1.01, 1.012, 1.013, 1.015, 1.017, 1.019, 1.021, 1.023, 1.025, 1.027, 1.029, 1.032, 1.034, 1.037, 1.04, 1.042, 1.045, 1.048, 1.051, 1.054, 1.058, 1.061, 1.064, 1.068, 1.071, 1.075, 1.079, 1.082, 1.086, 1.09, 1.094, 1.098, 1.103, 1.107, 1.111, 1.116, 1.12, 1.125, 1.129, 1.134, 1.139, 1.143, 1.148, 1.153, 1.158, 1.163, 1.169, 1.174, 1.179, 1.184, 1.19, 1.195, 1.201, 1.206, 1.212, 1.218, 1.223, 1.229, 1.235, 1.241, 1.247, 1.253, 1.259, 1.265, 1.271, 1.277, 1.284, 1.29, 1.296, 1.303, 1.309, 1.316, 1.322, 1.329, 1.335, 1.342, 1.349, 1.355, 1.362, 1.369, 1.376, 1.383, 1.39, 1.397, 1.404, 1.411, 1.414
};

/*
	Returns the distance between two points (x1,y1) and (x2,y2).
	Taken from tankbattle5
*/
real8 MiscFunctions::distance(sint4 x1, sint4 y1, sint4 x2, sint4 y2) {
	//Correct calculation. Requires lot of CPU time.
	/*real8 ASquare = ((real8)x2 - (real8)x1) * ((real8)x2 - (real8)x1);
	real8 BSquare = ((real8)y2 - (real8)y1) * ((real8)y2 - (real8)y1);
	real8 D = sqrt(ASquare + BSquare);
	return D;*/

	//Optimized version. Requires less CPU time.
	real8 lX = (real8)abs(x2 - x1);
	real8 lY = (real8)abs(y2 - y1);
	real8 max = lX;
	real8 min = lY;
	if (lY > max) {
		max = lY;
		min = lX;
	}

	int t = (int)(min / max * (real8)100);
	real8 appD = max * preCalc[t];
	return appD;
}

sint4 MiscFunctions::Intersects(sint4 yVal,Line line)
{
	sint4 x1,y1,x2,y2;
	y1 = line.y1;
	y2 = line.y2;
	x1 = line.x1;
	x2 = line.x2;
	
	//Are both ends of the line above/below the scan-line?
	if((y1 > yVal && y2 > yVal) ||
	   (y1 < yVal && y2 < yVal))
	{
		return -1;
	}
	
	//cout << "Line intersects" << endl;
	//If not, then it must intersect somewhere
	
	//Find the gradient
	sint4 intersect;
	//Catch the special cases
	if(x1==x2)
	{
		//Gradient is infinite
		intersect = x1;
	}
	else
	{
		real8 m = (y2-y1) / (x2-x1);
		if(m==0)
		return -1;
		//Find the constant, c=y-mx
		real8 c = y1 - (m * x1);
		//Find the x intersect, x = y/m
		intersect = (yVal - c) / m;
	}
	
	return intersect;
	
} 

bool MiscFunctions::Intersects(Line line1, Line line2)
{
    //Quick rejection
    //Check if the bounding boxes intersect
    sint4 minX1 = min(line1.x1,line1.x2);
    sint4 maxX1 = max(line1.x1,line1.x2);
    sint4 minX2 = min(line2.x1,line2.x2);
    sint4 maxX2 = max(line2.x1,line2.x2);
    
    sint4 minY1 = min(line1.y1,line1.y2);
    sint4 maxY1 = max(line1.y1,line1.y2);
    sint4 minY2 = min(line2.y1,line2.y2);
    sint4 maxY2 = max(line2.y1,line2.y2);
    
    //If any of these are true, they don't intersect.  Otherwise they might
    if(minX1 > maxX2) return false;
    if(minY1 > maxY2) return false;
    if(maxX1 < minX2) return false;
    if(maxY1 < minY2) return false;
    
    
    //Straddling
    //Else compare the cross products
    
	ScalarPoint v1(line2.x1 - line1.x1, line2.y1 - line1.y1);
	ScalarPoint v2(line1.x2 - line2.x2, line1.y2 - line2.y2);
	ScalarPoint v3(line2.x2 - line1.x1, line2.y2 - line1.y1);
	ScalarPoint v4(line1.x2 - line1.x1, line1.y2 - line1.y1);
	
	if(XProduct(v1,v2) < 0 && XProduct(v3,v4) < 0) return false;
	if(XProduct(v1,v2) > 0 && XProduct(v3,v4) > 0) return false;
	
	return true;

    
    //Taken from: http://www.math.niu.edu/~rusin/known-math/95/line_segs
}

//Tests intersection between a line and a sphere
bool MiscFunctions::Intersects(Line line, Loc point, uint4 radius)
{
	sint4 a,b,c,u,x1,x2,x3,y1,y2,y3;
	x1 = line.x1;
	x2 = line.x2;
	x3 = point.x;
	y1 = line.y1;
	y2 = line.y2;
	y3 = point.y;
	a = (x2 - x1)*(x2 - x1) + (y2 - y1)*(y2 - y1);
	b = 2 * ((x2 - x1)*(x1 - x3) + (y2 - y1)*(y1 - y3));
	c = x3 * x3 + y3 * y3 + x1 * x1 + y1 * y1 - 2 * (x3 * x1 + y3 * y1) - radius * radius;
	u = b * b - 4 * a * c;
	return u >= 0;
	
	//Taken from: http://local.wasp.uwa.edu.au/~pbourke/geometry/sphereline/
}

float MiscFunctions::XProduct(ScalarPoint v1, ScalarPoint v2)
{
	return (v1.x * v2.y) - (v1.y * v2.x);
}

