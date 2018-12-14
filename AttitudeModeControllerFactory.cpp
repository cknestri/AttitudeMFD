#include "AttitudeModeControllerFactory.h"
#include "UserAttitudeModeController.h"
#include <memory>

IAttitudeModeController* GetAttitudeModeController(
	REF_MODE mode,
	VESSEL* spacecraft,
	DWORD displayWidth,
	DWORD displayHeight)
{
	return new UserAttitudeModeController(spacecraft, displayWidth, displayHeight);
}