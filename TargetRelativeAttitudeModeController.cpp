#include "TargetRelativeAttitudeModeController.h"
#include "OrbiterSDK.h"
#include "CDK.h"
#include "Display.h"
#include <algorithm>

using namespace std;
TargetRelativeAttitudeModeController::TargetRelativeAttitudeModeController(
	VESSEL* spacecraft,
	DWORD displayWidth,
	DWORD displayHeight)
	: m_spacecraft(spacecraft)
	, m_displayWidth(displayWidth)
	, m_displayHeight(displayHeight)
	, m_selectedTargetIndex(0)
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
	display->DisplayText("Target Name: %s", m_targetName);

	return true;
}

void TargetRelativeAttitudeModeController::UpdateState()
{

}

void TargetRelativeAttitudeModeController::EnableAutopilot()
{
}

void TargetRelativeAttitudeModeController::DisableAutopilot()
{
}

void TargetRelativeAttitudeModeController::Control()
{
}

int TargetRelativeAttitudeModeController::GetButtonMenu(const MFDBUTTONMENU** buttonMenu) const
{
	static const MFDBUTTONMENU s_buttonMenu[] = {
		{"User Att", "Mode", '1'},
		{"Velocity", "Mode", '2'},
		{"Target Rel", "Mode", '3'},
		{"Entry Interface", "Mode", '4'},
		{"Select Mode", 0, 'M'},
		{"Attitude Hold", 0, 'H'},
		{"Set Reference", "Attitude", '.'},
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
		m_targetList.push_back(oapiGetObjectByIndex(objectIndex));
	}

	std::sort(m_targetList.begin(), m_targetList.end(), [this](const OBJHANDLE object1, const OBJHANDLE object2)
	{
		return (GetRelativeDistance(object2) > GetRelativeDistance(object1));
	});
}


double TargetRelativeAttitudeModeController::GetRelativeDistance(const OBJHANDLE objectHandle) const
{
	VECTOR3 relativePosition;

	m_spacecraft->GetRelativePos(objectHandle, relativePosition);
	return Mag(relativePosition);
}

void TargetRelativeAttitudeModeController::SelectNextTarget()
{
	m_selectedTargetIndex = (m_selectedTargetIndex + 1) % m_targetList.size();
	oapiGetObjectName(m_targetList[m_selectedTargetIndex], m_targetName, sizeof(m_targetName));
}