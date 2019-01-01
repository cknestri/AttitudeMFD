#pragma once

#include "IAttitudeModeController.h"
#include "BaseAttitudeModeController.h"
#include "IAutopilot.h"
#include "TrimState.h"
#include <map>
#include <vector>

class TargetRelativeAttitudeModeController : public IAttitudeModeController
{
public:
	TargetRelativeAttitudeModeController(
		VESSEL* spacecraft,
		const std::shared_ptr<IAutopilot>& autopilot,
		const CreateDisplayFunction& createDisplay);
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
	VESSELSTATUS m_status;
	bool m_isAutopilotEngaged;
	std::shared_ptr<IAutopilot> m_autopilot;
	CreateDisplayFunction m_createDisplay;

	std::map<DWORD, std::function<void()>> m_commandMap;

	unsigned int m_selectedTargetIndex;
	std::vector<OBJHANDLE> m_targetList;
	char m_targetName[100];
	
	VECTOR3 m_relativePosition;
	VECTOR3 m_relativeVelocity;
	double m_radialVelocity;
	VECTOR3 m_pitchYawAngles;

	TrimState m_trimState;

	void InitializeCommandMap();
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