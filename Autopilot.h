#pragma once
#include "IAutopilot.h"
#include "OrbiterSDK.h"
#include <iostream>
#include <fstream>
#include <map>

class Autopilot : public IAutopilot
{
public:
	Autopilot(VESSEL* spacecraft);
	virtual ~Autopilot();

	bool SetAttitude(
		const VECTOR3& targetAttitude,
		const VECTOR3& currentAttitude,
		DEADBAND deadBand,
		double deltaTime) override;

	bool SetAttitudeInAxis(
		double targetAttitude,
		double currentAttitude,
		AXIS axis,
		DEADBAND deadBand,
		double deltaTime) override;

	void ShutdownAllEngines() override;

private:
	VESSEL* m_spacecraft;
	VECTOR3 m_principleMomentOfInertia;
	double m_maxTorque[3];
	mutable std::ofstream m_log;

	void InitializeMaxTorque();
	double GetThrusterGroupMaxTorque(THGROUP_TYPE thrusterGroup) const;
	double GetThusterTorque(THGROUP_TYPE thrusterGroup, int thrusterIndex) const;

	bool SetRotationRateInAxis(AXIS axis, double targetRotationRate, double deltaTime);
	bool NullRotationRateInAxis(AXIS axis, double deltaTime);

	bool IsDeltaValueWithinDeadband(double deltaValue, double deadband) const;
	bool IsRotationRateZero(double rotationRate) const;
	double GetRotationRateDeadband(double targetRotationRate) const;
	void ShutdownRotationThrustersInAxis(AXIS axis);
	double GetTargetRotationRate(double deltaAngle) const;
	THGROUP_TYPE GetThrusterGroupForRotationInAxis(AXIS axis, double rotationRate) const;
};