#include "BaseAttitudeModeController.h"

BaseAttitudeModeControl::BaseAttitudeModeControl(
	VESSEL* spacecraft,
	const std::shared_ptr<IAutopilot>& autopilot,
	const CreateDisplayFunction& createDisplay)
	: m_spacecraft(spacecraft)
	, m_autopilot(autopilot)
	, m_createDisplay(createDisplay)
{

}

BaseAttitudeModeControl::~BaseAttitudeModeControl()
{

}

void BaseAttitudeModeControl::PrintMFDName(const std::shared_ptr<IDisplay>& display) const
{
	display->SetTextColor(RGB(255, 255, 255));
	display->DisplayText("Attitude MFD");
	display->SetTextColor(RGB(0, 255, 0));
	display->PrintNewline();
}

void BaseAttitudeModeControl::PrintReferenceMode(const std::shared_ptr<IDisplay>& display, const char * referenceMode, bool isAutopilotEngaged) const
{
	char* holdString = "";

	if (isAutopilotEngaged)
	{
		holdString = " (Hold)";
	}
	
	display->DisplayText("Ref Mode: %s%s", referenceMode, holdString);
	display->PrintNewline();
}

void BaseAttitudeModeControl::PrintRelativeAttitude(const std::shared_ptr<IDisplay>& display, const Attitude& relativeAttitude) const
{
	display->PrintAngle("Set Pitch", relativeAttitude.data[PITCH]);
	display->PrintAngle("Set Yaw ", relativeAttitude.data[YAW]);
	display->PrintAngle("Set Roll", relativeAttitude.data[ROLL]);
	display->PrintNewline();
}


