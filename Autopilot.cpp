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
		DEADBAND deadBand,
		double deltaTime)
{
	double targetRotationRate = 0.0;			
	double rotationRateDeadband = 0.0;	

	double deltaRate = 0.0;		
	double deltaAngle = targetAttitude - currentAttitude;

	VESSELSTATUS status;
	m_spacecraft->GetStatus(status);
	double currentRotationRate = status.vrot.data[axis];

	// Let's take care of the good case first :-)
	if (IsWithinDeadband(deltaAngle, deadBand))
	{
		if (IsRotationRateZero(currentRotationRate))
		{
			ShutdownRotationThrusters(axis);
			return true;
		} 
	
		return (NullRotationRate(axis));
	}

	targetRotationRate = GetTargetRotationRate(deltaAngle);

	rotationRateDeadband = min(targetRotationRate / 2, Radians(0.01/*2*/));
	
	if (deltaAngle < 0 ) {
		targetRotationRate = -targetRotationRate;
		rotationRateDeadband = -rotationRateDeadband;
	}

	double deltaRotationRate = targetRotationRate - currentRotationRate;

	double mass = m_spacecraft->GetMass();
	double torque = (mass * m_principleMomentOfInertia.data[axis] * deltaRate) / deltaTime;
	double thusterLevel = min(torque / m_maxTorque[axis], 1);
	THGROUP_TYPE thrusterGroup = GetThrusterGroupForRotation(axis, deltaRate);

	if (IsWithinDeadband(deltaRate, rotationRateDeadband))
	{
		ShutdownRotationThrusters(axis);
	}
	else
	{
		m_spacecraft->SetThrusterGroupLevel(thrusterGroup, thusterLevel);
	}

	/*if (DeltaAngle > 0) {
		if (DeltaRate > rotationRateDeadband) {
			Thrust = (Mass * PMI.data[axis] * DeltaRate) / deltaTime;
			Level = min((Thrust/MaxThrust), 1);
			m_spacecraft->SetAttitudeRotLevel(axis, Level);
		} else if (DeltaRate < -rotationRateDeadband) { 
			Thrust = (Mass * PMI.data[axis] * DeltaRate) / deltaTime;
			Level = max((Thrust/MaxThrust), -1);
			m_spacecraft->SetAttitudeRotLevel(axis, Level);
		} else {
			ShutdownRotationThrusters(axis);
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
			ShutdownRotationThrusters(axis);
		}
	}*/
	return false;
}

bool Autopilot::NullRotationRate(AXIS axis)
{
	VESSELSTATUS status;
	m_spacecraft->GetStatus(status);

	double rotationRate = status.vrot.data[axis];

	if (IsRotationRateZero(rotationRate)) {
		ShutdownRotationThrusters(axis);
		return true;
	}

	double RateDeadband = Radians(0.001), Thrust, Level,
			Mass = m_spacecraft->GetMass(),
			MaxThrust = m_spacecraft->GetMaxThrust(ENGINE_ATTITUDE),
			Size = m_spacecraft->GetSize();

	Thrust = -(Mass * m_principleMomentOfInertia.data[axis] * rotationRate) / (Size);	
	Level = min((Thrust/MaxThrust), 1);
	m_spacecraft->SetAttitudeRotLevel(axis, Level);

	return false;
}

bool Autopilot::IsWithinDeadband(double value, double deadBand) const
{
	return (fabs(value) < deadBand);
}

bool Autopilot::IsRotationRateZero(double rotationRate) const
{
	return IsWithinDeadband(rotationRate, RATE_NULL);
}

void Autopilot::ShutdownRotationThrusters(AXIS axis)
{
	switch (axis)
	{
	case PITCH:
		m_spacecraft->SetThrusterGroupLevel(THGROUP_ATT_PITCHUP, 0);
		m_spacecraft->SetThrusterGroupLevel(THGROUP_ATT_PITCHDOWN, 0);
		break;
	case YAW:
		m_spacecraft->SetThrusterGroupLevel(THGROUP_ATT_YAWLEFT, 0);
		m_spacecraft->SetThrusterGroupLevel(THGROUP_ATT_YAWRIGHT, 0);
		break;
	case ROLL:
		m_spacecraft->SetThrusterGroupLevel(THGROUP_ATT_BANKLEFT, 0);
		m_spacecraft->SetThrusterGroupLevel(THGROUP_ATT_BANKRIGHT, 0);
		break;
	default:
		throw std::runtime_error("Unexpected axis");
	}
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

THGROUP_TYPE Autopilot::GetThrusterGroupForRotation(AXIS axis, double rotationRate) const
{
	bool isPositiveRate = rotationRate >= 0;

	switch (axis)
	{
	case PITCH:
		return isPositiveRate ? THGROUP_ATT_PITCHUP : THGROUP_ATT_PITCHDOWN;
	case YAW:
		return isPositiveRate ? THGROUP_ATT_YAWRIGHT : THGROUP_ATT_YAWLEFT;
	case ROLL:
		return isPositiveRate ? THGROUP_ATT_BANKRIGHT : THGROUP_ATT_BANKLEFT;
	default:
		throw std::runtime_error("Unexpected axis");
	}
}
