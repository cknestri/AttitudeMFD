#pragma once

#include <Orbitersdk.h>

class IAttitudeModeController
{
public:
	virtual void Start() = 0;
	virtual bool Update (oapi::Sketchpad* sketchpad) = 0;
	virtual void UpdateState() = 0;
	virtual void EnableAutopilot() = 0;
	virtual void DisableAutopilot() = 0;
	virtual void Control(double deltaTime) = 0;
	virtual int GetButtonMenu(const MFDBUTTONMENU** buttonMenu) const = 0;
	virtual bool ProcessKey(DWORD key) = 0;
};