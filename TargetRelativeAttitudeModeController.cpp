#include "TargetRelativeAttitudeModeController.h"
#include "OrbiterSDK.h"
#include "CDK.h"
#include <algorithm>

using namespace std;

static VECTOR3 GetPY(VECTOR3 PitchYaw)
{
	VECTOR3 Res = { 0, 0, 0 };

	Res = GetPitchYawRoll(PitchYaw, NULL_VECTOR);

	// Change the signs to make it consistent
	Res.data[PITCH] = -Res.data[PITCH];

	return Res;

}

TargetRelativeAttitudeModeController::TargetRelativeAttitudeModeController(
	VESSEL* spacecraft,
	const shared_ptr<IAutopilot>& autopilot,
	const CreateDisplayFunction& createDisplay)
	: m_spacecraft(spacecraft)
	, m_autopilot(autopilot)
	, m_createDisplay(createDisplay)
	, m_isAutopilotEngaged(false)
	, m_selectedTargetIndex(0)
	, m_relativePosition(NULL_VECTOR)
	, m_relativeVelocity(NULL_VECTOR)
	, m_radialVelocity(0.0)
	, m_pitchYawAngles(NULL_VECTOR)
{
}

TargetRelativeAttitudeModeController::~TargetRelativeAttitudeModeController()
{
}

void TargetRelativeAttitudeModeController::Start()
{
	BuildTargetList();

	m_selectedTargetIndex = 0;
	oapiGetObjectName(m_targetList[m_selectedTargetIndex], m_targetName, sizeof(m_targetName));
}

/*char Buffer[100];

PrintRefMode();

sprintf(Buffer, "Target: %s", TargetName);
TextOut(hDC, 5, CurrentLine, Buffer, strlen(Buffer));
CurrentLine += LINE;

// Print distance
sprintf(Buffer, "Distance:", 8);
TextOut(hDC, 5, CurrentLine, Buffer, strlen(Buffer));
ScaleOutput(Buffer, Mag(RelPos));
TextOut(hDC, 100, CurrentLine, Buffer, strlen(Buffer));
CurrentLine += LINE;
sprintf(Buffer, "Rad Vel:", 8);
TextOut(hDC, 5, CurrentLine, Buffer, strlen(Buffer));
ScaleOutput(Buffer, RadialVel);
TextOut(hDC, 100, CurrentLine, Buffer, strlen(Buffer));
CurrentLine += 2 * LINE;

PrintRelVel();
CurrentLine += 2 * LINE;


PrintAngleRate("Pitch:", PitchYawRoll.data[PITCH], Status.vrot.x);
PrintAngleRate("Yaw:", PitchYawRoll.data[YAW], Status.vrot.y);
PrintAngleRate("Roll:", PitchYawRoll.data[ROLL], Status.vrot.z);

CurrentLine += 2 * LINE;

PrintRotThrust();
*/

bool TargetRelativeAttitudeModeController::Update(oapi::Sketchpad* sketchpad)
{
	auto display = m_createDisplay(sketchpad);

	display->Reset();
	
	display->SetTextColor(RGB(255, 255, 255));
	display->DisplayText("Attitude MFD");
	display->SetTextColor(RGB(0, 255, 0));
	display->PrintNewline();

	if (m_isAutopilotEngaged)
	{
		display->DisplayText("Ref Mode: Target Relative  (Hold)");
	}
	else
	{
		display->DisplayText("Ref Mode: Target Relative");
	}

	display->PrintNewline();
	display->DisplayText("Target Name: %s", m_targetName);

	char scaledValueBuffer[100];
	
	ScaleOutput(scaledValueBuffer, Mag(m_relativePosition));
	display->DisplayText("Distance: %s", scaledValueBuffer);

	ScaleOutput(scaledValueBuffer, m_radialVelocity);
	display->DisplayText("Radial Velocity: %s", scaledValueBuffer);
	display->PrintNewline();

	display->PrintAngleAndRate("Pitch:", m_pitchYawAngles.data[PITCH], m_status.vrot.x);
	display->PrintAngleAndRate("Yaw:", m_pitchYawAngles.data[YAW], m_status.vrot.y);

	return true;
}

void TargetRelativeAttitudeModeController::UpdateState()
{
	m_spacecraft->GetStatus(m_status);

	SetRelativePositionToSelectedTarget();
	SetRelativeVelocityToSelectedTarget();
	SetRadialVelocityToSelectedTarget();

	m_pitchYawAngles = GetPY(m_relativePosition);
}

void TargetRelativeAttitudeModeController::EnableAutopilot()
{
	m_isAutopilotEngaged = true;
}

void TargetRelativeAttitudeModeController::DisableAutopilot()
{
	m_isAutopilotEngaged = false;
	m_autopilot->ShutdownAllEngines();
}

void TargetRelativeAttitudeModeController::Control()
{
	if (m_isAutopilotEngaged)
	{
		m_autopilot->SetAttitude(NULL_VECTOR, m_pitchYawAngles, DB_FINE, 1.0);
	}
}

