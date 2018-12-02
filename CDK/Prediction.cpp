//////////////////////////////////////////////////////////////////////////////////////////////
//
//	Prediction.cpp - Code to solve the prediction problem - given the position and velocity
//					 vectors at some initial time, what will the position and velocity vectors
//					 be at specified time in the future.  The method implemented here is
//					 described in Chapter 4 of "Fundementals of Astrodynamics" by Bate, Mueller,
//					 and White.
//
//	Copyright Chris Knestrick, 2003
//
//	For a full description of the copyright, please see the file Copyright.txt
///////////////////////////////////////////////////////////////////////////////////////////////

#include <stdlib.h>
#include "CDK.H"

static inline double CalcZ(double X, double a)
{
	return (pow(X, 2) / a);
}

static inline double CalcC(double Z)
{
	if (Z > 0) {
		return ((1 - cos(sqrt(Z))) / Z);
	} else if (Z < 0) {
		return ((1 - cosh(sqrt(-Z))) / Z);
	} else {
		return 0;
		// Z == 0  This needs to be added
	}
}

static inline double CalcS(double Z)
{
	if (Z > 0) {
		return ((sqrt(Z) - sin(sqrt(Z))) / sqrt(pow(Z, 3)));
	} else if (Z < 0) {
		return ((sinh(sqrt(-Z)) - sqrt(-Z)) / sqrt(pow(-Z, 3)));
	} else {	
		return 0;
		// Z == 0 This needs to be added
	}	
}

static inline double CalcF(double X, double C, double r)
{
	return (1 - ((pow(X, 2) / r) * C));
}

static inline double CalcG(double Time, double X, double S, double SqrtMu)
{
	return (Time - ((pow(X, 3) / SqrtMu) * S) );
}

static inline double CalcFDot(double SqrtMu, double r_0, double r, double X, double Z, double S)
{
	return ((SqrtMu / (r * r_0)) * X * (Z * S - 1));
}

static inline double CalcGDot(double X, double C, double r)
{
	return (1 - ((pow(X, 2) / r) * C));
}


// Required accuracy for the iterative computations
const double EPSILON = 0.0001;

// Iterative method to calculate new X and Z values for the specified time of flight
static inline void CalcXandZ(double &X, double &Z, const VECTOR3 Pos, const VECTOR3 Vel,
							 double a, const double Time, const double SqrtMu)
{
	const double MAX_ITERS = 10;
	double C, S, T, dTdX, DeltaTime, r = Mag(Pos), IterNum = 0;

	// These don't change over the iterations
	double RVMu = (Pos * Vel) / SqrtMu;		// Dot product of position and velocity divided 
											// by the squareroot of Mu	
	double OneRA = (1 - (r / a));			// One minus Pos over the semi-major axis

	C = CalcC(Z);
	S = CalcS(Z);
	T = ((RVMu * pow(X, 2) * C) +  (OneRA * pow(X, 3) * S) + (r * X)) / SqrtMu;

	DeltaTime = Time - T;

	// Iterate while the result isn't within tolerances
	while (fabs(DeltaTime) > EPSILON && IterNum++ < MAX_ITERS) {
		dTdX = ((pow(X, 2) * C) + (RVMu * X * (1 - Z * S)) + (r * (1 - Z * C))) / SqrtMu;

		X = X + (DeltaTime / dTdX);
		Z = CalcZ(X, a);
		
		C = CalcC(Z);
		S = CalcS(Z);
		T = ((RVMu * pow(X, 2) * C) +  (OneRA * pow(X, 3) * S) + (r * X)) / SqrtMu;

		DeltaTime = Time - T;

	}


}

// Given the specified position and velocity vectors for a given orbit, retuns the position
// and velocity vectors after a specified time
void PredictPosVelVectors(const VECTOR3 &Pos, const VECTOR3 &Vel, double a, double Mu, 
						  double Time, VECTOR3 &NewPos, VECTOR3 &NewVel)
{
	double SqrtMu = sqrt(Mu);

	// Variables for computation
	double X = (SqrtMu * Time) / a;					// Initial guesses for X
	double Z = CalcZ(X, a);							// and Z
	double C, S;									// C(Z) and S(Z)
	double F, FDot, G, GDot;

	// Calculate the X and Z for the specified time of flight
	CalcXandZ(X, Z, Pos, Vel, a, Time, SqrtMu);

	// Calculate C(Z) and S(Z)
	C = CalcC(Z);
	S = CalcS(Z);

	// Calculate the new position and velocity vectors
	F = CalcF(X, C, Mag(Pos));
	G = CalcG(Time, X, S, SqrtMu);
	NewPos = (Pos * F) + (Vel * G);

	FDot = CalcFDot(SqrtMu, Mag(Pos), Mag(NewPos), X, Z, S);
	GDot = CalcGDot(X, C, Mag(NewPos));

	NewVel = (Pos * FDot) + (Vel * GDot);
}