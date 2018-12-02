#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "CDK.h"


// Print out a vector to the debug string, with a precision of two decimal places
void PrintVector(VECTOR3 Vec)
{
	sprintf_s(oapiDebugString(), 100, "%.3f  %.3f  %.3f", Vec.x, Vec.y, Vec.z);
}

// Same as above, except the vector contains angles
void PrintAngleVector(VECTOR3 Vec)
{
	sprintf_s(oapiDebugString(), 100, "%.3f  %.3f  %.3f", Degree(Vec.x), Degree(Vec.y), Degree(Vec.z));
}

// Print an integer
void PrintInt(int x)
{
	sprintf_s(oapiDebugString(), 100, "%d", x);	
}

// Print a real (double)
void PrintReal(double x)
{
	sprintf_s(oapiDebugString(), 100, "%.3f", x);	
}

void PrintReal(double x, double y)
{
	sprintf_s(oapiDebugString(), 100, "%.3f  %.3f", x, y);	
}

void PrintReal(double x, double y, double z)
{
	sprintf_s(oapiDebugString(), 100, "%.3f  %.3f  %.3f", x, y, z);	
}

// Prints a string
void PrintString(char *str)
{
	sprintf_s(oapiDebugString(),100,  "%s", str);	
}