int TargetRelativeAttitudeModeController::GetButtonMenu(const MFDBUTTONMENU** buttonMenu) const
{
	static const MFDBUTTONMENU s_buttonMenu[] = {
		{"Next Target", nullptr, 'N'},
		{"Prev Target", nullptr, 'P'},
		//{"Closest Target", nullptr, ' '},
		//{"Select Base", nullptr, 'B'},
	};

	if (buttonMenu != NULL)
	{
		*buttonMenu = s_buttonMenu;
	}

	return (sizeof(s_buttonMenu) / sizeof(s_buttonMenu[0]));
}

bool TargetRelativeAttitudeModeController::ProcessKey(DWORD key)
{
	switch (key)
	{
	case OAPI_KEY_N:
		SelectNextTarget();
		return true;
	case OAPI_KEY_P:
		SelectPreviousTarget();
		return true;
	default:
		return false;
	}
}

void TargetRelativeAttitudeModeController::BuildTargetList()
{
	unsigned int objectCount = oapiGetObjectCount();

	m_targetList.clear();
	m_targetList.reserve(objectCount);

	for (unsigned int objectIndex = 0; objectIndex < objectCount; objectIndex++)
	{
		OBJHANDLE target = oapiGetObjectByIndex(objectIndex);

		if (m_spacecraft->GetHandle() != target)
		{
			m_targetList.push_back(target);
		}
	}

	std::sort(m_targetList.begin(), m_targetList.end(), [this](const OBJHANDLE object1, const OBJHANDLE object2)
	{
		return (GetRelativeDistanceToObject(object2) > GetRelativeDistanceToObject(object1));
	});
}


double TargetRelativeAttitudeModeController::GetRelativeDistanceToObject(const OBJHANDLE objectHandle) const
{
	VECTOR3 relativePosition;

	m_spacecraft->GetRelativePos(objectHandle, relativePosition);
	return Mag(relativePosition);
}

void TargetRelativeAttitudeModeController::SetRelativePositionToSelectedTarget()
{
	VECTOR3 localTargetPosition;
	GetLocalPositionOfObject(GetSelectedTargetHandle(), localTargetPosition);

	VECTOR3 localSpacecraftPosition;
	GetLocalPositionOfObject(m_spacecraft->GetHandle(), localSpacecraftPosition);

	VECTOR3 relativePosition = localTargetPosition - localSpacecraftPosition;

	m_relativePosition = relativePosition;
}

void TargetRelativeAttitudeModeController::GetLocalPositionOfObject(OBJHANDLE object, VECTOR3& localPosition) const
{
	VECTOR3 globalPosition;
	oapiGetGlobalPos(object, &globalPosition);

	m_spacecraft->Global2Local(globalPosition, localPosition);
}

void TargetRelativeAttitudeModeController::SetRelativeVelocityToSelectedTarget()
{
	VECTOR3 globalTargetRelativeVelocity;
	m_spacecraft->GetRelativeVel(GetSelectedTargetHandle(), globalTargetRelativeVelocity);

	VECTOR3 globalSpacecraftPosition;
	oapiGetGlobalPos(m_spacecraft->GetHandle(), &globalSpacecraftPosition);

	VECTOR3 relativeVelocity;
	m_spacecraft->Global2Local((globalTargetRelativeVelocity + globalSpacecraftPosition), relativeVelocity);

	m_relativeVelocity = relativeVelocity;
}

void TargetRelativeAttitudeModeController::SetRadialVelocityToSelectedTarget()
{
	m_radialVelocity = (m_relativePosition * m_relativeVelocity) / Mag(m_relativePosition);
}


OBJHANDLE TargetRelativeAttitudeModeController::GetSelectedTargetHandle() const
{
	return m_targetList[m_selectedTargetIndex];
}

void TargetRelativeAttitudeModeController::SelectNextTarget()
{
	m_selectedTargetIndex = (m_selectedTargetIndex + 1) % m_targetList.size();
	oapiGetObjectName(m_targetList[m_selectedTargetIndex], m_targetName, sizeof(m_targetName));
}

void TargetRelativeAttitudeModeController::SelectPreviousTarget()
{
	if (m_selectedTargetIndex == 0)
	{
		m_selectedTargetIndex = m_targetList.size() - 1;
	}
	else
	{
		m_selectedTargetIndex--;
	}

	oapiGetObjectName(m_targetList[m_selectedTargetIndex], m_targetName, sizeof(m_targetName));
}

