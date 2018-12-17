#pragma once
#include "orbitermath.h"

enum AXIS
{
	PITCH,
	YAW,
	ROLL
};

// The deadband for attitude adjustments.  These are just the "standard" deadband limits - a user
// can pass any deadband value to SetAttitude().  For example, a 5 degree deadband: 
// SetAttitude(TargetYaw, CurrentYaw, YAW, DEADBAND(Radians(5)))
typedef double DEADBAND;

const DEADBAND DB_FINE = Radians(0.1), 
			   DB_NORMAL = Radians(0.5), 
			   DB_COARSE = Radians(1);

class IAutopilot
{
	virtual bool SetAttitude(
		double targetAttitude,
		double currentAttitude,
		AXIS axis,
		DEADBAND deadBand,
		double deltaTime) = 0;

	virtual bool NullRotationRate(AXIS axis) = 0;
};