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
	m_maxTorque[PITCH] = GetThrusterGroupMaxTorque(THGROUP_ATT_PITCHUP);// + 
							//GetThrusterGroupMaxTorque(THGROUP_ATT_PITCHDOWN);
	
	m_maxTorque[YAW] = GetThrusterGroupMaxTorque(THGROUP_ATT_YAWLEFT);// + 
							//GetThrusterGroupMaxTorque(THGROUP_ATT_YAWRIGHT);
	
	m_maxTorque[ROLL] = GetThrusterGroupMaxTorque(THGROUP_ATT_BANKLEFT);// + 
							//GetThrusterGroupMaxTorque(THGROUP_ATT_BANKRIGHT);
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
	double deltaAngle = targetAttitude - currentAttitude;

	// Let's take care of the good case first :-)
	if (IsWithinDeadband(deltaAngle, deadBand))
	{
		return NullRotationRate(axis, deltaTime);
	}
	else
	{
		double targetRotationRate = GetTargetRotationRate(deltaAngle);

		return SetRotationRate(axis, targetRotationRate, deltaTime);
	}
}

bool Autopilot::SetRotationRate(AXIS axis, double targetRotationRate, double deltaTime)
{
	VESSELSTATUS status;
	m_spacecraft->GetStatus(status);
	double currentRotationRate = status.vrot.data[axis];
	double deltaRotationRate = targetRotationRate - currentRotationRate;
	double rotationRateDeadband = min(targetRotationRate / 2, Radians(0.01/*2*/));

	double mass = m_spacecraft->GetMass();
	double torque = (mass * m_principleMomentOfInertia.data[axis] * fabs(deltaRotationRate)) / deltaTime;
	double thusterLevel = min(torque / m_maxTorque[axis], 1);
	THGROUP_TYPE thrusterGroup = GetThrusterGroupForRotation(axis, deltaRotationRate);

	if (IsWithinDeadband(deltaRotationRate, rotationRateDeadband))
	{
		ShutdownRotationThrusters(axis);
		return true;
	}
	else
	{
		m_spacecraft->SetThrusterGroupLevel(thrusterGroup, thusterLevel);
	}

	return false;
}

bool Autopilot::NullRotationRate(AXIS axis, double deltaTime)
{
	VESSELSTATUS status;
	m_spacecraft->GetStatus(status);

	return SetRotationRate(axis, 0.0, deltaTime);
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
	double rotationRateMagnitude = 0.0;

	if (fabs(deltaAngle) < DEADBAND_LOW)
	{
		rotationRateMagnitude = RATE_FINE;
	} 
	else if (fabs(deltaAngle) < DEADBAND_MID)
	{
		rotationRateMagnitude = RATE_LOW;
	} 
	else if (fabs(deltaAngle) < DEADBAND_HIGH)
	{
		rotationRateMagnitude = RATE_MID;
	} 
	else if (fabs(deltaAngle) < DEADBAND_MAX)
	{
		rotationRateMagnitude = RATE_HIGH;
	} 
	else
	{
		rotationRateMagnitude = RATE_MAX;
	}

	if (deltaAngle >= 0)
	{
		return rotationRateMagnitude;
	}
	else
	{
		return -rotationRateMagnitude;
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
		return isPositiveRate ? THGROUP_ATT_YAWLEFT : THGROUP_ATT_YAWRIGHT;
	case ROLL:
		return isPositiveRate ? THGROUP_ATT_BANKRIGHT : THGROUP_ATT_BANKLEFT;
	default:
		throw std::runtime_error("Unexpected axis");
	}
}
