#pragma once

#include "IAttitudeModeController.h"
#include <vector>

class TargetRelativeAttitudeModeController : public IAttitudeModeController
{
public:
	TargetRelativeAttitudeModeController(
		VESSEL* spacecraft,
		DWORD displayWidth,
		DWORD displayHeight);
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

	unsigned int m_selectedTargetIndex;
	std::vector<OBJHANDLE> m_targetList;
	char m_targetName[100];
	
	void BuildTargetList();
	double GetRelativeDistance(const OBJHANDLE objectHandle) const;
	void SelectNextTarget();
};