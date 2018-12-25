#include "Autopilot.h"
#include <cerrno>

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

using namespace std;

struct TelemetryFrame
{
	VECTOR3 targetAttitude;
	VECTOR3 currentAttitude;
	VECTOR3 targetRotationRate;
	VECTOR3 deltaRorationRate;
	VECTOR3 roationRateDeadBand;
	VECTOR3 torque;
	VECTOR3 thrusterLevel;

	void Clear()
	{
		memset(this, 0, sizeof(this));
	}

	void Print(ofstream& log)
	{
		log << targetAttitude.data[PITCH] << "," << targetAttitude.data[YAW] << "," << targetAttitude.data[ROLL] << ",";
		log << currentAttitude.data[PITCH] << "," << currentAttitude.data[YAW] << "," << currentAttitude.data[ROLL] << ",";
		log << targetRotationRate.data[PITCH] << "," << targetRotationRate.data[YAW] << "," << targetRotationRate.data[ROLL] << ",";
		log << deltaRorationRate.data[PITCH] << "," << deltaRorationRate.data[YAW] << "," << deltaRorationRate.data[ROLL] << ",";
		log << roationRateDeadBand.data[PITCH] << "," << roationRateDeadBand.data[YAW] << "," << roationRateDeadBand.data[ROLL] << ",";
		log << torque.data[PITCH] << "," << torque.data[YAW] << "," << torque.data[ROLL] << ",";
		log << thrusterLevel.data[PITCH] << "," << thrusterLevel.data[YAW] << "," << thrusterLevel.data[ROLL] << endl;
	}
};

TelemetryFrame g_telemetryFrame;

Autopilot::Autopilot(VESSEL* spacecraft)
	: m_spacecraft(spacecraft)
{
	m_log.open("c:\\Users\\chris\\telemetry.csv");

	PrintString(strerror(errno));

	m_log << "Target Attitude Pitch,Target Attitude Yaw, Target Attitude Roll,";
	m_log << "Current Attitude Pitch,Current Attitude Yaw,Current Attitude Roll,";
	m_log << "Target Rotation Rate Pitch, Target Rotation Rate Yaw, Target Rotation Rate Roll,";
	m_log << "Delta RotationRate Pitch,Delta RotationRate Yaw,Delta RotationRate Roll,";
	m_log << "Rotation Rate Deadband Pitch,Rotation Rate Deadband Yaw,Rotation Rate Deadband Roll,";
	m_log << "Torque Pitch,Torque Yaw,Torque Roll,";
	m_log << "Thruster Level Pitch,Thruster Level Yaw,Thruster Level Roll" << endl;

	m_spacecraft->GetPMI(m_principleMomentOfInertia);
	InitializeMaxTorque();
}

Autopilot::~Autopilot()
{
	m_log.close();
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
	const VECTOR3& targetAttitude,
	const VECTOR3& currentAttitude,
	DEADBAND deadBand,
	double deltaTime)
{
	g_telemetryFrame.Clear();

	auto isPitchSet = SetAttitudeInAxis(targetAttitude.data[PITCH], currentAttitude.data[PITCH], PITCH, deadBand, deltaTime);
	auto isYawSet = SetAttitudeInAxis(targetAttitude.data[YAW], currentAttitude.data[YAW], YAW, deadBand, deltaTime);
	auto isRollSet = SetAttitudeInAxis(targetAttitude.data[ROLL], currentAttitude.data[ROLL], ROLL, deadBand, deltaTime);

	g_telemetryFrame.Print(m_log);

	return (isPitchSet && isYawSet && isRollSet);
}

bool Autopilot::SetAttitudeInAxis(
		double targetAttitude,
		double currentAttitude,
		AXIS axis,
		DEADBAND deadBand,
		double deltaTime)
{
	double deltaAngle = targetAttitude - currentAttitude;

	g_telemetryFrame.targetAttitude.data[axis] = DEG * targetAttitude;
	g_telemetryFrame.currentAttitude.data[axis] = DEG * currentAttitude;

	// Let's take care of the good case first :-)
	if (IsWithinDeadband(deltaAngle, deadBand))
	{
		return NullRotationRateInAxis(axis, deltaTime);
	}
	else
	{
		double targetRotationRate = GetTargetRotationRate(deltaAngle);

		return SetRotationRateInAxis(axis, targetRotationRate, deltaTime);
	}
}

bool Autopilot::SetRotationRateInAxis(AXIS axis, double targetRotationRate, double deltaTime)
{
	VESSELSTATUS status;
	m_spacecraft->GetStatus(status);
	double currentRotationRate = status.vrot.data[axis];
	double deltaRotationRate = targetRotationRate - currentRotationRate;
	double rotationRateDeadband = min(targetRotationRate / 2, Radians(0.01/*2*/));

	g_telemetryFrame.targetRotationRate.data[axis] = DEG * targetRotationRate;
	g_telemetryFrame.deltaRorationRate.data[axis] = DEG * deltaRotationRate;
	g_telemetryFrame.roationRateDeadBand.data[axis] = DEG * rotationRateDeadband;

	if (IsWithinDeadband(deltaRotationRate, rotationRateDeadband))
	{
		ShutdownRotationThrustersInAxis(axis);
		return true;
	}
	else
	{
		double mass = m_spacecraft->GetMass();
		double torque = (mass * m_principleMomentOfInertia.data[axis] * fabs(deltaRotationRate)) / deltaTime;
		double thusterLevel = min(torque / m_maxTorque[axis], 1);
		THGROUP_TYPE thrusterGroup = GetThrusterGroupForRotationInAxis(axis, deltaRotationRate);

		g_telemetryFrame.torque.data[axis] = torque;
		g_telemetryFrame.thrusterLevel.data[axis] = deltaRotationRate >= 0 ? thusterLevel * 100 : -thusterLevel * 100;

		m_spacecraft->SetThrusterGroupLevel(thrusterGroup, thusterLevel);
	}

	return false;
}

void Autopilot::Disable()
{
	ShutdownRotationThrustersInAxis(PITCH);
	ShutdownRotationThrustersInAxis(YAW);
	ShutdownRotationThrustersInAxis(ROLL);

}

bool Autopilot::NullRotationRateInAxis(AXIS axis, double deltaTime)
{
	return SetRotationRateInAxis(axis, 0, deltaTime);
}

bool Autopilot::IsWithinDeadband(double value, double deadBand) const
{
	if (value > 0 && deadBand < 0)
	{
		return false;
	}
	else if (value < 0 && deadBand > 0)
	{
		return false;
	}
	else
	{
		return (fabs(value) < fabs(deadBand));
	}
}

bool Autopilot::IsRotationRateZero(double rotationRate) const
{
	return IsWithinDeadband(rotationRate, RATE_NULL);
}

void Autopilot::ShutdownRotationThrustersInAxis(AXIS axis)
{
	g_telemetryFrame.torque.data[axis] = 0;
	g_telemetryFrame.thrusterLevel.data[axis] = 0;

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

THGROUP_TYPE Autopilot::GetThrusterGroupForRotationInAxis(AXIS axis, double rotationRate) const
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

