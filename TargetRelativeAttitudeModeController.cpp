#include "TargetRelativeAttitudeModeController.h"
#include "OrbiterSDK.h"
#include "Display.h"

TargetRelativeAttitudeModeController::TargetRelativeAttitudeModeController(
		VESSEL* spacecraft,
		DWORD displayWidth,
		DWORD displayHeight)
	: m_spacecraft(spacecraft)
	, m_displayWidth(displayWidth)
	, m_displayHeight(displayHeight)
{
}

TargetRelativeAttitudeModeController::~TargetRelativeAttitudeModeController()
{

}

void TargetRelativeAttitudeModeController::Start()
{

}

bool TargetRelativeAttitudeModeController::Update(oapi::Sketchpad* sketchpad)
{
	Display* display = new Display(sketchpad, m_displayWidth, m_displayHeight);

	display->Reset();
	
	display->SetTextColor(RGB(255, 255, 255));
	display->DisplayText("Attitude MFD");
	display->SetTextColor(RGB(0, 255, 0));
	display->IncrementCurrentLine();

	display->DisplayText("Ref Mode: Target Relative");
	
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
	return 0;
}

bool TargetRelativeAttitudeModeController::ProcessKey(DWORD key)
{
	return false;
}
