//////////////////////////////////////////////////////////////////////////////////////////////
//
//	OrbiterMath.h - General math operation that I found useful for Orbiter programming
//
//	Copyright Chris Knestrick, 2002
//
//	For a full description of the copyright, please see the file Copyright.txt
///////////////////////////////////////////////////////////////////////////////////////////////

#ifndef ORBITER_MATH_H
#define ORBITER_MATH_H
#include <math.h>
#include <stdio.h>
#include "Orbitersdk.h"


// Prototypes
void MultiplyByMatrix(const VECTOR3 &Initial, const MATRIX3 &RotMatrix, VECTOR3 &Result);
void RotateVector(const VECTOR3 &Initial, const VECTOR3 &Angles, VECTOR3 &Result);
double CalcSphericalDistance(VECTOR3 Pos1, VECTOR3 Pos2, double Radius);
double CalcSphericalBearing(VECTOR3 Pos1, VECTOR3 Pos2);
void CalcOffsetLatLong(VECTOR3 Pos, VECTOR3& Result, double Distance, double Angle);
double AddAngles(double Initial, double Angle);
VECTOR3 GetPitchYawRoll(VECTOR3 PitchYaw, VECTOR3 Roll);
void PrintPitchYawRoll(VECTOR3 PitchYaw, VECTOR3 Roll);
double CalcTimeOfFlight();

// Null vector
const VECTOR3 NULL_VECTOR = { 0.0, 0.0, 0.0 };

// Returns the rotation matrix for a rotation of a given angle around the X axis (Pitch)
void inline GetRotMatrixX(double Angle, MATRIX3 &RotMatrixX)
{
	RotMatrixX.m11 = 1;
	RotMatrixX.m12 = 0;
	RotMatrixX.m13 = 0;
	RotMatrixX.m21 = 0;
	RotMatrixX.m22 = cos(Angle);
	RotMatrixX.m23 = sin(Angle);
	RotMatrixX.m31 = 0;
	RotMatrixX.m32 = -sin(Angle);
	RotMatrixX.m33 = cos(Angle);
}

// Returns the rotation matrix for a rotation of a given angle around the Y axis (Yaw)
void inline GetRotMatrixY(double Angle, MATRIX3 &RotMatrixY)
{
	RotMatrixY.m11 = cos(Angle);
	RotMatrixY.m12 = 0;
	RotMatrixY.m13 = -sin(Angle);
	RotMatrixY.m21 = 0;
	RotMatrixY.m22 = 1;
	RotMatrixY.m23 = 0;
	RotMatrixY.m31 = sin(Angle);
	RotMatrixY.m32 = 0;
	RotMatrixY.m33 = cos(Angle);


}

// Returns the rotation matrix for a rotation of a given angle around the Z axis (Roll)
void inline GetRotMatrixZ(double Angle, MATRIX3 &RotMatrixZ)
{
	RotMatrixZ.m11 = cos(Angle);
	RotMatrixZ.m12 = sin(Angle);
	RotMatrixZ.m13 = 0;
	RotMatrixZ.m21 = -sin(Angle);
	RotMatrixZ.m22 = cos(Angle);
	RotMatrixZ.m23 = 0;
	RotMatrixZ.m31 = 0;
	RotMatrixZ.m32 = 0;
	RotMatrixZ.m33 = 1;
}

// Note that some of these vector operations have actually been added into the new version of 
// Orbiter.


// Divide a vector by an integer scalar
inline VECTOR3 operator/ (const VECTOR3 &a, int b)
{
	VECTOR3 c;
	c.x = a.x / b;
	c.y = a.y / b;
	c.z = a.z / b;

	return c;
}

// This is the dot product
inline double operator* (const VECTOR3 &a, const VECTOR3 &b)
{
	VECTOR3 c;

	c.x = a.x * b.x;
	c.y = a.y * b.y;
	c.z = a.z * b.z;

	return (c.x + c.y + c.z);
}


// Are the vectors equal?
inline double operator== (const VECTOR3 &a, const VECTOR3 &b)
{
	return ((a.x == b.x) && (a.y == b.y) && (a.z == b.z));
}

// Returns the cross product of two vectors
inline VECTOR3 CrossProduct(const VECTOR3 &a, const VECTOR3 &b)
{
	VECTOR3 c;

	c.x = (a.y * b.z - a.z * b.y);
	c.y = (a.z * b.x - a.x * b.z);
	c.z = (a.x * b.y - a.y * b.x);

	return c;
}

// Magnitude of a vector
inline double Mag(const VECTOR3 &a)
{
	return (sqrt(pow(a.x, 2) + pow(a.y, 2) + pow(a.z, 2)));
}

// Horizontal magnitude (z and x components of the vector)
inline double HMag(const VECTOR3 &a)
{
	return (sqrt(pow(a.x, 2) + pow(a.z, 2)));
}

// Normalize a vector
inline VECTOR3 Normalize(const VECTOR3 &a)
{
	return (a/Mag(a));
}

// Convert Radians --> Degrees
static inline double Degree(double Rad)
{
	return DEG * Rad;
}

// Convert Degrees --> Radians
static inline double Radians(double Deg)
{
	return RAD * Deg;
}

// Convert km --> m
static inline double M(double km)
{
	return (km * 1000.0);
}

// Convert m --> km
static inline double KM(double m)
{
	return (m / 1000.0);
}

// Scale a value before it's displayed.  The buffer must be at least 10 bytes.
inline void ScaleOutput(char *Buffer, double Value)
{
	if (fabs(Value) < 1000) {
		sprintf_s(Buffer, 10, "%.2f", Value);
	} else if (fabs(Value) < 1000000) {
		sprintf_s(Buffer, 10, "%.2fk", Value/1000);
	} else if (fabs(Value) < 1495978707) {		// 1 A.U. / 100 
		sprintf_s(Buffer, 10, "%.2fM", Value/1000000);
	} else {
		sprintf_s(Buffer, 10, "%.2fAU", Value/1495978707);
	}

}
#endif