#pragma once
#include "IAutopilot.h"
#include "OrbiterSDK.h"

class Autopilot : public IAutopilot
{
public:
	Autopilot(VESSEL* spacecraft);
	virtual ~Autopilot();

	bool SetAttitude(
		double targetAttitude,
		double currentAttitude,
		AXIS axis,
		DEADBAND deadBand,
		double deltaTime) override;

private:
	VESSEL* m_spacecraft;
	VECTOR3 m_principleMomentOfInertia;
	double m_maxTorque[3];

	void InitializeMaxTorque();
	double GetThrusterGroupMaxTorque(THGROUP_TYPE thrusterGroup) const;
	double GetThusterTorque(THGROUP_TYPE thrusterGroup, int thrusterIndex) const;

	bool SetRotationRate(AXIS axis, double targetRotationRate, double deltaTime);
	bool NullRotationRate(AXIS axis, double deltaTime);

	bool IsWithinDeadband(double value, double deadBand) const;
	bool IsRotationRateZero(double rotationRate) const;
	void ShutdownRotationThrusters(AXIS axis);
	double GetTargetRotationRate(double deltaAngle)const;
	THGROUP_TYPE GetThrusterGroupForRotation(AXIS axis, double rotationRate) const;
};