#ifndef CDK_H
#define CDK_H

// Include other necessary files - user only needs to include this one
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "Elements.h"
#include "GravBodyData.h"
#include "OrbiterMath.h"
#include "Prediction.h"
#include "EI.h"
#include "Orbitersdk.h"

enum AXIS
{
	PITCH,
	YAW,
	ROLL
};

#define DimensionOf(array) (sizeof(array)/sizeof(array[0]))

// Prototypes for general functions
//void ScaleOutput(char *Buffer, double Value);
void PrintVector(VECTOR3 Vec);
void PrintAngleVector(VECTOR3 Vec);
void PrintInt(int x);
void PrintReal(double x);
void PrintReal(double x, double y);
void PrintReal(double x, double y, double z);
void PrintString(char *str);

#endif