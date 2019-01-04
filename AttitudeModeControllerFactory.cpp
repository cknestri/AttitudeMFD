#include "AttitudeModeControllerFactory.h"
#include "UserAttitudeModeController.h"
#include "VelocityAttitudeModeController.h"
#include "TargetRelativeAttitudeModeController.h"
#include "EntryInterfaceAttitudeModeController.h"
#include "Autopilot.h"
#include "Display.h"
#include <memory>

IAttitudeModeController* GetAttitudeModeController(
	REF_MODE mode,
	VESSEL* spacecraft,
	DWORD displayWidth,
	DWORD displayHeight)
{
	auto autopilot = std::make_shared<Autopilot>(spacecraft);

	auto createDisplay = [displayWidth, displayHeight](oapi::Sketchpad* sketchpad)
	{
		return std::make_shared<Display>(sketchpad, displayWidth, displayHeight);
	};

	switch (mode)
	{
	case USER_ATT: 
		return new UserAttitudeModeController(spacecraft, autopilot, createDisplay);
	case VELOCITY:
		return new VelocityAttitudeModeController(spacecraft, autopilot, createDisplay);
	case TARGET_RELATIVE:
		return new TargetRelativeAttitudeModeController(spacecraft, autopilot, createDisplay);
	case EI:
		return new EntryInterfaceAttitudeModeController(spacecraft, displayWidth, displayHeight);
	default:
		throw std::runtime_error("Unrecognized mode");
	}
}