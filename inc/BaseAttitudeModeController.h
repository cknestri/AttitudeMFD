#pragma once

#include "OrbiterSDK.h"
#include "IDisplay.h"
#include "IAutopilot.h"
#include <functional>
#include <memory>

typedef VECTOR3 Attitude;
typedef std::function<std::shared_ptr<IDisplay>(oapi::Sketchpad* sketchpad)> CreateDisplayFunction;

class BaseAttitudeModeControl
{
public:
	BaseAttitudeModeControl(
		VESSEL* spacecraft,
		const std::shared_ptr<IAutopilot>& autopilot,
		const CreateDisplayFunction& createDisplay);

	virtual ~BaseAttitudeModeControl();

protected:
	VESSEL* m_spacecraft;
	std::shared_ptr<IAutopilot> m_autopilot;
	CreateDisplayFunction m_createDisplay;

	void PrintMFDName(const std::shared_ptr<IDisplay>& display) const;
	void PrintReferenceMode(const std::shared_ptr<IDisplay>& display, const char* referenceMode, bool isAutopilotEngaged) const;
};
