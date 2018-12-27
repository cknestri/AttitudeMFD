#include "EntryInterfaceAttitudeModeController.h"
#include "OrbiterSDK.h"
#include "Display.h"

EntryInterfaceAttitudeModeController::EntryInterfaceAttitudeModeController(
		VESSEL* spacecraft,
		DWORD displayWidth,
		DWORD displayHeight)
	: m_spacecraft(spacecraft)
	, m_displayWidth(displayWidth)
	, m_displayHeight(displayHeight)
{
}

EntryInterfaceAttitudeModeController::~EntryInterfaceAttitudeModeController()
{

}

void EntryInterfaceAttitudeModeController::Start()
{

}

bool EntryInterfaceAttitudeModeController::Update(oapi::Sketchpad* sketchpad)
{
	Display* display = new Display(sketchpad, m_displayWidth, m_displayHeight);

	display->Reset();
	
	display->SetTextColor(RGB(255, 255, 255));
	display->DisplayText("Attitude MFD");
	display->SetTextColor(RGB(0, 255, 0));
	display->PrintNewline();

	display->DisplayText("Ref Mode: Entry Interface");
	
	return true;
}

void EntryInterfaceAttitudeModeController::UpdateState()
{
}

void EntryInterfaceAttitudeModeController::EnableAutopilot()
{
}

void EntryInterfaceAttitudeModeController::DisableAutopilot()
{
}

void EntryInterfaceAttitudeModeController::Control()
{
}

int EntryInterfaceAttitudeModeController::GetButtonMenu(const MFDBUTTONMENU** buttonMenu) const
{
	return 0;
}

bool EntryInterfaceAttitudeModeController::ProcessKey(DWORD key)
{
	return false;
}
