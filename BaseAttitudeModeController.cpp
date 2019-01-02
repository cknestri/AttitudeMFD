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