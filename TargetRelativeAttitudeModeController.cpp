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
	InitializeCommandMap();
}

TargetRelativeAttitudeModeController::~TargetRelativeAttitudeModeController()
{
}

void TargetRelativeAttitudeModeController::Start()
{
	SelectClosestTarget();
}

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
	display->PrintNewline();

	ScaleOutput(scaledValueBuffer, m_relativeVelocity.data[VERTICAL]);
	display->DisplayText("Vertical: %s", scaledValueBuffer);
	ScaleOutput(scaledValueBuffer, m_relativeVelocity.data[LATERAL]);
	display->DisplayText("Lateral: %s", scaledValueBuffer);
	ScaleOutput(scaledValueBuffer, m_relativeVelocity.data[FORE_AFT]);
	display->DisplayText("Fore/Aft: %s", scaledValueBuffer);
	display->PrintNewline();

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

	if (m_trimState.IsEnabled())
	{
		m_autopilot->TrimRelativeVelocity(m_relativeVelocity, m_trimState);
	}
}

int TargetRelativeAttitudeModeController::GetButtonMenu(const MFDBUTTONMENU** buttonMenu) const
{
	static const MFDBUTTONMENU s_buttonMenu[] = {
		{"Next Target", nullptr, 'N'},
		{"Prev Target", nullptr, 'P'},
		{"Closest Target", nullptr, ' '},
		{"Trim Vertical", "(Numpad)", '1'},
		{"Trim Lateral", "(Numpad)", '2'},
		{"Trim F/A", "(Numpad)", '3'},
		{"Trim All", "(Numpad)", '5'},
		{"Trim Vert, Lat", "(Numpad)", '7'},
		{"Trim Lat, F/A", "(Numpad)", '8'},
		{"Trim Vert, F/A", "(Numpad)", '9'}
	};

	if (buttonMenu != NULL)
	{
		*buttonMenu = s_buttonMenu;
	}

	return (sizeof(s_buttonMenu) / sizeof(s_buttonMenu[0]));
}

bool TargetRelativeAttitudeModeController::ProcessKey(DWORD key)
{
	auto iter = m_commandMap.find(key);

	if (iter != m_commandMap.end())
	{
		iter->second();
		return true;
	}

	return false;
}

void TargetRelativeAttitudeModeController::InitializeCommandMap()
{
	m_commandMap[OAPI_KEY_N] = [this]() { SelectNextTarget(); };
	m_commandMap[OAPI_KEY_P] = [this]() { SelectPreviousTarget(); };
	m_commandMap[OAPI_KEY_SPACE] = [this]() { SelectClosestTarget(); };
	m_commandMap[OAPI_KEY_NUMPAD5] = [this]() { m_trimState = TrimState(true, true, true); };
	m_commandMap[OAPI_KEY_NUMPAD1] = [this]() { m_trimState = TrimState(true, false, false); };
	m_commandMap[OAPI_KEY_NUMPAD2] = [this]() { m_trimState = TrimState(false, true, false); };
	m_commandMap[OAPI_KEY_NUMPAD3] = [this]() { m_trimState = TrimState(false, false, true); };
	m_commandMap[OAPI_KEY_NUMPAD7] = [this]() { m_trimState = TrimState(true, true, false); };
	m_commandMap[OAPI_KEY_NUMPAD8] = [this]() { m_trimState = TrimState(false, true, true); };
	m_commandMap[OAPI_KEY_NUMPAD9] = [this]() { m_trimState = TrimState(true, false, true); };
}

void TargetRelativeAttitudeModeController::BuildTargetList()
{
	unsigned int objectCount = oapiGetObjectCount();

	m_targetList.clear();
	m_targetList.reserve(objectCount + 1);	// Add 1 in case there is a base

	for (unsigned int objectIndex = 0; objectIndex < objectCount; objectIndex++)
	{
		OBJHANDLE target = oapiGetObjectByIndex(objectIndex);

		if (m_spacecraft->GetHandle() != target)
		{
			m_targetList.push_back(target);
		}
	}

	VESSELSTATUS status;
	m_spacecraft->GetStatus(status);

	if (status.base != NULL)
	{
		m_targetList.push_back(status.base);
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

void TargetRelativeAttitudeModeController::SelectClosestTarget()
{
	BuildTargetList();

	m_selectedTargetIndex = 0;
	oapiGetObjectName(m_targetList[m_selectedTargetIndex], m_targetName, sizeof(m_targetName));
}

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
