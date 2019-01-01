#pragma once
#include "CDK.h"
#include "orbitermath.h"
#include "TrimState.h"

// The deadband for attitude adjustments.  These are just the "standard" deadband limits - a user
// can pass any deadband value to SetAttitude().  For example, a 5 degree deadband: 
// SetAttitude(TargetYaw, CurrentYaw, YAW, DEADBAND(Radians(5)))
typedef double DEADBAND;

const DEADBAND DB_FINE = Radians(0.1), 
			   DB_NORMAL = Radians(0.5), 
			   DB_COARSE = Radians(1);

class IAutopilot
{
public:
	virtual bool SetAttitude(
		const VECTOR3& targetAttitude,
		const VECTOR3& currentAttitude,
		DEADBAND deadBand,
		double deltaTime) = 0;

	virtual bool SetAttitudeInAxis(
		double targetAttitude,
		double currentAttitude,
		AXIS axis,
		DEADBAND deadBand,
		double deltaTime) = 0;

	virtual bool TrimRelativeVelocity(const VECTOR3& relativeVelocity, const TrimState& trimState) = 0;
	virtual void ShutdownAllEngines() = 0;
};