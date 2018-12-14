#pragma once
#include "IAttitudeModeController.h"
#include "AttitudeMFD.h"

IAttitudeModeController* GetAttitudeModeController(
	REF_MODE mode,
	VESSEL* spacecraft,
	DWORD displayWidth,
	DWORD DisplayHeight);