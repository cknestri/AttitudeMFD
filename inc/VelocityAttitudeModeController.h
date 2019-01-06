#pragma once

#include "IAttitudeModeController.h"
#include "BaseAttitudeModeController.h"
#include "IAutopilot.h"
#include <map>

class VelocityAttitudeModeController : public IAttitudeModeController, public BaseAttitudeModeControl
{
public:
	VelocityAttitudeModeController(
		VESSEL* spacecraft,
		const std::shared_ptr<IAutopilot>& autopilot,
		const CreateDisplayFunction& createDisplay);
	virtual ~VelocityAttitudeModeController();

	void Start() override;
	bool Update(oapi::Sketchpad* sketchpad) override;
	void UpdateState() override;
	void EnableAutopilot();
	void DisableAutopilot();
	void Control(double deltaTime) override;
	int GetButtonMenu(const MFDBUTTONMENU** buttonMenu) const override;
	bool ProcessKey(DWORD key);

private:
	VESSELSTATUS m_status;
	VECTOR3 m_globalSpacecraftPosition;
	Attitude m_referenceAttitude;
	Attitude m_relativeAttitude;
	VECTOR3 m_pitchYawRollAngles;
	bool m_isAutopilotEngaged;

	std::map<DWORD, std::function<void()>> m_commandMap;

	void InitializeCommandMap();
	VECTOR3 CalcPitchYawRollAngles();
	VECTOR3 GetPYR(VECTOR3 Pitch, VECTOR3 YawRoll);
};