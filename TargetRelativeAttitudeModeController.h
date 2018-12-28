#pragma once

#include "IAttitudeModeController.h"
#include "IAutopilot.h"
#include <vector>

class TargetRelativeAttitudeModeController : public IAttitudeModeController
{
public:
	TargetRelativeAttitudeModeController(
		VESSEL* spacecraft,
		DWORD displayWidth,
		DWORD displayHeight,
		const std::shared_ptr<IAutopilot>& autopilot);
	virtual ~TargetRelativeAttitudeModeController();

	void Start() override;
	bool Update(oapi::Sketchpad* sketchpad) override;
	void UpdateState() override;
	void EnableAutopilot();
	void DisableAutopilot();
	void Control() override;
	int GetButtonMenu(const MFDBUTTONMENU** buttonMenu) const override;
	bool ProcessKey(DWORD key);

private:
	VESSEL* m_spacecraft;
	DWORD m_displayWidth;
	DWORD m_displayHeight;
	VESSELSTATUS m_status;
	bool m_isAutopilotEngaged;
	std::shared_ptr<IAutopilot> m_autopilot;

	unsigned int m_selectedTargetIndex;
	std::vector<OBJHANDLE> m_targetList;
	char m_targetName[100];
	
	VECTOR3 m_relativePosition;
	VECTOR3 m_relativeVelocity;
	double m_radialVelocity;
	VECTOR3 m_pitchYawAngles;

	void BuildTargetList();
	double GetRelativeDistanceToObject(const OBJHANDLE objectHandle) const;
	void SetRelativePositionToSelectedTarget();
	void SetRelativeVelocityToSelectedTarget();
	void SetRadialVelocityToSelectedTarget();
	void GetLocalPositionOfObject(OBJHANDLE object, VECTOR3& localPosition) const;
	OBJHANDLE GetSelectedTargetHandle() const;
	void SelectNextTarget();
	void SelectPreviousTarget();
};