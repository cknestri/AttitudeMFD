#pragma once

#include "IAttitudeModeController.h"
#include "BaseAttitudeModeController.h"
#include "CDK.h"
#include <map>

class UserAttitudeModeController : public IAttitudeModeController, public BaseAttitudeModeControl
{
public:
	UserAttitudeModeController(
		VESSEL* spacecraft,
		const std::shared_ptr<IAutopilot>& autopilot,
		const CreateDisplayFunction& createDisplay);
	virtual ~UserAttitudeModeController();

	void Start() override;
	bool Update(oapi::Sketchpad* sketchpad) override;
	void UpdateState() override;
	void EnableAutopilot();
	void DisableAutopilot();
	void Control(double deltaTime) override;
	int UserAttitudeModeController::GetButtonMenu(const MFDBUTTONMENU** buttonMenu) const override;
	bool ProcessKey(DWORD key);

private:
	VESSELSTATUS m_status;
	VECTOR3 m_pitchYawRollAngles;
	Attitude m_referenceAttitude;
	Attitude m_relativeAttitude;
	bool m_isAutopilotEngaged;

	std::map<DWORD, std::function<void()>> m_commandMap;

	void InitializeCommandMap();
	void CalculateAttitude();
	void SetReferenceAttitude();
};