//void inline AttitudeMFD::SetTrimMode(TRIM_MODE Mode)
//{
//	if (RefMode != TARGET_RELATIVE) {
//		return;
//	}
//
//	// If we hit the same key a second time, we'll disable the trim mode
//	if ((TrimStatus == T_ENGAGED) && (TrimMode == Mode)) {
//		ToggleTrimStatus();
//		return;
//	}
//
//	ToggleTrimStatus();
//	TrimMode = Mode;
//}
//
//void AttitudeMFD::ToggleTrimStatus()
//{
//	TrimStatus = (TRIM_STATUS)((TrimStatus + 1) % 2);
//
//	// Don't leave the thrusters firing!
//	if (TrimStatus == T_DISENGAGED) {
//		Spacecraft->SetAttitudeLinLevel(NULL_VECTOR);
//	}
//
//}
//
//void inline AttitudeMFD::SetTrimLevelVert()
//{
//	double Level = 0.0;
//
//	// First, we'll calculate the thrust level assuming that we'll
//	// be using the linear thruster
//	Level = -(Mass * RelVel.data[TA_VERT]) / MaxAttThrust;
//
//	// We need to decided if we really want to use linear thrusters, or if
//	// it's better to use one of the engines
//	if (HaveHoverEngine() && Level > 3.0) {
//		Level = -(Mass * RelVel.data[TA_VERT]) / MaxHoverThrust;
//		// NormalizeThrustLevel(Level);
//		Spacecraft->SetEngineLevel(ENGINE_HOVER, Level);
//	}
//	else {
//		// NormalizeThrustLevel(Level);
//		Spacecraft->SetAttitudeLinLevel((int)TA_VERT, Level);
//	}
//
//}
//
//void inline AttitudeMFD::SetTrimLevelLat()
//{
//	double Level = 0.0;
//
//	// First, we'll calculate the thrust level assuming that we'll
//	// be using the linear thruster
//	Level = -(Mass * RelVel.data[TA_LAT]) / MaxAttThrust;
//
//	// NormalizeThrustLevel(Level);
//	Spacecraft->SetAttitudeLinLevel((int)TA_LAT, Level);
//
//}
//
//
//void inline AttitudeMFD::SetTrimLevelFA()
//{
//	double Level = 0.0;
//
//	// First, we'll calculate the thrust level assuming that we'll
//	// be using the linear thruster
//	Level = -(Mass * RelVel.data[TA_FA]) / MaxAttThrust;
//
//	if (HaveMainEngine() && Level > 3.0) {
//		Level = -(Mass * RelVel.data[TA_FA]) / MaxMainThrust;
//		// NormalizeThrustLevel(Level);
//		Spacecraft->SetEngineLevel(ENGINE_MAIN, Level);
//	}
//	else if (HaveRetroEngine() && Level < -3.0) {
//		Level = (Mass * RelVel.data[TA_FA]) / MaxRetroThrust;
//		// NormalizeThrustLevel(Level);
//		Spacecraft->SetEngineLevel(ENGINE_RETRO, Level);
//	}
//	else {
//		// NormalizeThrustLevel(Level);
//		Spacecraft->SetAttitudeLinLevel((int)TA_FA, Level);
//	}
//}
//
//void inline AttitudeMFD::SetTrimThrustLevel(T_AXIS Axis)
//{
//	const double DEADBAND = 0.001;
//
//	if (fabs(RelVel.data[Axis]) > DEADBAND) {
//		switch (Axis) {
//		case TA_VERT:
//			SetTrimLevelVert();
//			TrimStatus = T_ENGAGED;
//			break;
//		case TA_LAT:
//			SetTrimLevelLat();
//			TrimStatus = T_ENGAGED;
//			break;
//		case TA_FA:
//			SetTrimLevelFA();
//			TrimStatus = T_ENGAGED;
//			break;
//		default:
//			break;
//		}
//	}
//}
//
//
//// This function assumes that trim is engaged
//void inline AttitudeMFD::Trim()
//{
//
//	// "Clear the board"
//	Spacecraft->SetAttitudeLinLevel(NULL_VECTOR);
//
//	// This will be reset by SetTrimThrustLevel() if trim is used
//	TrimStatus = T_DISENGAGED;
//
//	if (TrimMode == T_VERT ||
//		TrimMode == T_ALL ||
//		TrimMode == T_VERT_LAT ||
//		TrimMode == T_VERT_FA) {
//
//		// Clear hover engine thrust
//		Spacecraft->SetEngineLevel(ENGINE_HOVER, 0.0);
//
//		SetTrimThrustLevel(TA_VERT);
//	}
//
//	if (TrimMode == T_LAT ||
//		TrimMode == T_ALL ||
//		TrimMode == T_VERT_LAT ||
//		TrimMode == T_LAT_FA) {
//		SetTrimThrustLevel(TA_LAT);
//	}
//
//	if (TrimMode == T_FA ||
//		TrimMode == T_ALL ||
//		TrimMode == T_LAT_FA ||
//		TrimMode == T_VERT_FA) {
//
//		// Clear main/retro thrust
//		Spacecraft->SetEngineLevel(ENGINE_MAIN, 0.0);
//		Spacecraft->SetEngineLevel(ENGINE_RETRO, 0.0);
//
//		SetTrimThrustLevel(TA_FA);
//	}
//
//}