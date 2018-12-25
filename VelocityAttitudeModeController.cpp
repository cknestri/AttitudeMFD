#include "VelocityAttitudeModeController.h"
#include "CDK.h"
#include "Display.h"

VelocityAttitudeModeController::VelocityAttitudeModeController(
		VESSEL* spacecraft,
		DWORD displayWidth,
		DWORD displayHeight)
	: m_spacecraft(spacecraft)
	, m_displayWidth(displayWidth)
	, m_displayHeight(displayHeight)
{
}

VelocityAttitudeModeController::~VelocityAttitudeModeController()
{

}

void VelocityAttitudeModeController::Start()
{

}

bool VelocityAttitudeModeController::Update(oapi::Sketchpad* sketchpad)
{
	Display* display = new Display(sketchpad, m_displayWidth, m_displayHeight);

	display->Reset();
	
	display->SetTextColor(RGB(255, 255, 255));
	display->DisplayText("Attitude MFD");
	display->SetTextColor(RGB(0, 255, 0));
	display->IncrementCurrentLine();

	display->DisplayText("Ref Mode: Velocity");
	
	return true;
}

void VelocityAttitudeModeController::UpdateState()
{
}

void VelocityAttitudeModeController::EnableAutopilot()
{
}

void VelocityAttitudeModeController::DisableAutopilot()
{
}

void VelocityAttitudeModeController::Control()
{
}

int VelocityAttitudeModeController::GetButtonMenu(const MFDBUTTONMENU** buttonMenu) const
{
	return 0;
}

bool VelocityAttitudeModeController::ProcessKey(DWORD key)
{
	return false;
}
