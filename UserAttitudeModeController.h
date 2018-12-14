#pragma once

#include "IAttitudeModeController.h"
#include "CDK.h"

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
	int UserAttitudeModeController::GetButtonMenu(const MFDBUTTONMENU** buttonMenu) const override;

private:
	VESSEL* m_spacecraft;
	DWORD m_displayWidth;
	DWORD m_displayHeight;
	VESSELSTATUS m_status;
	VECTOR3 m_globalSpacecraftPosition;
	VECTOR3 m_pitchYawRollAngles;
	Attitude m_referenceAttitude;

	void CalculateAttitude();
	void SetReferenceAttitude();
	VECTOR3 CalcPitchYawRollAngles();
	VECTOR3 GetPYR(VECTOR3 Pitch, VECTOR3 YawRoll);
};