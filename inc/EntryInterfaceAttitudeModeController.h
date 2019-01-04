#pragma once

#include "IAttitudeModeController.h"
#include "BaseAttitudeModeController.h"
#include <map>
#include <memory>

class EntryInterfaceAttitudeModeController : public IAttitudeModeController, public BaseAttitudeModeControl
{
public:
	EntryInterfaceAttitudeModeController(
		VESSEL* spacecraft,
		const std::shared_ptr<IAutopilot>& autopilot,
		const CreateDisplayFunction& createDisplay);
	virtual ~EntryInterfaceAttitudeModeController();

	void Start() override;
	bool Update(oapi::Sketchpad* sketchpad) override;
	void UpdateState() override;
	void EnableAutopilot();
	void DisableAutopilot();
	void Control() override;
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

	ENTRY_INTERFACE m_entryInterface;

	void InitializeCommandMap();
	VECTOR3 CalcPitchYawRollAngles();
	VECTOR3 GetPYR(VECTOR3 Pitch, VECTOR3 YawRoll);
};