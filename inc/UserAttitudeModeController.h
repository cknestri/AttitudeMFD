#pragma once

#include "IAttitudeModeController.h"
#include "BaseAttitudeModeController.h"
#include "CDK.h"
#include "IAutopilot.h"
#include "IDisplay.h"

typedef VECTOR3 Attitude;

class UserAttitudeModeController : public IAttitudeModeController
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
	void Control() override;
	int UserAttitudeModeController::GetButtonMenu(const MFDBUTTONMENU** buttonMenu) const override;
	bool ProcessKey(DWORD key);

private:
	VESSEL* m_spacecraft;
	VESSELSTATUS m_status;
	VECTOR3 m_globalSpacecraftPosition;
	VECTOR3 m_pitchYawRollAngles;
	Attitude m_referenceAttitude;
	Attitude m_relativeAttitude;
	bool m_isAutopilotEngaged;
	std::shared_ptr<IAutopilot> m_autopilot;
	CreateDisplayFunction m_createDisplay;

	void CalculateAttitude();
	void SetReferenceAttitude();
	VECTOR3 CalcPitchYawRollAngles();
	VECTOR3 GetPYR(VECTOR3 Pitch, VECTOR3 YawRoll);
};