#pragma once

#include "IAttitudeModeController.h"
#include "CDK.h"
#include "IAutopilot.h"

typedef VECTOR3 Attitude;

class UserAttitudeModeController : public IAttitudeModeController
{
public:
	UserAttitudeModeController(
		VESSEL* spacecraft,
		DWORD displayWidth,
		DWORD displayHeight);
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
	DWORD m_displayWidth;
	DWORD m_displayHeight;
	VESSELSTATUS m_status;
	VECTOR3 m_globalSpacecraftPosition;
	VECTOR3 m_pitchYawRollAngles;
	Attitude m_referenceAttitude;
	Attitude m_relativeAttitude;
	bool m_isAutopilotEngaged;
	IAutopilot* m_autopilot;

	void CalculateAttitude();
	void SetReferenceAttitude();
	VECTOR3 CalcPitchYawRollAngles();
	VECTOR3 GetPYR(VECTOR3 Pitch, VECTOR3 YawRoll);
};