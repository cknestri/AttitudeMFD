#pragma once

#include "OrbiterSDK.h"
#include "IDisplay.h"
#include "IAutopilot.h"
#include <functional>
#include <memory>

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
};
