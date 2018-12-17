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

	bool NullRotationRate(AXIS axis) override;

private:
	VESSEL* m_spacecraft;
	VECTOR3 m_principleMomentOfInertia;
	double m_maxTorque[3];

	void InitializeMaxTorque();
	double GetThrusterGroupMaxTorque(THGROUP_TYPE thrusterGroup) const;
	double GetThusterTorque(THGROUP_TYPE thrusterGroup, int thrusterIndex) const;

	bool IsWithinDeadband(double value, double deadBand) const;
	bool IsRotationRateZero(double rotationRate) double;
	double GetTargetRotationRate(double deltaAngle)const ;
};