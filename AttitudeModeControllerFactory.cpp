#include "AttitudeModeControllerFactory.h"
#include "UserAttitudeModeController.h"
#include "VelocityAttitudeModeController.h"
#include "TargetRelativeAttitudeModeController.h"
#include "EntryInterfaceAttitudeModeController.h"
#include "Autopilot.h"
#include <memory>

IAttitudeModeController* GetAttitudeModeController(
	REF_MODE mode,
	VESSEL* spacecraft,
	DWORD displayWidth,
	DWORD displayHeight)
{
	auto autopilot = std::make_shared<Autopilot>(spacecraft);

	switch (mode)
	{
	case USER_ATT: 
		return new UserAttitudeModeController(spacecraft, displayWidth, displayHeight, autopilot);
	case VELOCITY:
		return new VelocityAttitudeModeController(spacecraft, displayWidth, displayHeight);
	case TARGET_RELATIVE:
		return new TargetRelativeAttitudeModeController(spacecraft, displayWidth, displayHeight, autopilot);
	case EI:
		return new EntryInterfaceAttitudeModeController(spacecraft, displayWidth, displayHeight);
	default:
		throw std::runtime_error("Unrecognized mode");
	}
}