#include "TargetRelativeAttitudeModeController.h"
#include "OrbiterSDK.h"
#include "CDK.h"
#include "Display.h"
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
	DWORD displayWidth,
	DWORD displayHeight,
	const shared_ptr<IAutopilot>& autopilot)
	: m_spacecraft(spacecraft)
	, m_displayWidth(displayWidth)
	, m_displayHeight(displayHeight)
	, m_autopilot(autopilot)
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

bool TargetRelativeAttitudeModeController::Update(oapi::Sketchpad* sketchpad)
{
	Display* display = new Display(sketchpad, m_displayWidth, m_displayHeight);

	display->Reset();
	
	display->SetTextColor(RGB(255, 255, 255));
	display->DisplayText("Attitude MFD");
	display->SetTextColor(RGB(0, 255, 0));
	display->PrintNewline();

	display->DisplayText("Ref Mode: Target Relative");
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
	m_autopilot->Disable();
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
		{"User Att", "Mode", '1'},
		{"Velocity", "Mode", '2'},
		{"Target Rel", "Mode", '3'},
		{"Entry Interface", "Mode", '4'},
		{"Select Mode", nullptr, 'M'},
		{"Attitude Hold", nullptr, 'H'},

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