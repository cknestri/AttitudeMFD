#include "Autopilot.h"

const double RATE_MAX = Radians(2.0);
const double DEADBAND_MAX = Radians(10.0);
const double RATE_HIGH = Radians(1.0);
const double DEADBAND_HIGH = Radians(3.0);
const double RATE_MID = Radians(0.5);
const double DEADBAND_MID = Radians(1.0);
const double RATE_LOW = Radians(0.25);
const double DEADBAND_LOW = Radians(0.15);
const double RATE_FINE = Radians(0.01);

const double RATE_NULL = Radians(0.0001);

Autopilot::Autopilot(VESSEL* spacecraft)
	: m_spacecraft(spacecraft)
{
	m_spacecraft->GetPMI(m_principleMomentOfInertia);
	InitializeMaxTorque();
}

Autopilot::~Autopilot()
{
}

void Autopilot::InitializeMaxTorque()
{

}

double Autopilot::GetThrusterGroupMaxTorque(THGROUP_TYPE thrusterGroup) const
{
	double maxTorque = 0.0;

	int totalThrusters = m_spacecraft->GetGroupThrusterCount(thrusterGroup);

	for (int thrusterIndex = 0; thrusterIndex < totalThrusters; thrusterIndex++)
	{
		maxTorque += GetThusterTorque(thrusterGroup, thrusterIndex);
	}

	return maxTorque;
}

double Autopilot::GetThusterTorque(THGROUP_TYPE thrusterGroup, int thrusterIndex) const
{
	THRUSTER_HANDLE thruster = m_spacecraft->GetGroupThruster(thrusterGroup, thrusterIndex);

	VECTOR3 thrusterPosition;
	m_spacecraft->GetThrusterRef(thruster, thrusterPosition);

	return (m_spacecraft->GetThrusterMax(thruster) * Mag(thrusterPosition));
}

bool Autopilot::SetAttitude(
		double targetAttitude,
		double currentAttitude,
		AXIS axis,
		DEADBAND deadBand
		double deltaTime)
{
	double targetRotationRate = 0.0;			
	double rotationRateDeadband = 0.0;	
	double Thrust;			
	double Level;					

	double DeltaRate;		
	double deltaAngle = targetAttitude - currentAttitude;

	// Get State
	double Mass = m_spacecraft->GetMass();
	VESSELSTATUS status = m_spacecreat->GetStatus(status);
	double currentRotationRate = status.vrot.data[axis];

	// Let's take care of the good case first :-)
	if (IsWithinDeadBand(deltaAngle, deadBand))
	{
		if (IsRotationRateZero(currentRotationRate))
		{
			m_spacecraft->SetAttitudeRotLevel(axis, 0);
			return true;
		} 
	
		return (NullRate(axis));
	}

	targetRotationRate = GetTargetRotationRate(deltaAngle);

	rotationRateDeadband = min(targetRotationRate / 2, Radians(0.01/*2*/));
	
	if (DeltaAngle < 0 ) {
		targetRotationRate = -targetRotationRate;
		rotationRateDeadband = -rotationRateDeadband;
	}

	double deltaRotationRate = targetRotationRate - currentRotationRate;
	
	if (DeltaAngle > 0) {
		if (DeltaRate > rotationRateDeadband) {
			Thrust = (Mass * PMI.data[axis] * DeltaRate) / deltaTime;
			Level = min((Thrust/MaxThrust), 1);
			m_spacecraft->SetAttitudeRotLevel(axis, Level);
		} else if (DeltaRate < -rotationRateDeadband) { 
			Thrust = (Mass * PMI.data[axis] * DeltaRate) / deltaTime;
			Level = max((Thrust/MaxThrust), -1);
			m_spacecraft->SetAttitudeRotLevel(axis, Level);
		} else {
			m_spacecraft->SetAttitudeRotLevel(axis, 0);
		}
	} else {
		if (DeltaRate < rotationRateDeadband) {
			Thrust = (Mass * PMI.data[axis] * DeltaRate) / deltaTime;
			Level = max((Thrust/MaxThrust), -1);
			m_spacecraft->SetAttitudeRotLevel(axis, Level);
		} else if (DeltaRate > -rotationRateDeadband) { 
			Thrust = (Mass * PMI.data[axis] * DeltaRate) / deltaTime;
			Level = min((Thrust/MaxThrust), 1);
			m_spacecraft->SetAttitudeRotLevel(axis, Level);
		} else {
			m_spacecraft->SetAttitudeRotLevel(axis, 0);
		}
	}
	return false;
}

bool Autopilot::NullRotationRate(AXIS axis)
{
	VESSELSTATUS status;
	m_spacecraft->GetStatus(status);

	double rotationRate = Status.vrot.data[axis];

	double RateDeadband = Radians(0.001), Thrust, Level,
			Mass = Vessel->GetMass(),
			MaxThrust = Vessel->GetMaxThrust(ENGINE_ATTITUDE),
			Size = Vessel->GetSize();

	if (IsRotationRateZero(rotationRate)) {
		Vessel->SetAttitudeRotLevel(Axis, 0.0);
		return true;
	}

	Thrust = -(Mass * PMI.data[Axis] * Rate) / (Size);	
	Level = min((Thrust/MaxThrust), 1);
	Vessel->SetAttitudeRotLevel(Axis, Level);

	return false;
}

bool IsWithinDeadband(double value, double deadBand) const
{
	return (fabs(value) < deadBand);
}

bool IsRotationRateZero(double rotationRate)
{
	IsWithinDeadband(rotationRate, RATE_NULL);
}

double Autopilot::GetTargetRotationRate(double deltaAngle) const 
{
	if (fabs(deltaAngle) < DEADBAND_LOW)
	{
		return RATE_FINE;
	} 
	else if (fabs(deltaAngle) < DEADBAND_MID)
	{
		return RATE_LOW;
	} 
	else if (fabs(deltaAngle) < DEADBAND_HIGH)
	{
		return RATE_MID;
	} 
	else if (fabs(deltaAngle) < DEADBAND_MAX)
	{
		return RATE_HIGH;
	} 
	else
	{
		return RATE_MAX;
	}
}
