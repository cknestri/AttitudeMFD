#pragma once

#include <Orbitersdk.h>

class IAttitudeModeController
{
public:
	virtual void Start() = 0;
	virtual bool Update (oapi::Sketchpad* sketchpad) = 0;
	virtual void UpdateState() = 0;
};