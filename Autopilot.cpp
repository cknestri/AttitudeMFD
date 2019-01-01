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
	VECTOR3 relativeVelocity;
	VECTOR3 thrust;
	VECTOR3 thrusterLevelLinear;

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
		log << thrusterLevel.data[PITCH] << "," << thrusterLevel.data[YAW] << "," << thrusterLevel.data[ROLL] << ",";
		log << relativeVelocity.data[VERTICAL] << "," << relativeVelocity.data[LATERAL] << "," << relativeVelocity.data[FORE_AFT] << ",";
		log << thrust.data[VERTICAL] << "," << thrust.data[LATERAL] << "," << thrust.data[FORE_AFT] << ",";
		log << thrusterLevelLinear.data[VERTICAL] << "," << thrusterLevelLinear.data[LATERAL] << "," << thrusterLevelLinear.data[FORE_AFT] << endl;
	}
};

TelemetryFrame g_telemetryFrame;

Autopilot::Autopilot(VESSEL* spacecraft)
	: m_spacecraft(spacecraft)
{
	m_log.open("c:\\Users\\chris\\telemetry.csv");

	m_log << "Target Attitude Pitch,Target Attitude Yaw, Target Attitude Roll,";
	m_log << "Current Attitude Pitch,Current Attitude Yaw,Current Attitude Roll,";
	m_log << "Target Rotation Rate Pitch, Target Rotation Rate Yaw, Target Rotation Rate Roll,";
	m_log << "Delta RotationRate Pitch,Delta RotationRate Yaw,Delta RotationRate Roll,";
	m_log << "Rotation Rate Deadband Pitch,Rotation Rate Deadband Yaw,Rotation Rate Deadband Roll,";
	m_log << "Torque Pitch,Torque Yaw,Torque Roll,";
	m_log << "Thruster Level Pitch,Thruster Level Yaw,Thruster Level Roll,";
	m_log << "Relative Velocity Vertical, Relative Velocity Lateral, Relative Velocity F/A,";
	m_log << "Thrust Vertical, Thrust Lateral, Thrust Velocity F/A,";
	m_log << "Thruster Level Vertical, Thruster Level Lateral, Thruster Level F/A,";

	m_spacecraft->GetPMI(m_principleMomentOfInertia);
	
	InitializeMaxTorque();
	InitializeMaxThrust();
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

void Autopilot::InitializeMaxThrust()
{
	m_maxThrust[VERTICAL] = GetThrusterGroupMaxThrust(THGROUP_ATT_UP);
	m_maxThrust[LATERAL] = GetThrusterGroupMaxThrust(THGROUP_ATT_LEFT);
	m_maxThrust[FORE_AFT] = GetThrusterGroupMaxThrust(THGROUP_ATT_FORWARD);
}

double Autopilot::GetThrusterGroupMaxThrust(THGROUP_TYPE thrusterGroup) const
{
	double maxThrust = 0.0;

	int totalThrusters = m_spacecraft->GetGroupThrusterCount(thrusterGroup);

	for (int thrusterIndex = 0; thrusterIndex < totalThrusters; thrusterIndex++)
	{
		maxThrust += GetThusterThrust(thrusterGroup, thrusterIndex);
	}

	return maxThrust;
}

double Autopilot::GetThusterThrust(THGROUP_TYPE thrusterGroup, int thrusterIndex) const
{
	THRUSTER_HANDLE thruster = m_spacecraft->GetGroupThruster(thrusterGroup, thrusterIndex);
	
	return m_spacecraft->GetThrusterMax(thruster);
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
	if (IsDeltaValueWithinDeadband(deltaAngle, deadBand))
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
	double rotationRateDeadband = GetRotationRateDeadband(targetRotationRate);

	g_telemetryFrame.targetRotationRate.data[axis] = DEG * targetRotationRate;
	g_telemetryFrame.deltaRorationRate.data[axis] = DEG * deltaRotationRate;
	g_telemetryFrame.roationRateDeadBand.data[axis] = DEG * rotationRateDeadband;

	// Clear the thrusters.  If there are any thruster firings, they will be commanded in this function.
	ShutdownRotationThrustersInAxis(axis);

	if (IsDeltaValueWithinDeadband(deltaRotationRate, rotationRateDeadband))
	{
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

void Autopilot::ShutdownAllEngines()
{
	ShutdownRotationThrustersInAxis(PITCH);
	ShutdownRotationThrustersInAxis(YAW);
	ShutdownRotationThrustersInAxis(ROLL);

	ShutdownTranslationThrustersInAxis(VERTICAL);
	ShutdownTranslationThrustersInAxis(LATERAL);
	ShutdownTranslationThrustersInAxis(FORE_AFT);
}

bool Autopilot::NullRotationRateInAxis(AXIS axis, double deltaTime)
{
	return SetRotationRateInAxis(axis, 0, deltaTime);
}

bool Autopilot::IsDeltaValueWithinDeadband(double deltaValue, double deadband) const
{
	return (fabs(deltaValue) <= fabs(deadband));
}

double Autopilot::GetRotationRateDeadband(double targetRotationRate) const
{
	return min(fabs(targetRotationRate) / 2, Radians(0.01/*2*/));
}

bool Autopilot::IsRotationRateZero(double rotationRate) const
{
	return IsDeltaValueWithinDeadband(rotationRate, RATE_NULL);
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

bool Autopilot::TrimRelativeVelocity(const VECTOR3& relativeVelocity, const TrimState& trimState)
{
	g_telemetryFrame.Clear();

	bool isVerticalTrimmed = true;
	bool isLateralTrimmed = true;
	bool isForeAftTrimmed = true;

	if (trimState.IsVerticalEnabled())
	{
		isVerticalTrimmed = TrimRelativeVelocityInAxis(relativeVelocity.data[VERTICAL], VERTICAL);
	}

	if (trimState.IsLateralEnabled())
	{
		isLateralTrimmed = TrimRelativeVelocityInAxis(relativeVelocity.data[LATERAL], LATERAL);
	}

	if (trimState.IsForeAftEnabled())
	{
		isForeAftTrimmed = TrimRelativeVelocityInAxis(relativeVelocity.data[FORE_AFT], FORE_AFT);
	}

	g_telemetryFrame.Print(m_log);

	return false;
}

bool Autopilot::TrimRelativeVelocityInAxis(double relativeVelocity, LINEAR_AXIS axis)
{
	// Clear the thrusters.  If there are any thruster firings, they will be commanded in this function.
	ShutdownTranslationThrustersInAxis(axis);

	g_telemetryFrame.relativeVelocity.data[axis] = relativeVelocity;

	if (IsRelativeVelocityWithinDeadBand(relativeVelocity))
	{
		g_telemetryFrame.thrust.data[axis] = 0;
		g_telemetryFrame.thrusterLevelLinear.data[axis] = 0;

		return true;
	}
	
	double mass = m_spacecraft->GetMass();
	double thrust = -(mass * relativeVelocity);
	THGROUP_TYPE thrusterGroup = GetThrusterGroupForTranslationInAxis(axis, thrust);
	double thrusterLevel = min(fabs(thrust) / m_maxThrust[axis], 1);

	m_spacecraft->SetThrusterGroupLevel(thrusterGroup, thrusterLevel);

	g_telemetryFrame.thrust.data[axis] = thrust;
	g_telemetryFrame.thrusterLevelLinear.data[axis] = thrusterLevel;

	return false;
}

bool Autopilot::IsRelativeVelocityWithinDeadBand(double relativeVelocity) const
{
	const double DEADBAND = 0.01;

	return (fabs(relativeVelocity) < DEADBAND);
}

THGROUP_TYPE Autopilot::GetThrusterGroupForTranslationInAxis(LINEAR_AXIS axis, double thrust) const
{
	bool isPositiveDirection = thrust >= 0;

	switch (axis)
	{
	case VERTICAL:
		return isPositiveDirection ? THGROUP_ATT_UP: THGROUP_ATT_DOWN;
	case LATERAL:
		return isPositiveDirection ? THGROUP_ATT_RIGHT : THGROUP_ATT_LEFT;
	case FORE_AFT:
		return isPositiveDirection ? THGROUP_ATT_FORWARD : THGROUP_ATT_BACK;
	default:
		throw std::runtime_error("Unexpected axis");
	}
}

void Autopilot::ShutdownTranslationThrustersInAxis(LINEAR_AXIS axis)
{
	switch (axis)
	{
	case VERTICAL:
		m_spacecraft->SetThrusterGroupLevel(THGROUP_ATT_UP, 0);
		m_spacecraft->SetThrusterGroupLevel(THGROUP_ATT_DOWN, 0);
		break;
	case LATERAL:
		m_spacecraft->SetThrusterGroupLevel(THGROUP_ATT_LEFT, 0);
		m_spacecraft->SetThrusterGroupLevel(THGROUP_ATT_RIGHT, 0);
		break;
	case FORE_AFT:
		m_spacecraft->SetThrusterGroupLevel(THGROUP_ATT_FORWARD, 0);
		m_spacecraft->SetThrusterGroupLevel(THGROUP_ATT_BACK, 0);
		break;
	default:
		throw std::runtime_error("Unexpected axis");
	}
}